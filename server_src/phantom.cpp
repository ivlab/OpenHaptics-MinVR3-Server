
#include "phantom.h"

#include <iostream>

#include "event_mgr.h"
#include "force_messages.h"



void HLCALLBACK Button1DownCallback(HLenum event, HLuint object, HLenum thread, HLcache *cache, void *userdata) {
    ((EventMgr*)userdata)->QueueEvent(new VREvent(ForceMessages::get_primary_btn_down_event_name()));
    ((EventMgr*)userdata)->QueueForClient(new VREvent(ForceMessages::get_primary_btn_down_event_name()));
}

void HLCALLBACK Button1UpCallback(HLenum event, HLuint object, HLenum thread, HLcache *cache, void *userdata) {
    ((EventMgr*)userdata)->QueueEvent(new VREvent(ForceMessages::get_primary_btn_up_event_name()));
    ((EventMgr*)userdata)->QueueForClient(new VREvent(ForceMessages::get_primary_btn_up_event_name()));
}

void HLCALLBACK MotionCallback(HLenum event, HLuint object, HLenum thread, HLcache *cache, void *userdata) {
    double pos[3];
    hlGetDoublev(HL_PROXY_POSITION, pos);
    ((EventMgr*)userdata)->QueueEvent(new VREventVector3(ForceMessages::get_position_update_event_name(), (float)pos[0], (float)pos[1], (float)pos[2]));
    //std::cout << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;
    ((EventMgr*)userdata)->QueueForClient(new VREventVector3(ForceMessages::get_position_update_event_name(), (float)pos[0], (float)pos[1], (float)pos[2]));

    double rot[4];
    hlGetDoublev(HL_PROXY_ROTATION, rot);
    ((EventMgr*)userdata)->QueueEvent(new VREventQuaternion(ForceMessages::get_rotation_update_event_name(), (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3]));
    ((EventMgr*)userdata)->QueueForClient(new VREventQuaternion(ForceMessages::get_rotation_update_event_name(), (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3]));
}


Phantom::Phantom(EventMgr* event_mgr) : hd_device_(HD_INVALID_HANDLE), hl_context_(0), event_mgr_(event_mgr) {
}

Phantom::~Phantom() {
    // once force effects have been registered with the phantom class, the phantom class is "in charge" of them,
    // so good practice to delete that memory when done.
    for (const auto &entry : effects_) {
        delete entry.second;
    }
    
    hlMakeCurrent(NULL);
    hlDeleteContext(hl_context_);
    hdDisableDevice(hd_device_);
}


bool Phantom::is_primary_btn_down() {
    HLboolean down;
    hlGetBooleanv(HL_BUTTON1_STATE, &down);
    return down;
}

bool Phantom::is_in_adjusted_workspace() {
    float xabs = adjusted_workspace_size_[0] / 2.0;
    float yabs = adjusted_workspace_size_[1] / 2.0;
    float zabs = adjusted_workspace_size_[2] / 2.0;
    if (position()[0] < -xabs) return false;
    if (position()[0] > xabs) return false;
    if (position()[1] < -yabs) return false;
    if (position()[1] > yabs) return false;
    if (position()[2] < -zabs) return false;
    if (position()[2] > zabs) return false;
    return true;
}


double* Phantom::transform() {
    hlGetDoublev(HL_PROXY_TRANSFORM, transform_);
    return (double*)transform_;
}

double* Phantom::position() {
    hlGetDoublev(HL_PROXY_POSITION, position_);
    return (double*)position_;
}


double* Phantom::rotation() {
    hlGetDoublev(HL_PROXY_ROTATION, rotation_);
    return (double*)rotation_;
}


