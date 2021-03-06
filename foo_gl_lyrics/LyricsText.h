#ifndef __LYRICS_TEXT_H__
#define __LYRICS_TEXT_H__

#include <windows.h>
#include "gl/gl.h"
#include "LyricsCommon.h"

#pragma warning(disable:4244)
//OpenGL������ʾ��
class LyricsText
{
    HFONT hFont;
    //operator HFONT( ) const;
    COLORREF cl;
public:
    LyricsText();
    virtual ~LyricsText();
    void SetTextColor(COLORREF textcolor);//������ɫ����
    void SetLrcFont(HFONT f);//why can't be HFONT&  ?
    void MyCreateFont(char *facename, int height, int weight, bool italic,bool underline,bool strikeout);
    void ShowText(int x, int y, LPCTSTR lpszText);//��ʾͼ��2D����
    void Show2DGbkText(char *str);//��ʾͼ��2D����
    void Show3DText(unsigned char *str);//��ʾͼ��3D����
    void SetGlCtx(GL_CONTEXT *glCtx);

private:
    unsigned char m_red;
    unsigned char m_green;
    unsigned char m_blue;

    GL_CONTEXT *m_glCtx;
};

#endif
