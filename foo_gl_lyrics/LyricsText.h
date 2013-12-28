#ifndef __LYRICS_TEXT_H__
#define __LYRICS_TEXT_H__

#include <windows.h>
#include "gl/gl.h"
#include "LyricsCommon.h"

#pragma warning(disable:4244)
//OpenGL汉字显示类
class LyricsText
{
    HFONT hFont;
    //operator HFONT( ) const;
    COLORREF cl;
public:
    LyricsText();
    virtual ~LyricsText();
    void SetTextColor(COLORREF textcolor);//字体颜色设置
    void SetLrcFont(HFONT f);//why can't be HFONT&  ?
    void MyCreateFont(char *facename, int height, int weight, bool italic,bool underline,bool strikeout);
    void ShowText(int x, int y, LPCTSTR lpszText);//显示图象2D汉字
    void Show2DGbkText(char *str);//显示图形2D汉字
    void Show3DText(unsigned char *str);//显示图形3D汉字
    void SetGlCtx(GL_CONTEXT *glCtx);

private:
    unsigned char m_red;
    unsigned char m_green;
    unsigned char m_blue;

    GL_CONTEXT *m_glCtx;
};

#endif
