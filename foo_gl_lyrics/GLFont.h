//GLFont.h文件

#ifndef __GL_FONT_H__
#define __GL_FONT_H__

#include <windows.h>
#include "gl/gl.h"
#pragma warning(disable:4244)
//OpenGL汉字显示类
class MyGLfont
{
    HFONT hFont;
    //operator HFONT( ) const;
    COLORREF cl;
public:
    MyGLfont();
    virtual ~MyGLfont();
    void SetTextColor(COLORREF textcolor);//字体颜色设置
    void SetLrcFont(HFONT f);//why can't be HFONT&  ?
    void MyCreateFont(char *facename, int height, int weight, bool italic,bool underline,bool strikeout);
    void ShowText(int x, int y, LPCTSTR lpszText);//显示图象2D汉字
    void Show2DGbkText(char *str);//显示图形2D汉字
    void Show3DText(unsigned char *str);//显示图形3D汉字
    void SetArea(float width, float height, float front, float back);

private:
    unsigned char m_red;
    unsigned char m_green;
    unsigned char m_blue;
    float m_width;//宽度(世界坐标系，非窗口)
    float m_height;//高度(世界坐标系, 非窗口)
    float m_front;
    float m_back;
};

#endif
