#ifndef _FOOBAR_GL_LYRICS_STRING_UTILS_
#define _FOOBAR_GL_LYRICS_STRING_UTILS_

#include <string>
#include <Windows.h>

//time and string utils are defined here
namespace LyricsUtils {

    void UnicodeToUTF8(char* pOut,wchar_t* pText);

    void GBToUTF8(std::string& pOut, char *pText, int pLen);

    std::string UTF8ToGB(const char* str);

    long long getCurTime();
}


#endif //_FOOBAR_GL_LYRICS_STRING_UTILS_
