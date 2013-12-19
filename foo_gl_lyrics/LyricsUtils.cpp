#include "LyricsUtils.h"

#include <sys/types.h>
#include <sys/timeb.h>

void LyricsUtils::UnicodeToUTF8(char* pOut,wchar_t* pText)   
{   
    // 注意 WCHAR高低字的顺序,低字节在前，高字节在后   
    char* pchar = (char *)pText;   
    pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));  
    pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);  
    pOut[2] = (0x80 | (pchar[0] & 0x3F));  
} 

void LyricsUtils::GBToUTF8(std::string& pOut, char *pText, int pLen)  
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

std::string LyricsUtils::UTF8ToGB(const char* str)
{
    WCHAR *strSrc;
    TCHAR *szRes;
    int len;

    //get size
    int stringSize = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    strSrc = new WCHAR[stringSize + 1];
    memset(strSrc, 0, stringSize + 1);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, stringSize + 1);



    stringSize = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
    szRes = new TCHAR[stringSize + 1];
    len = (stringSize + 1) * sizeof(CHAR);
    memset(szRes, 0, len);
    WideCharToMultiByte(CP_ACP, 0, strSrc, -1, (LPSTR)szRes, stringSize, NULL, NULL);

    delete []strSrc;
    std::string strGB((char*)szRes);
    delete []szRes;
    return strGB;
}

long long LyricsUtils::getCurTime()
{
    timeb timeNow;
    ftime(&timeNow);
    return timeNow.time * 1000 + timeNow.millitm;
}
