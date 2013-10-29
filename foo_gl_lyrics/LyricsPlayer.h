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
    bool stopDisplayLrc();

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

    bool setCurrentSong(const string &strSongName, const string &strAlbum, const string &strSinger);

    /// \brief 设置当前歌曲名称
    ///
    /// \param strSongName   歌曲名称
    /// \return       void  
    /// \see 无
    bool setCurrentSong(const string &strSongName);

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
    bool getNextLrcLine(pair<unsigned int, string> &lrcObj);

    static DWORD WINAPI delayFun(_In_  LPVOID lpParameter);

private:
    string m_strDir; ///< 歌词文件目录
    vector<pair<unsigned int, string>> m_lycVec; ///< 歌词容器(以行为单位)pair<timestamp, lyrics>
    HANDLE m_timerQueue; ///< 定时器队列
    static string m_info;       ///< 歌词信息(曲目、歌手、专辑、作者)
    LYC_CALLBACK m_cbFun;  ///< 回调函数，用于写回歌词
    vector<pair<unsigned int, string>>::size_type m_curLyc; ///< 当前歌词行数
};

#endif//__LYRICS_PLAYER_H__