bool Phantom::Init(const std::string &device_name) {
    HDErrorInfo error;

    hd_device_ = hdInitDevice(HD_DEFAULT_DEVICE);
    if (HD_DEVICE_ERROR(error = hdGetError())) {
        hduPrintError(stderr, &error, "Failed to initialize haptic device");
        fprintf(stderr, "Press any key to exit");
        getchar();
        exit(-1);
    }

    hl_context_ = hlCreateContext(hd_device_);
    hlMakeCurrent(hl_context_);
    CheckHapticError();

    // Enable optimization of the viewing parameters when rendering
    // geometry for OpenHaptics.
    hlEnable(HL_HAPTIC_CAMERA_VIEW);

    // setup callbacks for input from the phantom
    hlAddEventCallback(HL_EVENT_1BUTTONDOWN, HL_OBJECT_ANY, HL_CLIENT_THREAD, &Button1DownCallback, event_mgr_);
    hlAddEventCallback(HL_EVENT_1BUTTONUP, HL_OBJECT_ANY, HL_CLIENT_THREAD, &Button1UpCallback, event_mgr_);
    hlAddEventCallback(HL_EVENT_MOTION, HL_OBJECT_ANY, HL_CLIENT_THREAD, &MotionCallback, event_mgr_);

    // set thresholds for how much the phantom must before a new motion event is fired
    hlEventd(HL_EVENT_MOTION_LINEAR_TOLERANCE, 1.0);   // default 1 mm
    hlEventd(HL_EVENT_MOTION_ANGULAR_TOLERANCE, 0.02); // default 0.02 radians

    hlTouchableFace(HL_FRONT_AND_BACK);

    
    
    // For the IV/LAB Phantom Premium 1.5, the workspace reported from OpenHaptics with 
    //   HLdouble maxWorkspaceDims[6];
    //   hlGetDoublev(HL_MAX_WORKSPACE_DIMS, maxWorkspaceDims);
    // is this: -265, -94, -95, 265, 521.5, 129
    // But, these are not very user friendly values.
    // First, the center of the workspace is not (0,0,0).
    // Second, this is a super conservative view of the usable workspace.  The device can
    // typically generate good force feedback in a much larger volume.

    // So, this class uses these custom workspace dimensions instead of querying them from
    // openhaptics whenever the max dimensions are needed.  Note that if a different Phantom
    // device is used, these will need to be updated.
    custom_workspace_dims_[0] = -600;
    custom_workspace_dims_[1] = -94;
    custom_workspace_dims_[2] = -95;
    custom_workspace_dims_[3] = 600;
    custom_workspace_dims_[4] = 521.5;
    custom_workspace_dims_[5] = 495;


    //HLdouble maxWorkspaceDims[6];
    //hlGetDoublev(HL_MAX_WORKSPACE_DIMS, maxWorkspaceDims);
    //HLdouble size[3];
    //size[0] = maxWorkspaceDims[3] - maxWorkspaceDims[0];
    //size[1] = maxWorkspaceDims[4] - maxWorkspaceDims[1];
    //size[2] = maxWorkspaceDims[5] - maxWorkspaceDims[2];
    //HLdouble center[3];
    //center[0] = maxWorkspaceDims[0] + size[0] / 2.0;
    //center[1] = maxWorkspaceDims[1] + size[1] / 2.0;
    //center[2] = maxWorkspaceDims[2] + size[2] / 2.0;

    // For the Phantom Premium 1.5, the following values are much better
    HLdouble maxWorkspaceDims[6];
    // hlGetDoublev(HL_MAX_WORKSPACE_DIMS, maxWorkspaceDims);
    // -265, -94, -95, 265, 521.5, 129



    HLdouble size[3];
    size[0] = maxWorkspaceDims[3] - maxWorkspaceDims[0];
    size[1] = maxWorkspaceDims[4] - maxWorkspaceDims[1];
    size[2] = maxWorkspaceDims[5] - maxWorkspaceDims[2];
    HLdouble center[3];
    center[0] = maxWorkspaceDims[0] + size[0] / 2.0;
    center[1] = maxWorkspaceDims[1] + size[1] / 2.0;
    center[2] = maxWorkspaceDims[2] + size[2] / 2.0;


    adjusted_workspace_dims_
    HLdouble maxWorkspaceDims[6];
    hlGetDoublev(HL_MAX_WORKSPACE_DIMS, maxWorkspaceDims);
    adjusted_workspace_size_[0] = 2.25 * (maxWorkspaceDims[3] - maxWorkspaceDims[0]);
    adjusted_workspace_size_[1] = maxWorkspaceDims[4] - maxWorkspaceDims[1];
    adjusted_workspace_size_[2] = maxWorkspaceDims[5] - maxWorkspaceDims[2];

    CheckHapticError();
    return true;
}

