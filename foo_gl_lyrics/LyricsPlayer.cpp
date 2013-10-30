#include "LyricsPlayer.h"
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
    "[id:", //i don't know
    "[off",
    "[t_t"
};

string LyricsPlayer::m_info;

string UTF8ToGB(const char* str)
{
    WCHAR *strSrc;
    TCHAR *szRes;
    int len;

    //get size
    int stringSize = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    strSrc = new WCHAR[stringSize + 1];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, stringSize);

    stringSize = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
    szRes = new TCHAR[stringSize + 1];
    len = (stringSize + 1) * sizeof(CHAR);
    memset(szRes, 0, len);
    WideCharToMultiByte(CP_ACP, 0, strSrc, -1, (LPSTR)szRes, stringSize, NULL, NULL);

    delete []strSrc;
    string strGB((char*)szRes);
    delete []szRes;
    return strGB;
}

LyricsPlayer::LyricsPlayer(void) : m_cbFun(NULL), m_isUTF8(false)
{
}

LyricsPlayer::~LyricsPlayer(void)
{
}

void LyricsPlayer::setLrcDirectory(const string &strDir)
{
    m_lrcDir = strDir;
}

bool LyricsPlayer::setPlayingSong(const char *strSongName, const char *strAlbum, const char *strArtist)
{
    m_title = strSongName;
    m_artist = strArtist;
    m_album = strAlbum;

    //clear lyrics info
    m_lycVec.clear();
    m_info.clear();
    m_isUTF8 = false;

    //load lyrics file
    if (!loadLrcFile()) {

        //may be Encode in UTF-8
        m_title = UTF8ToGB(strSongName);
        m_artist = UTF8ToGB(strArtist);
        m_album = UTF8ToGB(strAlbum);

        m_isUTF8 = true;
        if (!loadLrcFile()) {
            m_isUTF8 = false;
            string strErr = "没有找到歌词";
            callClientCb(strErr);

            return false;
        }
    }
    
    //display music info
    m_info = m_artist + " - " + m_title;
    callClientCb(m_info);

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

        //string::size_type pos = line.find("[ti:");//曲目
        //if (pos != string::npos) {
        //    m_info += " 曲目 : ";
        //    GET_LYRICS_TEXT(m_info, line, pos);
        //    //if (m_isUTF8) {
        //    //    m_info += UTF8ToGB(line.substr(pos + 4, line.find_last_not_of("]") - pos - 3).c_str());
        //    //} else {
        //    //    m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
        //    //}
        //    continue;
        //}
        //
        //pos = line.find("[ar:");//歌手
        //if (pos != string::npos) {
        //    m_info += " 歌手 : ";
        //    GET_LYRICS_TEXT(m_info, line, pos);
        //    //m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
        //    continue;
        //}
        //
        //pos = line.find("[al:");//专辑
        //if (pos != string::npos) {
        //    m_info += " 专辑 : ";
        //    GET_LYRICS_TEXT(m_info, line, pos);
        //    //m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
        //    continue;
        //}

        //pos = line.find("[by:");//歌词提供
        //if (pos != string::npos) {
        //    m_info += " 歌词 : ";
        //    GET_LYRICS_TEXT(m_info, line, pos);
        //    //m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
        //    continue;
        //}

        //pos = line.find("[id:");//??
        //if (pos != string::npos) {
        //    m_info += " id : ";
        //    GET_LYRICS_TEXT(m_info, line, pos);
        //    //m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
        //    continue;
        //}

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
            strLrcLin = UTF8ToGB(line.c_str());
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
    HANDLE threadHandle;
    DWORD thID;

    //初始化歌词播放位置
    m_curLyc = 0;

    //创建线程开始播放歌词
    threadHandle = CreateThread(NULL, 0, delayFun, this, 0, &thID);

    if (threadHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    return true;
}


DWORD WINAPI LyricsPlayer::delayFun(_In_  LPVOID lpParameter)
{
    LyricsPlayer *player = (LyricsPlayer*)lpParameter;
    
    if (player == NULL || !player->isLrcCBValid()) {
        return -1;
    }

    player->callClientCb(m_info);
    pair<unsigned int, string> lrcObj;
    unsigned int lastTimeStamp = 0;
    unsigned int delay;
    while (player->getNextLrcLine(lrcObj)) {
        
        delay = lrcObj.first - lastTimeStamp;
        lastTimeStamp = lrcObj.first;
        Sleep(delay);
        player->callClientCb(lrcObj.second);
    }

    return 0;
}


bool LyricsPlayer::getNextLrcLine(pair<unsigned int, string> &lrcObj)
{
    if (m_curLyc >= m_lycVec.size()) {
        return false;
    }

    lrcObj = m_lycVec[m_curLyc];
    m_curLyc++;
    return true;
}

void LyricsPlayer::callClientCb(const string &strLrc)
{
    m_cbFun(strLrc);
}

bool LyricsPlayer::loadLrcFile()
{
    //歌词文件的命名规则和Lyrics show panel相同，即 : 艺术家 - 歌曲名称.lrc
    string lrcName = m_lrcDir + m_artist + " - " + m_title + ".lrc";
    
    //todo ： 判读文件是否存在，如不存在需要搜索网络并下载
    
    return parseLrc(lrcName);
}

void LyricsPlayer::addLrcSentence(unsigned int timeStamp, string &lrc)
{
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
