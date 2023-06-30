/**
  This server program uses the OpenHaptics library to render haptics to a PHANToM force feedback device.  OpenHaptics is an
  older library that interfaces with OpenGL 2.0.  It can, very nicely, render haptic effects that are "drawn" to the screen using OpenGL
  commands, like glBegin(), glVertex3f(), glEnd(); however, this is old-school OpenGL and won't work directly with a more recent
  OpenGL 3.0+ library.  Thus, the program creates an old-school OpenGL context with the good-old GLUT library.
 */


/***

#include <iostream>

#include <minvr3.h>

#include "ambient_friction.h"
#include "ambient_viscous.h"
#include "event_mgr.h"
#include "graphics.h"
#include "line_constraint.h"
#include "phantom.h"
#include "point_constraint.h"
#include "surface_constraint.h"
#include "touch_surface.h"


// defaults that can be adjusted via command line options
int port = 9034;


// global vars
EventMgr* event_mgr;
Phantom* phantom;
Graphics* graphics;
SOCKET listener_fd = INVALID_SOCKET;
SOCKET client_fd = INVALID_SOCKET;


// force effects -- designed so that more can be easily added here
void register_force_effects() {
    phantom->BeginHapticFrame();
    phantom->RegisterForceEffect("AmbientFriction", new AmbientFriction(event_mgr));
    phantom->RegisterForceEffect("AmbientViscous", new AmbientViscous(event_mgr));
    phantom->RegisterForceEffect("PointConstraint", new PointConstraint(event_mgr));
    phantom->EndHapticFrame();
}

HLuint e = 9999;
HLuint c = 9999;

void mainloop() {

    phantom->BeginHapticFrame();
    phantom->CheckHapticError();
    phantom->PollForInput();
    phantom->CheckHapticError();

    graphics->Clear();
    graphics->DrawGraphics();
    graphics->SwapBuffers();

    phantom->CheckHapticError();
    phantom->DrawHaptics();
    phantom->CheckHapticError();


    // check for a new connection -- only one connection at a time is allowed.  new connections
    // replace the old and reset the system.
    if (MinVR3Net::IsReadyToRead(&listener_fd)) {
        SOCKET new_client_fd;
        if (MinVR3Net::TryAcceptConnection(listener_fd, &new_client_fd)) {
            phantom->Reset();
            client_fd = new_client_fd;
            std::cout << "New Connection" << std::endl;
        }
    }


    // collect VREvents from input devices and over the net

    if (client_fd != INVALID_SOCKET) {
        while (MinVR3Net::IsReadyToRead(&client_fd)) {
            VREvent* e = MinVR3Net::ReceiveVREvent(&client_fd);
            if (e != NULL) {
                event_mgr->QueueEvent(e);
                std::cout << "Received: " << *e << std::endl;
            }
        }
    }

    // respond to VREvents, with event_mgr calling any listeners who have subscribed to events
    event_mgr->ProcessQueue();
    phantom->CheckHapticError();

    // input devices and listeners may have generated new events or status messages to send back
    // the the client, send any of those now.
    event_mgr->ProcessClientQueue(&client_fd);
    phantom->CheckHapticError();




    
    if (e == 9999) {
        e = hlGenEffects(1);
        hlEffectd(HL_EFFECT_PROPERTY_GAIN, 0.8);
        hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, 1.0);
        hlStartEffect(HL_EFFECT_VISCOUS, e);
    }

    
    if (c == 9999) {
        c = hlGenShapes(1);
    }
    
    if (phantom->is_primary_btn_down()) {

        //hlMaterialf(HL_FRONT, HL_STIFFNESS, 0.2);
        //hlMaterialf(HL_FRONT, HL_DAMPING, 0.2);
        //hlMaterialf(HL_FRONT, HL_STATIC_FRICTION, 0.2);
       // hlMaterialf(HL_FRONT, HL_DYNAMIC_FRICTION, 0.2);

        *
        hlTouchModelf(HL_SNAP_DISTANCE, 2.0);
        hlTouchModel(HL_CONSTRAINT);
        hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, c);
        glBegin(GL_POINTS);
        glVertex3f(phantom->position()[0], phantom->position()[1], phantom->position()[2]);
        glEnd();
        hlEndShape();
        *

        std::cout << phantom->position()[0] << " " << phantom->position()[1] << " " << phantom->position()[2] << std::endl;
        hlTouchModel(HL_CONTACT);
        hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, c);
        glBegin(GL_TRIANGLES);
        glNormal3f(0, 0, 1);
        glVertex3d(100, 100, 0);
        glVertex3d(-100, 100, 0);
        glVertex3d(0, -100, 0);
        glEnd();
        hlEndShape();
    }
    
    
    //hlBeginShape(HL_SHAPE_DEPTH_BUFFER, myShapeId);
    //drawMyShapeHaptic();
    //hlEndShape();
    
    // flush haptics changes
    phantom->EndHapticFrame();
    phantom->CheckHapticError();


    *
    phantom->BeginHapticFrame();

    // check for a new connection -- only one connection at a time is allowed.  new connections
    // replace the old and reset the system.
    if (MinVR3Net::IsReadyToRead(&listener_fd)) {
        SOCKET new_client_fd;
        if (MinVR3Net::TryAcceptConnection(listener_fd, &new_client_fd)) {
            phantom->Reset();
            client_fd = new_client_fd;
            std::cout << "New Connection" << std::endl; 
        }
    }
    

    // collect VREvents from input devices and over the net
    phantom->PollForInput();
    if (client_fd != INVALID_SOCKET) {
        while (MinVR3Net::IsReadyToRead(&client_fd)) {
            VREvent* e = MinVR3Net::ReceiveVREvent(&client_fd);
            if (e != NULL) {
                event_mgr->QueueEvent(e);
                std::cout << "Received: " << *e << std::endl;
            }
        }
    }
    
    // respond to VREvents, with event_mgr calling any listeners who have subscribed to events
    event_mgr->ProcessQueue();
    
    // input devices and listeners may have generated new events or status messages to send back
    // the the client, send any of those now.
    event_mgr->ProcessClientQueue(&client_fd);
    
    // render the frame (haptics and graphics)
    phantom->DrawHaptics();
    graphics->DrawGraphics();


    glutPostRedisplay();
    glutSwapBuffers();
    phantom->EndHapticFrame();
    phantom->CheckHapticError();

    *
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
    register_force_effects();

    MinVR3Net::Init();
    MinVR3Net::CreateListener(port, &listener_fd);
    
    graphics->Run(mainloop); // does not return until window is closed
    
    delete phantom;
    delete graphics;
    MinVR3Net::CloseSocket(&listener_fd);
    MinVR3Net::CloseSocket(&client_fd);
    MinVR3Net::Shutdown();
    return 0;
}
**/



