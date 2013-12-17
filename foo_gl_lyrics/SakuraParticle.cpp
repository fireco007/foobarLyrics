#include "SakuraParticle.h"
#include <stdio.h>
#include <time.h>
#include <GL/gl.h> 
#include <GL/glu.h> 
#include <gl/glaux.h>

SakuraParticle *SakuraParticle::m_handle = NULL;

SakuraParticle::SakuraParticle(void)
{
}


SakuraParticle::~SakuraParticle(void)
{
    //if (m_handle != NULL) {
    //    delete m_handle;
    //    m_handle = NULL;
    //}
}

SakuraParticle *SakuraParticle::getInstance()
{
    if (m_handle == NULL) {
        m_handle = new SakuraParticle;
    }

    return m_handle;
}

void SakuraParticle::destroyInstance()
{
    if (m_handle != NULL) {
        delete m_handle;
        m_handle = NULL;
    }
}

void SakuraParticle::setRect(float width, float height, float front, float back)
{
    m_width = width;
    m_height = height;
    m_front = front;
    m_back = back;
}

VOID SakuraParticle::timerCallback(HWND wnd, UINT id, UINT_PTR ptr, DWORD unknown)
{
    SakuraParticle *instance = getInstance();
    instance->updateParticles();
    instance->sendPaintMsg();
}

void SakuraParticle::sendPaintMsg()
{
    InvalidateRect(m_wnd, NULL, TRUE);
}

void SakuraParticle::initParticles()
{
    memset(m_particles, 0, sizeof(PARTICLES) * MAX_PARTICLES);

    for (int i = 0; i < MAX_PARTICLES; ++i) {
        m_deadParticles.push_back(m_particles + i);
    }

    loadTexture("E:\\hana.bmp");
}

void SakuraParticle::drawParticles()
{
    const float fSize = 0.15f;
    glLoadIdentity();

    glColor4f(1.0f, 0.6f, 0.6f, 1.0f);

    std::list<PARTICLES*>::iterator activeIte = m_activeParticles.begin();
    for (; activeIte != m_activeParticles.end(); ++activeIte) {

        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2d(1, 1);
        glVertex3f((*activeIte)->x + fSize, (*activeIte)->y + fSize, (*activeIte)->z);
        glTexCoord2d(0, 1);
        glVertex3f((*activeIte)->x - fSize, (*activeIte)->y + fSize, (*activeIte)->z);
        glTexCoord2d(1, 0);
        glVertex3f((*activeIte)->x + fSize, (*activeIte)->y - fSize, (*activeIte)->z);
        glTexCoord2d(0, 0);
        glVertex3f((*activeIte)->x - fSize, (*activeIte)->y - fSize, (*activeIte)->z);

        glEnd();
    }
}

