
#ifndef FORCE_SERVER_GRAPHICS_H
#define FORCE_SERVER_GRAPHICS_H

#include <string>

// OpenGL includes
#if defined(WIN32) || defined(linux)
# include <GL/glut.h>
#elif defined(__APPLE__)
# include <GLUT/glut.h>
#endif

#include "event_mgr.h"

class Phantom;

class Graphics {
public:
    // Pass a ptr to the Phantom object so Graphics can query it for its state and draw feedback
    Graphics(EventMgr* event_mgr, Phantom* phantom);
    virtual ~Graphics();
    
    bool Init(int argc, char** argv);
    void Run(void (*mainloop_func)(void));
    
    void Draw();
    
protected:
    EventMgr *event_mgr_;
    Phantom *phantom_;
    
    GLuint cursor_display_list_;
};

#endif