/*****************************************************************************

Copyright (c) 2004 SensAble Technologies, Inc. All rights reserved.

OpenHaptics(TM) toolkit. The material embodied in this software and use of
this software is subject to the terms and conditions of the clickthrough
Development License Agreement.

For questions, comments or bug reports, go to forums at:
    http://dsc.sensable.com

Module Name:

  HelloSphere.cpp

Description:

  This example demonstrates basic haptic rendering of a shape.

******************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <assert.h>

#if defined(WIN32)
#include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
#include <GL/glut.h>
#elif defined(__APPLE__)
#include <GLUT/glut.h>
#endif

#include <HL/hl.h>
#include <HDU/hduMatrix.h>
#include <HDU/hduError.h>

#include <HLU/hlu.h>

/* Haptic device and rendering context handles. */
static HHD ghHD = HD_INVALID_HANDLE;
static HHLRC ghHLRC = 0;

/* Shape id for shape we will render haptically. */
HLuint gSphereShapeId = 0;
HLuint gLineShapeId = 0;
static double gLineShapeSnapDistance = 0.0;
HLuint gFrictionId = 0;

#define CURSOR_SIZE_PIXELS 20
static double gCursorScale;
static GLuint gCursorDisplayList = 0;

/* Function prototypes. */
void glutDisplay(void);
void glutReshape(int width, int height);
void glutIdle(void);
void glutMenu(int);

void exitHandler(void);

void initGL();
void initHL();
void initScene();
void drawSceneHaptics();
void drawSceneGraphics();
void drawCursor();
void updateWorkspace();