void SakuraParticle::updateParticles()
{
    //random generate new particles
    if ((rand() % 2) == 0 ) {

        std::list<PARTICLES*>::iterator deadIte = m_deadParticles.begin();
        if (deadIte != m_deadParticles.end()) {

            //init this particles
            //(*deadIte)->y = (rand() % (int)(m_height * 100)) / 100.0f - m_height / 2;
            //(*deadIte)->z = -((rand() % (int)(m_back - m_front * 100)) / 100.0f + m_front);
            //(*deadIte)->x = -m_width / 2;
            //(*deadIte)->a = (rand() % 100) / 100.0f;
            //(*deadIte)->xi = (rand() % 100) / 1000.0f + 0.1;

            (*deadIte)->y = m_height / 2  - 1.5f;
            (*deadIte)->z = -m_front;
            (*deadIte)->x = -m_width / 2;
            (*deadIte)->a = (rand() % 98) / 100.0f + 0.02f;

            (*deadIte)->xi = 0.01f;
            (*deadIte)->zi = -0.1f;
            (*deadIte)->yi = 0.0f;
            m_activeParticles.push_back(*deadIte);
            deadIte = m_deadParticles.erase(deadIte);
        }
    }

    if ((rand() % 2) == 0 ) {

        std::list<PARTICLES*>::iterator deadIte = m_deadParticles.begin();
        if (deadIte != m_deadParticles.end()) {

            //init this particles
            (*deadIte)->y = - (m_height / 2)  + 1.5f;
            (*deadIte)->z = -m_back;
            (*deadIte)->x = -m_width / 2;
            (*deadIte)->a = (rand() % 98) / 100.0f + 0.02f;

            (*deadIte)->xi = 0.01f;
            (*deadIte)->zi = 0.1f;
            (*deadIte)->yi = 3.1415926535f / 2;
            m_activeParticles.push_back(*deadIte);
            deadIte = m_deadParticles.erase(deadIte);
        }
    }

    //update the status of displaying particles
    std::list<PARTICLES*>::iterator activeIte = m_activeParticles.begin();
    while (activeIte != m_activeParticles.end()) {

        //update x coordinate
        (*activeIte)->x += (*activeIte)->xi;

        //update z coordinate
        (*activeIte)->z += (*activeIte)->zi;
        if ((*activeIte)->z - m_back >= 0.000001f) {
            (*activeIte)->zi = 0.1f;
        }

        if (m_front - (*activeIte)->z >= 0.000001f) {
            (*activeIte)->zi = -0.1f;
        }

        //update y coordinate
        (*activeIte)->yi += 0.1f;
        (*activeIte)->y = cos((*activeIte)->yi) * (m_height / 2);

        if ((*activeIte)->x > (m_width / 2)) {
            m_deadParticles.push_back(*activeIte);
            activeIte = m_activeParticles.erase(activeIte);
        } else {
            ++activeIte;
        }
    }
}

void SakuraParticle::startDisplay(HWND wnd)
{
    m_wnd = wnd;
    initParticles();
    SetTimer(wnd, NULL, 30, timerCallback);
    srand((unsigned)time(NULL));
}

unsigned int SakuraParticle::loadTexture(const char *imgFile)
{

#define BMP_HEAD_SIZE 54

    unsigned int texture;

    FILE *img = fopen(imgFile, "rb");

    if (img == NULL) {
        return 0;
    }

    //read the bmp size
    unsigned long imgWidth = 0;
    unsigned long imgHeight = 0;
    DWORD size = 0;
    fseek(img, 18, SEEK_SET);
    fread(&imgWidth, 4, 1, img);
    fread(&imgHeight, 4, 1, img);
    fseek(img, 0, SEEK_END);
    size = ftell(img) - BMP_HEAD_SIZE;

    //read the bmp data
    unsigned char *imgData = (unsigned char*)malloc(size);
    fseek(img, BMP_HEAD_SIZE, SEEK_SET);
    fread(imgData, size, 1, img);

    fclose(img);

    //unsigned char imgDataWithAlpha[64][64][4];
    //for (int i = 0; i < 64; i++) {
    //    for (int j = 0; j < 64; j++) {
    //        imgDataWithAlpha[i][j][0] = (unsigned char)*(imgData + i * 64 * 3 + j * 3 + 2);
    //        imgDataWithAlpha[i][j][1] = (unsigned char)*(imgData + i * 64 * 3 + j * 3 + 1);
    //        imgDataWithAlpha[i][j][2] = (unsigned char)*(imgData + i * 64 * 3 + j * 3);

    //        if(imgDataWithAlpha[i][j][0] == 255 && imgDataWithAlpha[i][j][1] == 255 && imgDataWithAlpha[i][j][2] == 255)
    //            imgDataWithAlpha[i][j][3] = 0;//set the white color's alpha to 0
    //        else
    //            imgDataWithAlpha[i][j][3] = 255;
    //    }
    //}

    //load bmp data into texture;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //we got a good video card, so let's use GL_LINEAR!
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    //gluBuild2DMipmaps(GL_TEXTURE_2D, 4, imgWidth, imgHeight, GL_RGBA, GL_UNSIGNED_BYTE, imgDataWithAlpha);
    
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, imgWidth, imgHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, imgData);
    if (imgData) {
        free(imgData);
    }

    return texture;
}

