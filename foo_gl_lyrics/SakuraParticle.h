#ifndef _SAKURA_PARTICLE_H_
#define _SAKURA_PARTICLE_H_

#include <Windows.h>
#include <list>
#include "LyricsCommon.h"

#define MAX_PARTICLES 1000

typedef struct  
{
    bool active;
    float life;
    float org_x;//store the original value of x
    float angle;
    
    //color
    float r;
    float g;
    float b;

    //alpha(transparency)
    float a;

    //position
    float x;
    float y;
    float z;

    //speed
    float xi;
    float zi;

    //rotate speed
    float rx;
    float ry; 
    
} PARTICLES;

//sakura particle system
class SakuraParticle
{

public:
    virtual ~SakuraParticle(void);

    void clearParticles();

    void setGlCtx(GL_CONTEXT *glCtx);

    void startDisplay(HWND wnd);

    static SakuraParticle *getInstance();
    
    static void destroyInstance();

    void updateParticles();

    void sendPaintMsg();

    void drawParticles();

private:

    static VOID CALLBACK timerCallback(HWND wnd, UINT id, UINT_PTR ptr, DWORD unknown);

    void initParticles();

    //only support 32bit(bgra) bmp
    unsigned int loadTexture(const char *imgFile);

    unsigned int loadPNGTexture(const char *imgFile);

    SakuraParticle(void);

private:
    static SakuraParticle *m_handle;
    HWND m_wnd;
    unsigned int m_period; //refresh peroid of particles
    PARTICLES m_particles[MAX_PARTICLES];
    std::list<PARTICLES*> m_activeParticles;
    std::list<PARTICLES*> m_deadParticles;

    GL_CONTEXT *m_glCtx;
};

#endif //_SAKURA_PARTICLE_H_
