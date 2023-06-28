/**
  This server program uses the OpenHaptics library to render haptics to a PHANToM force feedback device.  OpenHaptics is an
  older library that interfaces with OpenGL 2.0.  It can, very nicely, render haptic effects that are "drawn" to the screen using OpenGL
  commands, like glBegin(), glVertex3f(), glEnd(); however, this is old-school OpenGL and won't work directly with a more recent
  OpenGL 3.0+ library.  Thus, the program creates an old-school OpenGL context with the good-old GLUT library.
 */

#include <iostream>

#include <minvr3.h>

#include "event_mgr.h"
#include "graphics.h"
#include "phantom.h"

// defaults that can be adjusted via command line options
int port = 9034;



// global vars
EventMgr* event_mgr;
Phantom* phantom;
Graphics* graphics;
SOCKET listener_fd = INVALID_SOCKET;
SOCKET client_fd = INVALID_SOCKET;

void mainloop() {
    // check for a new connection -- only one connection at a time is allowed.  new connections
    // replace the old and reset the system.
    if (MinVR3Net::IsReadyToRead(&listener_fd)) {
        SOCKET new_client_fd;
        if (MinVR3Net::TryAcceptConnection(listener_fd, &new_client_fd)) {
            if (client_fd == INVALID_SOCKET) {
                // no existing client, initialize
                phantom->Init();
            }
            else {
                // replace the previous client, reset
                MinVR3Net::CloseSocket(&client_fd);
                phantom->Reset();
            }
            client_fd = new_client_fd;
        }
    }
    
    // collect VREvents from input devices and over the net
    phantom->PollForInput();
    if (client_fd != INVALID_SOCKET) {
        while (MinVR3Net::IsReadyToRead(&client_fd)) {
            VREvent* e = MinVR3Net::ReceiveVREvent(&client_fd);
            event_mgr->QueueEvent(e);
            std::cout << "Received: " << *e << std::endl;
        }
    }
    
    // respond to VREvents, with event_mgr calling any listeners who have subscribed to events
    event_mgr->ProcessQueue();
    
    // input devices and listeners may have generated new events or status messages to send back
    // the the client, send any of those now.
    event_mgr->ProcessClientQueue(&client_fd);
    
    // render the frame (haptics and graphics)
    phantom->Draw();
    graphics->Draw();
}


 
int main(int argc, char** argv) {
    // optionally, override defaults with command line options
    if (argc > 1) {
        std::string arg = argv[1];
        if ((arg == "help") || (arg == "-h") || (arg == "-help") || (arg == "--help")) {
            std::cout << "Usage: force_server [port]" << std::endl;
            std::cout << "  * port is the port on localhost clients should connect to, defaults to " << port << std::endl;
            std::cout << "  * Quits if an event named 'Shutdown' is received, or press Ctrl-C" << std::endl;
            exit(0);
        }
        port = std::stoi(arg);
    }
    
    event_mgr = new EventMgr();
    phantom = new Phantom(event_mgr);
    graphics = new Graphics(event_mgr, phantom);
    
    graphics->Init(argc, argv);
    phantom->Init();
    
    MinVR3Net::Init();
    MinVR3Net::CreateListener(port, &listener_fd);
    
    graphics->Run(mainloop);
    
    delete phantom;
    delete graphics;
    MinVR3Net::CloseSocket(&listener_fd);
    MinVR3Net::CloseSocket(&client_fd);
    MinVR3Net::Shutdown();
    return 0;
}
