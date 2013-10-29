#include "LyricsPlayer.h"
#include <cassert>
#include <fstream>

string LyricsPlayer::m_info;

LyricsPlayer::LyricsPlayer(void) : m_cbFun(NULL)
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

    //清空歌词信息
    m_lycVec.clear();
    m_info.clear();

    //加载歌词文件
    if (!loadLrcFile()) {
        string strErr = "没有找到歌词";
        callClientCb(strErr);

        return false;
    }
    
    //开始播放歌词
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
    while (file.peek() != EOF) {
        string line;
        getline(file, line);

        if (line.length() < 1) {
            continue;
        }

        string::size_type pos = line.find("[ti:");//曲目
        if (pos != string::npos) {
            m_info += " 曲目 : ";
            m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
            continue;
        }
        
        pos = line.find("[ar:");//歌手
        if (pos != string::npos) {
            m_info += " 歌手 : ";
            m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
            continue;
        }
        
        pos = line.find("[al:");//专辑
        if (pos != string::npos) {
            m_info += " 专辑 : ";
            m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
            continue;
        }

        pos = line.find("[by:");//歌词提供
        if (pos != string::npos) {
            m_info += " 歌词 : ";
            m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
            continue;
        }

        pos = line.find("[id:");//??
        if (pos != string::npos) {
            m_info += " id : ";
            m_info += line.substr(pos + 4, line.find_last_not_of("]") - pos - 3);
            continue;
        }

        //解析时间
        pos = line.find_first_of("[");
        string::size_type timeEndPos = line.find_first_of("]");
        string strTime = line.substr(pos + 1, timeEndPos - pos - 1);
        char time[2];
        memcpy(time, strTime.c_str(), 2); //获得分钟数
        int mins = atoi(time);
        memcpy(time, strTime.c_str() + 3, 2); //获得秒数
        int secs = atoi(time);
        memcpy(time, strTime.c_str() + 6, 2); //获得毫秒数
        int ms = atoi(time);

        unsigned int timeStamp = ms + mins * 60 * 1000 + secs * 1000; //计算毫秒
        
        strLrcLin = line.substr(timeEndPos + 1);
        pair<unsigned int, string> lrcElem = make_pair<unsigned int, string>(timeStamp, strLrcLin);
        m_lycVec.push_back(lrcElem);
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
