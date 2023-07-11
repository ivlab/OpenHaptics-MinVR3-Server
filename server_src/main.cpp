/**
  This server program uses the OpenHaptics library to render haptics to a PHANToM force feedback device.  OpenHaptics is an
  older library that interfaces with OpenGL 2.0.  It can, very nicely, render haptic effects that are "drawn" to the screen using OpenGL
  commands, like glBegin(), glVertex3f(), glEnd(); however, this is old-school OpenGL and won't work directly with a more recent
  OpenGL 3.0+ library.  Thus, the program creates an old-school OpenGL context with the good-old GLUT library.
 */

// including first to avoid any issues with include ordering with windows.h, glut, etc.
#include "open_haptics.h"

#include <iostream>

#include <minvr3.h>

#include "ambient_friction.h"
#include "ambient_viscous.h"
#include "event_mgr.h"
#include "line_constraint.h"
#include "phantom.h"
#include "point_constraint.h"
#include "surface_constraint.h"
#include "surface_contact.h"


// can be adjusted via command line options
int port = 9034;

// global vars
EventMgr* event_mgr;
Phantom* phantom;
SOCKET listener_fd = INVALID_SOCKET;
SOCKET client_fd = INVALID_SOCKET;


// force effects -- designed so that more can be easily added here
void registerForceEffects() {
    phantom->RegisterForceEffect(new AmbientFriction(event_mgr));
    phantom->RegisterForceEffect(new AmbientViscous(event_mgr));
    phantom->RegisterForceEffect(new PointConstraint(event_mgr));
    phantom->RegisterForceEffect(new LineConstraint(event_mgr));
    phantom->RegisterForceEffect(new SurfaceConstraint(event_mgr));
    phantom->RegisterForceEffect(new SurfaceContact(event_mgr));
}


// callbacks from glut/c++
void glutDisplay(void);
void glutReshape(int width, int height);
void glutIdle(void);
void glutMenu(int);
void exitHandler(void);

// graphics routines
void initGraphics();
void drawGraphics();
void shutdownGraphics();

// network routines
void initNetwork();
void readFromClient();
void writeToClient();
void shutdownNetwork();

void drawLine();

#define CAMERA_Z 600
#define CAMERA_FOV 50