/*******************************************************************************
 Initializes GLUT for displaying a simple haptic scene.
*******************************************************************************/
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(500, 500);
    glutCreateWindow("HelloSphere Example");

    // Set glut callback functions.
    glutDisplayFunc(glutDisplay);
    glutReshapeFunc(glutReshape);
    glutIdleFunc(glutIdle);

    glutCreateMenu(glutMenu);
    glutAddMenuEntry("Quit", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Provide a cleanup routine for handling application exit.
    atexit(exitHandler);

    initScene();

    glutMainLoop();

    return 0;
}

/*******************************************************************************
 GLUT callback for redrawing the view.
*******************************************************************************/
void glutDisplay()
{
    drawSceneHaptics();

    drawSceneGraphics();

    glutSwapBuffers();
}

/*******************************************************************************
 GLUT callback for reshaping the window.  This is the main place where the
 viewing and workspace transforms get initialized.
*******************************************************************************/
void glutReshape(int width, int height)
{
    static const double kPI = 3.1415926535897932384626433832795;
    static const double kFovY = 40;

    double nearDist, farDist, aspect;

    glViewport(0, 0, width, height);

    // Compute the viewing parameters based on a fixed fov and viewing
    // a canonical box centered at the origin.

    nearDist = 500;// 1.0 / tan((kFovY / 2.0) * kPI / 180.0);
    farDist = 1500;// nearDist + 2.0;
    aspect = (double)width / height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(kFovY, aspect, nearDist, farDist);

    // Place the camera down the Z axis looking at the origin.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 1000,
        0, 0, 0,
        0, 1, 0);

    updateWorkspace();
}

/*******************************************************************************
 GLUT callback for idle state.  Use this as an opportunity to request a redraw.
 Checks for HLAPI errors that have occurred since the last idle check.
*******************************************************************************/
void glutIdle()
{
    HLerror error;

    while (HL_ERROR(error = hlGetError()))
    {
        fprintf(stderr, "HL Error: %s\n", error.errorCode);

        if (error.errorCode == HL_DEVICE_ERROR)
        {
            hduPrintError(stderr, &error.errorInfo,
                "Error during haptic rendering\n");
        }
    }

    glutPostRedisplay();
}

/******************************************************************************
 Popup menu handler.
******************************************************************************/
void glutMenu(int ID)
{
    switch (ID) {
    case 0:
        exit(0);
        break;
    }
}

/*******************************************************************************
 Initializes the scene.  Handles initializing both OpenGL and HL.
*******************************************************************************/
void initScene()
{
    initGL();
    initHL();
}

/*******************************************************************************
 Sets up general OpenGL rendering properties: lights, depth buffering, etc.
*******************************************************************************/
void initGL()
{
    static const GLfloat light_model_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    static const GLfloat light0_diffuse[] = { 0.9f, 0.9f, 0.9f, 0.9f };
    static const GLfloat light0_direction[] = { 0.0f, -0.4f, 1.0f, 0.0f };

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

    // Setup lighting model.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
    glEnable(GL_LIGHT0);
}

/*******************************************************************************
 Initialize the HDAPI.  This involves initing a device configuration, enabling
 forces, and scheduling a haptic thread callback for servicing the device.
*******************************************************************************/
void initHL()
{
    HDErrorInfo error;

    ghHD = hdInitDevice(HD_DEFAULT_DEVICE);
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        hduPrintError(stderr, &error, "Failed to initialize haptic device");
        fprintf(stderr, "Press any key to exit");
        getchar();
        exit(-1);
    }

    ghHLRC = hlCreateContext(ghHD);
    hlMakeCurrent(ghHLRC);

    // Enable optimization of the viewing parameters when rendering
    // geometry for OpenHaptics.
    hlEnable(HL_HAPTIC_CAMERA_VIEW);

    // Generate id for the shape.
    gSphereShapeId = hlGenShapes(1);


    // AMBIENT EFFECT EXAMPLE

    gFrictionId = hlGenEffects(1);
    hlBeginFrame();
    hlEffectd(HL_EFFECT_PROPERTY_GAIN, 0.05);
    hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, 0.1);
    hlStartEffect(HL_EFFECT_FRICTION, gFrictionId);
    hlEndFrame();

    gLineShapeId = hlGenShapes(1);

    HDdouble kStiffness;
    hdGetDoublev(HD_NOMINAL_MAX_STIFFNESS, &kStiffness);

    // We can get a good approximation of the snap distance to use by
    // solving the following simple force formula: 
    // >  F = k * x  <
    // F: Force in Newtons (N).
    // k: Stiffness control coefficient (N/mm).
    // x: Displacement (i.e. snap distance).
    const double kLineShapeForce = 7.0;
    gLineShapeSnapDistance = kLineShapeForce / kStiffness;

    hlTouchableFace(HL_FRONT);
}

