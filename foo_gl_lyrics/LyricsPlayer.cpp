#include "LyricsPlayer.h"
#include "utils.h"

#include <cassert>
#include <fstream>

#define GET_LYRICS_TEXT(text, linetext, pos) \
    if (m_isUTF8) { \
        text +=  UTF8ToGB(linetext.substr(pos + 4, linetext.find_last_not_of("]") - pos - 3).c_str()); \
    } else { \
        m_info += linetext.substr(pos + 4, linetext.find_last_not_of("]") - pos - 3); \
    }

#define LRC_LABEL_LEN 7
char lrc_label[LRC_LABEL_LEN][5] = {
    "[ti:", //title
    "[ar:", //artist
    "[al:", //album
    "[by:", //lyr file author
    "[id:", 
    "[off",
    "[t_t"
};

string LyricsPlayer::m_info;
HANDLE LyricsPlayer::m_thEvent;
HANDLE LyricsPlayer::m_freezeEvent;
long long LyricsPlayer::m_tmOffset = 0;

LyricsPlayer::LyricsPlayer(void) : m_cbFun(NULL), m_isUTF8(false), m_thLrc(NULL), m_tmStart(0)
    , m_tmPause(0), m_tmDelay(0), m_isPause(FALSE)
{
    m_thEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_freezeEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    InitializeCriticalSection(&m_cs);
}

LyricsPlayer::~LyricsPlayer(void)
{
    if (m_thLrc) {
        CloseHandle(m_thLrc);
    }

    if (m_thEvent) {
        CloseHandle(m_thEvent);
    }

    if (m_freezeEvent) {
        CloseHandle(m_freezeEvent);
    }

    DeleteCriticalSection(&m_cs);
}

void LyricsPlayer::setLrcDirectory(const string &strDir)
{
    m_lrcDir = strDir;
    LrcDownloader::setLrcDir(strDir);
}

bool LyricsPlayer::setPlayingSong(const char *strSongName, const char *strAlbum, const char *strArtist, long long lStartTime)
{
    m_title = strSongName;
    m_artist = strArtist;
    m_album = strAlbum;
    m_tmOffset = lStartTime; //set the start time;

    //clear lyrics info
    ResetEvent(m_freezeEvent);
    m_lycVec.clear();
    m_info.clear();
    m_isUTF8 = false;
    m_isPause = FALSE;
    m_tmPause = 0;
    m_tmDelay = 0;
    m_curLyc = 0;

    //load lyrics file
    if (!loadLrcFile()) {

        //may be Encode in UTF-8
        m_title = gl_lyrics_utils::UTF8ToGB(strSongName);
        m_artist = gl_lyrics_utils::UTF8ToGB(strArtist);
        m_album = gl_lyrics_utils::UTF8ToGB(strAlbum);

        m_isUTF8 = true;
        if (!loadLrcFile(true)) {
            m_isUTF8 = false;
            string strErr = "没有找到歌词";
            callClientCb(strErr);

            SetEvent(m_freezeEvent);
            return false;
        }
    }
    
    //display music info
    m_info = m_artist + " - " + m_title;
    callClientCb(m_info);

    SetEvent(m_freezeEvent);

    //play lyrics
    return startDisplayLrc();
}

void LyricsPlayer::setLrcCB(LYC_CALLBACK cb)
{
    m_cbFun = cb;
}

bool LyricsPlayer::isLrcCBValid()
{
    if (m_cbFun != NULL) {
        return true;
    }
    return false;
}

bool LyricsPlayer::parseLrc(const string &fileName)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        return false;
    }

    int lastTimeStamp = 0;
    string strLrcLin;
    vector<unsigned int> timeStampVec;
    bool bIsLabel = false;
    while (file.peek() != EOF) {

        string line;
        getline(file, line);

        if (line.length() < 1) {
            continue;
        }

        string::size_type pos;

        for (int i = 0; i < LRC_LABEL_LEN; i++) {
            if ((pos = line.find(lrc_label[i])) != string::npos) {
                bIsLabel = true;
                break;
            }
        }

        if (bIsLabel) {
            bIsLabel = false;
            continue;
        }

        //解析时间
        pos = line.find_first_of("[");
        string::size_type timeEndPos = line.find_first_of("]");
        string strTime = line.substr(pos + 1, timeEndPos - pos - 1);

        char time[2];
        memcpy(time, strTime.c_str(), 2); //get minute
        int mins = atoi(time);
        memcpy(time, strTime.c_str() + 3, 2); //get sec
        int secs = atoi(time);
        memcpy(time, strTime.c_str() + 6, 2); //get ms
        int ms = atoi(time);

        unsigned int timeStamp = ms + mins * 60 * 1000 + secs * 1000; //计算毫秒

        //support multi time stamp in one line
  
        timeStampVec.push_back(timeStamp);

        line = line.substr(timeEndPos + 1);
        while ((pos = line.find_first_of("[")) != string::npos) {
            timeEndPos = line.find_first_of("]");
            strTime = line.substr(pos + 1, timeEndPos - pos - 1);

            memcpy(time, strTime.c_str(), 2); //get minute
            mins = atoi(time);
            memcpy(time, strTime.c_str() + 3, 2); //get sec
            secs = atoi(time);
            memcpy(time, strTime.c_str() + 6, 2); //get ms
            ms = atoi(time);

            timeStamp = ms + mins * 60 * 1000 + secs * 1000; //计算毫秒
            timeStampVec.push_back(timeStamp);

            line = line.substr(timeEndPos + 1);
        }

        if (line.empty() || line.length() == 0 || line.compare(" ") == 0) {
            continue;
        }

        //check utf8
        if (m_isUTF8) {
            strLrcLin = gl_lyrics_utils::UTF8ToGB(line.c_str());
        } else {
            strLrcLin = line;
        }

        vector<unsigned int>::iterator ite = timeStampVec.begin();
        for (; ite != timeStampVec.end(); ++ite) {
            addLrcSentence(*ite, strLrcLin);
        }

        timeStampVec.clear();
    }

    return true;
}

