#ifndef _LRC_DOWNLOADER_H_
#define _LRC_DOWNLOADER_H_

#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")

#include <curl.h>
#include <string>

using namespace std;

class LrcDownloader
{
public:
    LrcDownloader(void);
    virtual ~LrcDownloader(void);

    bool getLrc(const char* artist, const char *title);

    int getLrcUrl(const char* getUrl, string &retBuff);
    int saveLrcFile(const char* strUrl, string &retBuff);

    //search in the html text to find out the lrc page
    static size_t searchLrcUrlHandler(void *buffer, size_t size, size_t nmemb, void *user_p);

    //search in the lrc page to get the lrc file
    static size_t saveLrcHandler(void *buffer, size_t size, size_t nmemb, void *user_p);

private:
    CURL *m_url;

    string m_siteUrl;

    static string m_lrcUrlBeginKey; //search the lrc file's url in the site page's source code
    static string m_lrcUrlEndKey; 
    static string m_lrcBeginKey; //the lrc is included in a http file. this is the keyword where the lrc begin
    static string m_lrcEndKey;

    static string m_lrcTitle;
    static string m_lrcArtist;
};

#endif //_LRC_DOWNLOADER_H_
