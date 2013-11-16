/*! 
* \file LyricsPlayer.h 
* \brief 歌词播放类文件 
*  
*查找、解析、下载、播放歌词
*
* \author  Excalibur, Email:c04n05@gmail.com
* \version 1.00.00  
* \date    2013-10-28
*/ 
#ifndef __LYRICS_PLAYER_H__
#define __LYRICS_PLAYER_H__

#include <Windows.h>
#include <string>
#include <vector>
#include "LrcDownloader.h"

using namespace std;

//参数string就是歌词，调用模块需要将string显示出来并刷新界面
typedef void (*LYC_CALLBACK)(const string&);

/// \brief 歌词播放器类 
///  
///无
class LyricsPlayer
{

public:
    LyricsPlayer(void);
    virtual ~LyricsPlayer(void);

    /// \brief 开始播放歌词
    ///
    /// \param   void   
    /// \return  bool 是否播放成功  
    /// \see 无  
    bool startDisplayLrc();

    /// \brief 停止播放歌词
    ///
    /// \param   void   
    /// \return  bool 是否停止 
    /// \see 无  
    void stopDisplayLrc();

    void pauseDisplayLrc(bool isPause);

    void startPlayAnyTime(unsigned int tmPos);

    /// \brief 设置歌词文件目录
    ///
    /// \param strDir   歌词文件目录
    /// \return       void  
    /// \see 无  
    void setLrcDirectory(const string &strDir);

    /// \brief 设置歌词回调函数
    ///
    ///此回调函数的参数就是当前的歌词
    ///
    /// \param cb   回调函数指针
    /// \return       void  
    /// \see 无  
    void setLrcCB(LYC_CALLBACK cb);

    /// \brief 歌词回调函数是否有效
    ///
    /// \param void
    /// \return   bool  是否有效
    /// \see 无  
    bool isLrcCBValid();

    /// \brief 设置当前播放歌曲
    ///
    /// \param strSongName   歌曲名称
    /// \param strAlbum      专辑
    /// \param strArtist     艺术家
    /// \return       void  
    /// \see 无
    bool setPlayingSong(const char *strSongName, const char *strAlbum, const char *strArtist, long long lStartTime);

    /// \brief 设置当前歌曲名称
    ///
    /// \param strSongName   歌曲名称
    /// \return       void  
    /// \see 无
    bool setPlayingSong(const string &strSongName);

    /// \brief 解析歌词文件
    ///
    /// \param fileName   歌词文件路径
    /// \return       void  
    /// \see 无
    bool parseLrc(const string &fileName);

private:

    /// \brief 调用回调函数
    ///
    /// \param strLrc   单行歌词
    /// \return       void  
    /// \see 无
    void callClientCb(const string &strLrc);

    /// \brief 获取一行歌词
    ///
    /// \param    lrcObj 延迟时间和歌词
    /// \return   bool   是否成功  
    /// \see 无
    bool getNextLrcLine(pair<unsigned int, string> &lrcObj, unsigned int &lastTimeStamp);

    
    unsigned int getSongIndex();

    /// \brief    歌词定时显示线程函数
    ///
    /// \param    lpParameter 线程函数参数
    /// \return   DWORD       线程退出状态  
    /// \see 无
    static DWORD WINAPI delayFun(_In_  LPVOID lpParameter);

    /// \brief 加载歌词文件
    ///
    /// \param    autoDownload  
    /// \return   bool   加载歌词是否成功  
    /// \see 无
    bool loadLrcFile(bool autoDownload = false);

    /// \brief insert one line of lyrics
    void addLrcSentence(unsigned int timeStamp, string &lrc);

private:
    string m_lrcDir; ///< 歌词文件目录
    vector<pair<unsigned int, string>> m_lrcVec; ///< 歌词容器(以行为单位)pair<timestamp, lyrics>
    static string m_info;       ///< 歌词信息(曲目、歌手、专辑、作者)
    LYC_CALLBACK m_cbFun;  ///< 回调函数，用于写回歌词
    vector<pair<unsigned int, string>>::size_type m_curLrcLine; ///< 当前歌词行数
    unsigned int m_songIndex;

    typedef vector<pair<unsigned int, string>>::size_type LYCVEC_SIZE;

    BOOL m_isUTF8;  ///< lyrics file is UTF8 form
    string m_title; ///< 歌曲名称
    string m_artist; ///< 歌手(艺术家)
    string m_album; ///< 专辑

    static HANDLE m_thEvent; ///< thread stop event 
    static HANDLE m_freezeEvent; ///< thread stop event 
    HANDLE m_thLrc; ///< thread
    CRITICAL_SECTION m_cs;
    bool m_isPause;

    static long long m_tmOffset; //time offset for parse lyrics and download lyrics if required

    //used for pause play and resume play 
    long long m_tmStart;
    long long m_tmPause;
    long long m_tmDelay;

    LrcDownloader m_downloader;
};

#endif//__LYRICS_PLAYER_H__
