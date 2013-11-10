#include "LrcDownloader.h"
#include <fstream>

using namespace std;

void UnicodeToUTF8(char* pOut,wchar_t* pText)   
{   
    // 注意 WCHAR高低字的顺序,低字节在前，高字节在后   
    char* pchar = (char *)pText;   
    pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));  
    pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);  
    pOut[2] = (0x80 | (pchar[0] & 0x3F));  
} 

void GBToUTF8(string& pOut,char *pText, int pLen)  
{    
    char buf[4] = {0};   
    int nLength = pLen* 3;   
    char* rst = new char[nLength];    
    memset(rst,0,nLength);    
    int i = 0 ,j = 0;   
    while(i < pLen)   
    {   
        //如果是英文直接复制就可以   
        if( *(pText + i) >= 0)   
        {    
            rst[j++] = pText[i++];  
        }   
        else    
        {    
            wchar_t pbuffer;
            ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pText + i, 2, &pbuffer, 1); 
            UnicodeToUTF8(buf, &pbuffer);  
            rst[j] = buf[0];     
            rst[j+1] = buf[1];   
            rst[j+2] = buf[2];   
            j += 3;   
            i += 2;   
        }     
    }    

    rst[j] ='\n';   //返回结果    
    pOut = rst;  
    delete []rst;  
    return;     
} 

string LrcDownloader::m_lrcUrlBeginKey = "http://www.1ting.com/lrc";
string LrcDownloader::m_lrcUrlEndKey = "\""; 
string LrcDownloader::m_lrcBeginKey = "<div id=\"lrc\">";;
string LrcDownloader::m_lrcEndKey = "</div>";

string LrcDownloader::m_lrcTitle;
string LrcDownloader::m_lrcArtist;

LrcDownloader::LrcDownloader(void)
{
    m_siteUrl = "http://so.1ting.com/all.do?q=";


    CURLcode retCode = curl_global_init(CURL_GLOBAL_ALL);

    if (CURLM_OK == retCode) {
        m_url = curl_easy_init();
    }

    if (NULL == m_url) {
        curl_global_cleanup();
    }

    curl_easy_setopt(m_url, CURLOPT_COOKIEJAR, "cookie.txt");
}


LrcDownloader::~LrcDownloader(void)
{
    if (m_url != NULL) {
        curl_easy_cleanup(m_url);
    }
    
    curl_global_cleanup();
}

size_t LrcDownloader::searchLrcUrlHandler(void *buffer, size_t size, size_t nmemb, void *user_p)
{
    string strIn((char*)buffer);

    size_t pos = strIn.find(m_lrcUrlBeginKey);
    if (pos == string::npos) {
        return size * nmemb;
    }

    string *str = (string*)user_p;
    strIn = strIn.substr(pos);
    pos = strIn.find(m_lrcUrlEndKey);
    strIn = strIn.substr(0, pos);
    str->append(strIn);

    return 0;
}

size_t LrcDownloader::saveLrcHandler(void *buffer, size_t size, size_t nmemb, void *user_p)
{
    string strIn((char*)buffer);
    size_t posStart = string::npos;
    string *str = (string*)user_p;

    if (str->empty()) {
        posStart = strIn.find(m_lrcBeginKey);
        if (posStart == string::npos) {
            return size * nmemb;
        }

        strIn = strIn.substr(posStart + m_lrcBeginKey.length());
    }

    size_t pos = strIn.find(m_lrcEndKey);
    if (pos != string::npos) {

        strIn = strIn.substr(0, pos);
        str->append(strIn);

        //set lrc file name
        string fileName;
        fileName = m_lrcArtist;
        fileName += " - ";
        fileName += m_lrcTitle;
        fileName += ".lrc";

        //attention : The file name seems only support ANSI 
        //i have tried utf8 and failed to create file.
        fstream fileStream(fileName, ios::out | ios::trunc);

        if (fileStream.is_open()) {
            fileStream << str->c_str();
        }

        fileStream.close();

        return 0;
    }

    str->append(strIn);
    return size * nmemb;
}

int LrcDownloader::getLrcUrl(const char* getUrl, string &retBuff)
{
    if (NULL == getUrl)
        return 0;

    CURLcode  ret;
    ret = curl_easy_setopt(m_url, CURLOPT_URL, getUrl);
    if (CURLM_OK != ret) {
        printf("set http get url : %s failed!\n", getUrl);
        return -1;
    }

    ret = curl_easy_setopt(m_url, CURLOPT_WRITEFUNCTION, searchLrcUrlHandler);
    if (CURLM_OK != ret) {
        printf("set http get's process data function failed! url : %s failed!\n", getUrl);
        return -1;
    }

    curl_easy_setopt(m_url, CURLOPT_WRITEDATA, &retBuff);
    if (CURLM_OK != ret) {
        printf("set http get's return buff failed! url : %s failed!\n", getUrl);
        return -1;
    }


    ret = curl_easy_perform(m_url);
    if (CURLM_OK != ret) {
        printf("http get url failed! url = %s\n", getUrl);
        return -1;
    }

    return 0;
}

int LrcDownloader::saveLrcFile(const char* strUrl, string &retBuff)
{
    if (NULL == strUrl)
        return 0;

    CURLcode  ret;
    ret = curl_easy_setopt(m_url, CURLOPT_URL, strUrl);
    if (CURLM_OK != ret) {
        printf("set http get url : %s failed!\n", strUrl);
        return -1;
    }

    ret = curl_easy_setopt(m_url, CURLOPT_WRITEFUNCTION, saveLrcHandler);
    if (CURLM_OK != ret) {
        printf("set http get's process data function failed! url : %s failed!\n", strUrl);
        return -1;
    }

    curl_easy_setopt(m_url, CURLOPT_WRITEDATA, &retBuff);
    if (CURLM_OK != ret) {
        printf("set http get's return buff failed! url : %s failed!\n", strUrl);
        return -1;
    }


    ret = curl_easy_perform(m_url);
    if (CURLM_OK != ret) {
        printf("http get url failed! url = %s\n", strUrl);
        return -1;
    }

    return 0;
}

bool LrcDownloader::getLrc(const char* artist, const char *title)
{
    m_lrcTitle = title;
    m_lrcArtist = artist;
    string strTitle;
    GBToUTF8(strTitle, const_cast<char*>(title), strlen(title));

    char strUrl[1024];
    memset(strUrl, 0, 1024);
    int urlSize = m_siteUrl.length();
    memcpy(strUrl, m_siteUrl.c_str(), urlSize);

    char spaceChar = 0xa;
    for (int i = 0; i < strTitle.length(); i++) {
        if (strTitle.at(i) == spaceChar) {
            continue;
        }
        sprintf(strUrl + urlSize + i * 3, "%%%02X", (unsigned char)strTitle.at(i));

    }
    /*%E7%A9%BA%E3%81%AF%E9%AB%98%E3%81%8F%E9%A2%A8%E3%81%AF%E6%AD%8C%E3%81%86*/
    /*%E7%A9%BA%E3%81%AF%E9%AB%98%E3%81%8F%E9%A2%A8%E3%81%AF%E6%AD%8C%E3%81%86%0A*/

    string ret;
    getLrcUrl(strUrl, ret);

    string strLrc;
    saveLrcFile(ret.c_str(), strLrc);


    return true;
}