void Phantom::RegisterForceEffect(ForceEffect* effect) {
    effects_[effect->Name()] = effect;
}


void Phantom::UpdateHapticWorkspace() {
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);


    // For the IV/LAB Phantom Premium 1.5, the code below reports
    //   size = 530, 615.5, 224
    //   center = 0, 213.75, 17
    // But, these are not very user friendly values.
    // First, the center of the workspace is not (0,0,0).
    // Second, this is a super conservative view of the usable workspace.  The device can
    // typically generate good force feedback in a much larger volume.
    
    //HLdouble maxWorkspaceDims[6];
    //hlGetDoublev(HL_MAX_WORKSPACE_DIMS, maxWorkspaceDims);
    //HLdouble size[3];
    //size[0] = maxWorkspaceDims[3] - maxWorkspaceDims[0];
    //size[1] = maxWorkspaceDims[4] - maxWorkspaceDims[1];
    //size[2] = maxWorkspaceDims[5] - maxWorkspaceDims[2];
    //HLdouble center[3];
    //center[0] = maxWorkspaceDims[0] + size[0] / 2.0;
    //center[1] = maxWorkspaceDims[1] + size[1] / 2.0;
    //center[2] = maxWorkspaceDims[2] + size[2] / 2.0;
    
    // For the Phantom Premium 1.5, the following values are much better
    HLdouble maxWorkspaceDims[6];
    // hlGetDoublev(HL_MAX_WORKSPACE_DIMS, maxWorkspaceDims);
    // -265, -94, -95, 265, 521.5, 129
    maxWorkspaceDims[0] = -600;
    maxWorkspaceDims[1] = -94;
    maxWorkspaceDims[2] = -95;

    maxWorkspaceDims[3] = 600;
    maxWorkspaceDims[4] = 521.5;
    maxWorkspaceDims[5] = 495;


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
}


void Phantom::BeginHapticFrame() {
    hlBeginFrame();
}

void Phantom::PollForInput() {
    hlCheckEvents();
}

void Phantom::Reset() {
    for (const auto &entry : effects_) {
        ForceEffect* effect = entry.second;
        effect->Reset();
    }
}



void Phantom::DrawHaptics() {
    for (const auto& entry : effects_) {
        ForceEffect* effect = entry.second;
        effect->DrawHaptics();
    }
    CheckHapticError();
}


void Phantom::EndHapticFrame() {
    hlEndFrame();
}