int main(int argc, char* argv[])
{
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
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("Force Server");

    // Set glut callback functions.
    glutDisplayFunc(glutDisplay);
    glutReshapeFunc(glutReshape);
    glutIdleFunc(glutIdle);

    // Add a simple menu
    glutCreateMenu(glutMenu);
    glutAddMenuEntry("Quit", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Provide a cleanup routine for handling application exit.
    atexit(exitHandler);

    // Initialize main systems that need to work together
    initNetwork();
    initGraphics();
    phantom->Init();
    registerForceEffects();

    // Open window, this will start calling the callbacks registered earlier to draw, etc.
    glutMainLoop();

    // Note: all cleanup is done in the exitHandler
    return 0;
}

void exitHandler() {
    shutdownNetwork();
    shutdownGraphics();
    delete phantom;
}



// GLUT CALLBACK FUNCTIONS

// this is essentially the program's mainloop
void glutDisplay() {

    // check for new connections and receive any pending events from the client
    readFromClient();
    phantom->CheckHapticError();

    // collect input events from the phantom
    phantom->PollForInput();
    phantom->CheckHapticError();

    // process events received, either over the network or user input, since the last frame.
    // since this comes after BeginHapticFrame(), it is ok to include OpenHaptics calls within
    // any event callback routines.
    event_mgr->ProcessQueue();
    phantom->CheckHapticError();


    // HAPTICS RENDERING PASS

    // OpenHaptics reads the GL modelview matrix at the beginning of each haptic frame and then
    // automatically applies offsets to the stylus position (and maybe some other things) based
    // on this.  This may work ok for quickly converting an existing OpenGL app to use haptics,
    // but it is super confusing for a server-based application like this one when the haptic
    // rendering is auto-magically impacted by the graphics rendering state!  Thus, our approach
    // is the load the identity matrix into the OpenGL modelview before starting the haptic frame.
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    phantom->BeginHapticFrame();
    phantom->CheckHapticError();

    phantom->DrawHaptics();
    phantom->CheckHapticError();

    phantom->EndHapticFrame();
    phantom->CheckHapticError();


    // GRAPHICS RENDERING PASS

    // Add a view matrix to the ModelView that positions the camera at some positive z value,
    // looking in the -Z direction toward the origin.  The CAMERA_Z and CAMERA_FOV values
    // should be set so that we get a good view of the entire custom_workspace volume.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    hduVector3Dd cam_pos(0, 0, CAMERA_Z);
    hduVector3Dd origin(0, 0, 0);
    hduVector3Dd up(0, 1, 0);
    gluLookAt(cam_pos[0], cam_pos[1], cam_pos[2],
              origin[0], origin[1], origin[2],
              up[0], up[1], up[2]);

    // draw graphics to the screen
    phantom->DrawGraphics();
    drawGraphics();
    phantom->CheckHapticError();
    glutSwapBuffers();
    
    // send any events generated for the client during this frame
    writeToClient();
} 


void glutIdle() {
    //phantom->CheckHapticError();
    glutPostRedisplay();
}

void glutMenu(int ID) {
    switch (ID) {
    case 0:
        exit(0);
        break;
    }
}

void glutReshape(int width, int height) {
    // Sets up a perspective camera that works well for looking at the haptic workspace in its
    // default units (mm).  The workspace is a non-uniform box.  The x dimension is the largest
    // and ranges from about -300mm to +300mm.  This view volume will include at least a box of
    // this size in all 3 dimensions.
    hduVector3Dd cam_pos(0, 0, CAMERA_Z);
    hduVector3Dd origin(0, 0, 0);
    hduVector3Dd up(0, 1, 0);
    double near_dist = 1;
    double far_dist = cam_pos[2] + 400;
    double fov = CAMERA_FOV;
    double aspect = (double)width / height;
    
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, near_dist, far_dist);
}


// MAIN GRAPHICS FUNCTIONS


void initGraphics() {
    static const GLfloat light_model_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    static const GLfloat light0_diffuse[] = { 0.9f, 0.9f, 0.9f, 0.9f };
    static const GLfloat light0_direction[] = { 0.0f, 1.0f, 1.0f, 0.0f };

    // Enable depth buffering for hidden surface removal.
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    // Cull back faces.
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // Setup other misc features.
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);

    // Setup lighting model.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
    glEnable(GL_LIGHT0);
}

void drawGraphics() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draws a cursor, outline of the haptic workspace, and any graphics defined by
    // the active force effects
    phantom->DrawGraphics();
    
    // draw any other elements of the scene...
    //glutSolidSphere(200, 32, 32);
}

void shutdownGraphics() {
    
}



// MAIN NETWORK FUNCTIONS

void initNetwork() {
    MinVR3Net::Init();
    MinVR3Net::CreateListener(port, &listener_fd);
}

void readFromClient() {
    // check for a new connection -- only one connection at a time is allowed.  new connections
    // replace the old and reset the system.
    if (MinVR3Net::IsReadyToRead(&listener_fd)) {
        SOCKET new_client_fd;
        if (MinVR3Net::TryAcceptConnection(listener_fd, &new_client_fd)) {
            phantom->Reset();
            client_fd = new_client_fd;
            //std::cout << "New Connection" << std::endl;
        }
    }
    
    // if a connection is active, receive any pending events
    if (client_fd != INVALID_SOCKET) {
        while (MinVR3Net::IsReadyToRead(&client_fd)) {
            VREvent* e = MinVR3Net::ReceiveVREvent(&client_fd);
            if (e != NULL) {
                event_mgr->QueueEvent(e);
                //std::cout << "Received: " << *e << std::endl;
            }
        }
    }
}

void writeToClient() {
    // input devices and listeners may have generated new events or status messages to send back
    // to the client
    event_mgr->ProcessClientQueue(&client_fd);
}

void shutdownNetwork() {
    MinVR3Net::CloseSocket(&listener_fd);
    MinVR3Net::CloseSocket(&client_fd);
    MinVR3Net::Shutdown();
}
