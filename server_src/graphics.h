
#ifndef FORCE_SERVER_GRAPHICS_H
#define FORCE_SERVER_GRAPHICS_H

#include <string>

#include "event_mgr.h"
#include "graphics_headers.h"

class Phantom;

class Graphics {
public:
    // Pass a ptr to the Phantom object so Graphics can query it for its state and draw feedback
    Graphics(EventMgr* event_mgr, Phantom* phantom);
    virtual ~Graphics();
    
    bool Init(int argc, char** argv);
    void Run(void (*mainloop_func)(void));
    
    void DrawGraphics();
    
protected:
    EventMgr *event_mgr_;
    Phantom *phantom_;
};

#endif