/*******************************************************************************
 This handler is called when the application is exiting.  Deallocates any state
 and cleans up.
*******************************************************************************/
void exitHandler()
{
    // Deallocate the sphere shape id we reserved in initHL.
    hlDeleteShapes(gSphereShapeId, 1);

    // Free up the haptic rendering context.
    hlMakeCurrent(NULL);
    if (ghHLRC != NULL)
    {
        hlDeleteContext(ghHLRC);
    }

    // Free up the haptic device.
    if (ghHD != HD_INVALID_HANDLE)
    {
        hdDisableDevice(ghHD);
    }
}

/*******************************************************************************
 Use the current OpenGL viewing transforms to initialize a transform for the
 haptic device workspace so that it's properly mapped to world coordinates.
*******************************************************************************/
void updateWorkspace()
{
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    HLdouble maxWorkspaceDims[6];
    hlGetDoublev(HL_MAX_WORKSPACE_DIMS, maxWorkspaceDims);

    HLdouble size[3];
    size[0] = maxWorkspaceDims[3] - maxWorkspaceDims[0];
    size[1] = maxWorkspaceDims[4] - maxWorkspaceDims[1];
    size[2] = maxWorkspaceDims[5] - maxWorkspaceDims[2];

    HLdouble center[3];
    center[0] = maxWorkspaceDims[0] + size[0] / 2.0;
    center[1] = maxWorkspaceDims[1] + size[1] / 2.0;
    center[2] = maxWorkspaceDims[2] + size[2] / 2.0;

    hlMatrixMode(HL_TOUCHWORKSPACE);
    hlLoadIdentity();

    // Not sure why the origin of the Phantom's workspace is not at the center of the workspace box.
    // This translation has the effect of placing the origin (0,0,0) at the center of the workable space
    // of the phantom.  So haptic shapes rendered at (0,0,0) are actually in the middle of the workspace.
    hlTranslated(-center[0], -center[1], -center[2]);

    // This does some magic to map the haptic workspace to the graphics view volume.  It seems to work
    // well aside from the need to add the translation above.
    hluFitWorkspace(projection);

    // Compute cursor scale to get a consistently sized cursor relative to screen space.
    gCursorScale = hluScreenToModelScale(modelview, projection, viewport);
    gCursorScale *= CURSOR_SIZE_PIXELS;
}


void drawLine()
{
    static GLuint displayList = 0;

    if (displayList)
    {
        glCallList(displayList);
    }
    else
    {
        displayList = glGenLists(1);
        glNewList(displayList, GL_COMPILE_AND_EXECUTE);
        glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING);
        glLineWidth(2.0);
        glBegin(GL_LINES);
        glVertex3f(-300, 250, 0);
        glVertex3f(300, 250, 0);
        glEnd();
        glPointSize(4);
        glBegin(GL_POINTS);
        glVertex3f(-300, 250, 0);
        glVertex3f(0, 250, 0);
        glVertex3f(300, 250, 0);
        glEnd();
        glPopAttrib();
        glEndList();
    }
}