bool Phantom::CheckHapticError() {
    HLerror error = hlGetError();
    if (error.errorCode == HL_NO_ERROR) {
        return false;
    }
    else if (error.errorCode == HL_DEVICE_ERROR) {
        std::cerr << "Phantom::CheckErrors() Device error: " << hdGetErrorString(error.errorInfo.errorCode) << std::endl;
    }
    else if (error.errorCode == HL_INVALID_ENUM) {
        std::cerr << "Phantom::CheckErrors() HL_INVALID_ENUM - An invalid value for an enumerated type was passed to an API function." << std::endl;
    }
    else if (error.errorCode == HL_INVALID_OPERATION) {
        std::cerr << "Phantom::CheckErrors() HL_INVALID_OPERATION An API function was called when the renderer was not in an appropriate "
            "state for that function call.For example, hlBeginShape() was called outside an hlBeginFrame() / hlEndFrame() pair, or "
            "hlBeginFrame() when no haptic rendering context was active." << std::endl;
    }
    else if (error.errorCode == HL_INVALID_VALUE) {
        std::cerr << "Phantom::CheckErrors() HL_INVALID_VALUE - A value passed to an API function is outside the valid range for that function." << std::endl;
    }
    else if (error.errorCode == HL_OUT_OF_MEMORY) {
        std::cerr << "Phantom::CheckErrors() HL_OUT_OF_MEMORY - There is not enough memory to complete the last API function called. This function " 
            "may have partially completed leaving the haptic renderer in an undefined state." << std::endl;
    }
    else if (error.errorCode == HL_STACK_OVERFLOW) {
        std::cerr << "Phantom::CheckErrors() HL_STACK_OVERFLOW - An API function was called that would have caused an overflow of the matrix stack." << std::endl;
    }
    else if (error.errorCode == HL_STACK_UNDERFLOW) {
        std::cerr << "Phantom::CheckErrors() HL_STACK_UNDERFLOW - An API function was called that would have caused an underflow of the matrix stack, " 
            "i.e.a call to hlPopMatrix() when the stack is empty." << std::endl;
    }
    else {
        std::cerr << "Phantom::CheckErrors() Unknown error." << std::endl;
    }
    return true;
}


void Phantom::DrawGraphics() {
    // Draw a simple cursor (code adapted from OpenHaptics examples)

    // sizes in mm
    static const double kCursorRadius = 8.0;
    static const double kCursorHeight = 20.0;

    // Push state
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
    glPushMatrix();

    // Get the proxy transform in world coordinates.'
    HLdouble proxyxform[16];
    hlGetDoublev(HL_PROXY_TRANSFORM, proxyxform);
    glMultMatrixd(proxyxform);

    // Apply the local cursor scale factor.
    //glScaled(gCursorScale, gCursorScale, gCursorScale);

    // Use change in color to signal whether the button is up/down
    glEnable(GL_COLOR_MATERIAL);
    HLboolean down;
    hlGetBooleanv(HL_BUTTON1_STATE, &down);
    if (down) {
        glColor3f(1.0, 0.5, 0.0);
    }
    else {
        glColor3f(0.0, 0.5, 1.0);
    }

    // Draw a cone
    GLUquadricObj* qobj = gluNewQuadric();
    gluCylinder(qobj, 0.0, kCursorRadius, kCursorHeight, 15, 15);
    // Draw a cap on the bottom of the cone
    glTranslated(0.0, 0.0, kCursorHeight);
    gluDisk(qobj, 0.0, kCursorRadius, 15, 1);
    //gluCylinder(qobj, kCursorRadius, 0.0, kCursorHeight / 5.0, 15, 15);
    gluDeleteQuadric(qobj);

    // Restore state
    glDisable(GL_COLOR_MATERIAL);
    glPopMatrix();
    glPopAttrib();
    
    
    // Draw a light indication of the max workspace size as reported by openhaptics with 
    // the change that they REALLY underestimate the usable horizontal space of the PHANToM 
    // Premium devices.  We can more than double the X dimension and get a lot of nice
    // usable space!  When client specify haptic effects, everything should work pretty
    // well if the coordinates fall within these dimensions.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScaled(adjusted_workspace_size_[0], adjusted_workspace_size_[1], adjusted_workspace_size_[2]);
    glDisable(GL_LIGHTING);
    if (is_in_adjusted_workspace()) {
        glColor3f(0.5, 0.5, 0.5);
    }
    else {
        glColor3f(0.8, 0.5, 0.5);
    }
    glutWireCube(1);
    glEnable(GL_LIGHTING);
    glPopMatrix();


    // Ask each force effect draw itself
    for (const auto& entry : effects_) {
        ForceEffect* effect = entry.second;
        effect->DrawGraphics();
    }
}
