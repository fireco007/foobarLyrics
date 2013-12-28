#ifndef __LYRICS_COMMON_H__
#define __LYRICS_COMMON_H__

#define PI 3.1415926f

//left = - right; bottom = - top;
struct GL_CONTEXT
{   
    float fRight;
    float fTop;
    
    float fNear;
    float fFar;

    float getWidth()
    {
        return 2 * fRight;
    }

    float getHeight()
    {
        return 2 * fTop;
    }
};

#endif //__LYRICS_COMMON_H__