/*******************************************************************************
 The main routine for displaying the scene.  Gets the latest snapshot of state
 from the haptic thread and uses it to display a 3D cursor.
*******************************************************************************/
void drawSceneGraphics()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw 3D cursor at haptic device position.
    drawCursor();



    // Draw a sphere using OpenGL.
    glutSolidSphere(200, 32, 32);
    
    glPushMatrix();
    drawLine();
    glPopMatrix();

    
    HLdouble maxWorkspaceDims[6];
    hlGetDoublev(HL_MAX_WORKSPACE_DIMS, maxWorkspaceDims);
    HLdouble size[3];
    size[0] = maxWorkspaceDims[3] - maxWorkspaceDims[0];
    size[1] = maxWorkspaceDims[4] - maxWorkspaceDims[1];
    size[2] = maxWorkspaceDims[5] - maxWorkspaceDims[2];
    HLdouble center[3];
    center[0] = maxWorkspaceDims[0] + size[0] / 2.0;
    center[1] = maxWorkspaceDims[1] + size[1] / 2.0;
    center[2] = maxWorkspaceDims[2] + size[2] / 2.0;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    //glTranslated(-center[0], -center[1], -center[2]);
    glScaled(size[0], size[1], size[2]);
    glDisable(GL_LIGHTING);
    glutWireCube(1);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    
}

/*******************************************************************************
 The main routine for rendering scene haptics.
*******************************************************************************/
void drawSceneHaptics()
{
    // Start haptic frame.  (Must do this before rendering any haptic shapes.)
    hlBeginFrame();


    // CONTACT EXAMPLE

    // Start a new haptic shape.  Use the feedback buffer to capture OpenGL 
    // geometry for haptic rendering.
    hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, gSphereShapeId);
    hlTouchModel(HL_CONTACT);
    // Set material properties for the shapes to be drawn.
    hlMaterialf(HL_FRONT_AND_BACK, HL_STIFFNESS, 0.7f);
    hlMaterialf(HL_FRONT_AND_BACK, HL_DAMPING, 0.1f);
    hlMaterialf(HL_FRONT_AND_BACK, HL_STATIC_FRICTION, 0.2f);
    hlMaterialf(HL_FRONT_AND_BACK, HL_DYNAMIC_FRICTION, 0.3f);
    // Use OpenGL commands to create geometry.
    glutSolidSphere(200, 32, 32);
    // End the shape.
    hlEndShape();


    // CONSTRAINT EXAMPLE

    glPushMatrix();
    hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, gLineShapeId);
    hlTouchModel(HL_CONSTRAINT);
    hlTouchModelf(HL_SNAP_DISTANCE, gLineShapeSnapDistance);
    hlMaterialf(HL_FRONT, HL_STIFFNESS, 0.2);
    hlMaterialf(HL_FRONT, HL_STATIC_FRICTION, 0);
    hlMaterialf(HL_FRONT, HL_DYNAMIC_FRICTION, 0);
    drawLine();
    hlEndShape();
    glPopMatrix();
    // End the haptic frame.
    hlEndFrame();
}


/*******************************************************************************
 Draws a 3D cursor for the haptic device using the current local transform,
 the workspace to world transform and the screen coordinate scale.
*******************************************************************************/
void drawCursor()
{
    static const double kCursorRadius = 0.5;
    static const double kCursorHeight = 1.5;
    static const int kCursorTess = 15;
    HLdouble proxyxform[16];

    GLUquadricObj* qobj = 0;

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
    glPushMatrix();

    if (!gCursorDisplayList)
    {
        gCursorDisplayList = glGenLists(1);
        glNewList(gCursorDisplayList, GL_COMPILE);
        qobj = gluNewQuadric();

        gluCylinder(qobj, 0.0, kCursorRadius, kCursorHeight,
            kCursorTess, kCursorTess);
        glTranslated(0.0, 0.0, kCursorHeight);
        gluCylinder(qobj, kCursorRadius, 0.0, kCursorHeight / 5.0,
            kCursorTess, kCursorTess);

        gluDeleteQuadric(qobj);
        glEndList();
    }

    // Get the proxy transform in world coordinates.
    hlGetDoublev(HL_PROXY_TRANSFORM, proxyxform);
    glMultMatrixd(proxyxform);

    // Apply the local cursor scale factor.
    glScaled(gCursorScale, gCursorScale, gCursorScale);

    glEnable(GL_COLOR_MATERIAL);
    HLboolean down;
    hlGetBooleanv(HL_BUTTON1_STATE, &down);
    if (down) {
        glColor3f(1.0, 0.5, 0.0);
    }
    else {
        glColor3f(0.0, 0.5, 1.0);
    }

    glCallList(gCursorDisplayList);

    glPopMatrix();
    glPopAttrib();
}

/******************************************************************************/