bool LyricsPlayer::startDisplayLrc()
{
    DWORD thID;

    if (m_thLrc) {
        CloseHandle(m_thLrc);
    }

    //创建线程开始播放歌词
    m_tmStart = gl_lyrics_utils::getCurTime();
    m_tmOffset = m_tmStart - m_tmOffset; //count the offset time
    m_thLrc = CreateThread(NULL, 0, delayFun, this, 0, &thID);

    if (m_thLrc == INVALID_HANDLE_VALUE) {
        return false;
    }

    return true;
}

void LyricsPlayer::stopDisplayLrc()
{
    //string stopMsg = "播放停止";
    SetEvent(m_thEvent);

    if (m_isPause) {
        m_isPause = FALSE;
    }

    //WaitForSingleObject(m_thLrc, INFINITE);
    //callClientCb(stopMsg);
}

void LyricsPlayer::startPlayAnyTime(unsigned int tmPos)
{
    pair<unsigned int, string> lrcElem;

    EnterCriticalSection(&m_cs);
    for (LYCVEC_SIZE i = 0; i < m_lycVec.size(); i++) {
        lrcElem =  m_lycVec[i];
        if (lrcElem.first > tmPos) {
            m_curLyc = i;
            m_lycVec[i - 1].first = tmPos;
            callClientCb(m_lycVec[i - 1].second);
            break;
        }
    }
    LeaveCriticalSection(&m_cs);

}

void LyricsPlayer::pauseDisplayLrc(bool isPause)
{
    if (m_isPause != isPause) {

        m_isPause = isPause;
        if (isPause) {
            m_tmPause = gl_lyrics_utils::getCurTime();
            //stop thread here

            ResetEvent(m_freezeEvent);
        } else {
            m_tmDelay += (gl_lyrics_utils::getCurTime() - m_tmPause);

            startPlayAnyTime(static_cast<unsigned int>(gl_lyrics_utils::getCurTime() - m_tmStart - m_tmDelay));
            SetEvent(m_freezeEvent);
        }
    }
}

DWORD WINAPI LyricsPlayer::delayFun(_In_  LPVOID lpParameter)
{
    LyricsPlayer *player = (LyricsPlayer*)lpParameter;
    
    if (player == NULL || !player->isLrcCBValid()) {
        return -1;
    }

    player->callClientCb(m_info);
    pair<unsigned int, string> lrcObj;
    unsigned int lastTimeStamp = m_tmOffset;
    unsigned int delay;
    while (player->getNextLrcLine(lrcObj, lastTimeStamp) && WaitForSingleObject(m_thEvent, 0) == WAIT_TIMEOUT) {
        
        delay = lrcObj.first - lastTimeStamp;
        Sleep(delay);
        if (WAIT_OBJECT_0 != WaitForSingleObject(m_freezeEvent, 0)) {
            continue;
        }

        player->callClientCb(lrcObj.second);
    }

    return 0;
}


bool LyricsPlayer::getNextLrcLine(pair<unsigned int, string> &lrcObj, unsigned int &lastTimeStamp)
{
    if (m_curLyc >= m_lycVec.size()) {
        return false;
    }

    EnterCriticalSection(&m_cs);

    lrcObj = m_lycVec[m_curLyc];
    
    if (m_curLyc == 0) {
        lastTimeStamp = 0;
    } else {
        lastTimeStamp = (m_lycVec[m_curLyc - 1].first);
    }
    m_curLyc++;

    LeaveCriticalSection(&m_cs);

    
    return true;
}

void LyricsPlayer::callClientCb(const string &strLrc)
{
    m_cbFun(strLrc);
}

bool LyricsPlayer::loadLrcFile(bool autoDownload)
{
    //歌词文件的命名规则和Lyrics show panel相同，即 : 艺术家 - 歌曲名称.lrc
    string lrcName = m_lrcDir + m_artist + " - " + m_title + ".lrc";

    bool flag = parseLrc(lrcName);
    if (!flag && autoDownload) {
        
        //download lrc
        m_downloader.getLrc(m_artist.c_str(), m_title.c_str());
        return parseLrc(lrcName);
    }

    return flag;
}

void LyricsPlayer::addLrcSentence(unsigned int timeStamp, string &lrc)
{
    if (lrc.empty()) {
        return;
    }

    pair<unsigned int, string> lrcElem = make_pair<unsigned int, string>(timeStamp, lrc);
    if (m_lycVec.size() == 0) {
        lrcElem = make_pair<unsigned int, string>(timeStamp, lrc);
        m_lycVec.push_back(lrcElem);

        return;
    }

    vector<pair<unsigned int, string>>::iterator ite = m_lycVec.begin();

    for (; ite != m_lycVec.end(); ++ite) {
        if (ite->first > timeStamp) {
            //ite--;
            m_lycVec.insert(ite, lrcElem);

            return;
        }
    }

    m_lycVec.push_back(lrcElem);

}

