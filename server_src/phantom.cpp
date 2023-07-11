
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


Phantom::Phantom(EventMgr* event_mgr) : hd_device_(HD_INVALID_HANDLE), hl_context_(0), event_mgr_(event_mgr), primary_down_(false) {
    world_to_custom_workspace_translation_ = hduVector3Dd(0, 0, 0);
    world_to_custom_workspace_rotation_ = hduQuaternion();
    world_to_custom_workspace_scale_ = hduVector3Dd(1, 1, 1);

    std::string event_name = ForceMessages::get_world_to_workspace_translation_event_name();
    event_mgr->AddListener(event_name, this, &Phantom::OnWorldToWorkspaceTranslationUpdate);

    event_name = ForceMessages::get_world_to_workspace_rotation_event_name();
    event_mgr->AddListener(event_name, this, &Phantom::OnWorldToWorkspaceRotationUpdate);

    event_name = ForceMessages::get_world_to_workspace_scale_event_name();
    event_mgr->AddListener(event_name, this, &Phantom::OnWorldToWorkspaceScaleUpdate);
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
    return primary_down_;
}

bool Phantom::is_in_custom_workspace() {
    float xabs = custom_workspace_size_[0] / 2.0;
    float yabs = custom_workspace_size_[1] / 2.0;
    float zabs = custom_workspace_size_[2] / 2.0;
    if (position()[0] < -xabs) return false;
    if (position()[0] > xabs) return false;
    if (position()[1] < -yabs) return false;
    if (position()[1] > yabs) return false;
    if (position()[2] < -zabs) return false;
    if (position()[2] > zabs) return false;
    return true;
}


double* Phantom::transform() {
    return (double*)transform_;
}

double* Phantom::position() {
    return (double*)position_;
}


double* Phantom::rotation() {
    return (double*)rotation_;
}

double* Phantom::custom_workspace_dims() {
    return (double*)custom_workspace_dims_;
}

double* Phantom::custom_workspace_size() {
    return (double*)custom_workspace_size_;
}

double* Phantom::custom_workspace_center() {
    return (double*)custom_workspace_center_;
}

void Phantom::OnWorldToWorkspaceTranslationUpdate(VREvent* e) {
    VREventVector3* e_vec3 = dynamic_cast<VREventVector3*>(e);
    if (e_vec3 != NULL) { // should always pass
        world_to_custom_workspace_translation_[0] = e_vec3->x();
        world_to_custom_workspace_translation_[1] = e_vec3->y();
        world_to_custom_workspace_translation_[2] = e_vec3->z();
    }
}

void Phantom::OnWorldToWorkspaceRotationUpdate(VREvent* e) {
    VREventQuaternion* e_quat = dynamic_cast<VREventQuaternion*>(e);
    if (e_quat != NULL) { // should always pass
        world_to_custom_workspace_rotation_[0] = e_quat->x();
        world_to_custom_workspace_rotation_[1] = e_quat->y();
        world_to_custom_workspace_rotation_[2] = e_quat->z();
        world_to_custom_workspace_rotation_[3] = e_quat->w();
    }
}

void Phantom::OnWorldToWorkspaceScaleUpdate(VREvent* e) {
    VREventVector3* e_vec3 = dynamic_cast<VREventVector3*>(e);
    if (e_vec3 != NULL) { // should always pass
        world_to_custom_workspace_scale_[0] = e_vec3->x();
        world_to_custom_workspace_scale_[1] = e_vec3->y();
        world_to_custom_workspace_scale_[2] = e_vec3->z();
    }
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

    // tell OpenHaptics to ignore the OpenGL modelview matrix since our OpenGL program is really just for
    // displaying some simple graphics to help us debug.  We will be specifying geometry directly
    // in haptic "room space" coordinates rather than relative to the modelview.
    hlDisable(HL_USE_GL_MODELVIEW);
    
    // For the IV/LAB Phantom Premium 1.5, the workspace reported from OpenHaptics with 
    //   HLdouble maxWorkspaceDims[6];
    //   hlGetDoublev(HL_MAX_WORKSPACE_DIMS, maxWorkspaceDims);
    // is this: -265, -94, -95, 265, 521.5, 129
    // These are not very user friendly values because the center of the workspace is not (0,0,0).

    // To remedy this, we do two things:
    // 1. We define these custom workspace dims and use them wherever we might otherwise
    //    ask openhaptics for the max dimensions.
    // 2. We apply a translation in the BeginHapticFrame() method to place (0,0,0) at
    //    the center of the custom workspace.  This means objects rendered at the origin
    //    will tend be be centered within the Phantom's good working range.
    custom_workspace_dims_[0] = -250;
    custom_workspace_dims_[1] = -95;
    custom_workspace_dims_[2] = -130;
    custom_workspace_dims_[3] = 250;
    custom_workspace_dims_[4] = 230;
    custom_workspace_dims_[5] = 130;

    custom_workspace_size_[0] = custom_workspace_dims_[3] - custom_workspace_dims_[0];
    custom_workspace_size_[1] = custom_workspace_dims_[4] - custom_workspace_dims_[1];
    custom_workspace_size_[2] = custom_workspace_dims_[5] - custom_workspace_dims_[2];
    
    custom_workspace_center_[0] = custom_workspace_dims_[0] + custom_workspace_size_[0] / 2.0;
    custom_workspace_center_[1] = custom_workspace_dims_[1] + custom_workspace_size_[1] / 2.0;
    custom_workspace_center_[2] = custom_workspace_dims_[2] + custom_workspace_size_[2] / 2.0;
           
    CheckHapticError();
    return true;
}

void Phantom::RegisterForceEffect(ForceEffect* effect) {
    effects_[effect->Name()] = effect;
}


void Phantom::BeginHapticFrame() {
    hlBeginFrame();

    hlMatrixMode(HL_TOUCHWORKSPACE);
    hlLoadIdentity();

    // This centering transformation converts from the "custom workspace" coordinates defined in this class
    // to the raw device coordinates of the phantom.  The transformation is just a simple translation so
    // that (0,0,0) is custom workspace coordinates is located at the center of the usable space of the
    // device.  For some reason, this is not the case by default.
    hlTranslated(custom_workspace_center()[0], custom_workspace_center()[1], custom_workspace_center()[2]);
}

hduMatrix TRS(hduVector3Dd t, hduQuaternion r, hduVector3Dd s) {
    hduMatrix T, R, S;
    T = hduMatrix::createTranslation(t);
    r.toRotationMatrix(R);
    S = hduMatrix::createScale(s);
    return T * R * S;
}

void Phantom::PollForInput() {
    // proxy coordinates are returned in world coordinates, but we want touch coordinates
    hlCheckEvents();

    // cache latest values at the beginning of each frame
    hlGetBooleanv(HL_BUTTON1_STATE, &primary_down_);


    /*
      working on coordinate spaces here...

    HLdouble pos_cw[3];
    HLdouble rot_cw[4];
    HLdouble mat_cw[16];

    hlGetDoublev(HL_PROXY_POSITION, pos_cw);
    hlGetDoublev(HL_PROXY_ROTATION, rot_cw);
    hlGetDoublev(HL_PROXY_TRANSFORM, mat_cw);
    //std::cout << pos_custom_workspace[0] << " " << pos_custom_workspace[1] << " " << pos_custom_workspace[2] << std::endl;

    hduMatrix w2cw = get_world_to_custom_workspace_matrix();
    hduMatrix cw2w = w2cw.getInverse();

    hduVector3Dd pos_w;
    cw2w.multMatrixVec(hduVector3Dd(pos_cw), pos_w);

    hduMatrix rot_cw_as_mat;
    hduQuaternion(rot_cw).toRotationMatrix(rot_cw_as_mat);
    cw2w_rot = rot_cw_as_mat.getInverse();


    hduQuaternion cw2w_rot = world_to_custom_workspace_rotation_;
    cw2w_rot.inverse();

    hduQuaternion rot_world = cw2w_rot. * rot_custom_workspace;
    */
}

void Phantom::Reset() {
    for (const auto &entry : effects_) {
        ForceEffect* effect = entry.second;
        effect->Reset();
    }
}

hduMatrix Phantom::get_world_to_custom_workspace_matrix() {
    // build world to custom workspace matrix
    hduMatrix T, R, S;
    T = hduMatrix::createTranslation(world_to_custom_workspace_translation_);
    world_to_custom_workspace_rotation_.toRotationMatrix(R);
    S = hduMatrix::createScale(world_to_custom_workspace_scale_);
    return T * R * S;
}

void Phantom::DrawHaptics() {
    hlMatrixMode(HL_TOUCHWORKSPACE);
    hlPushMatrix();
    hduMatrix w2cw = get_world_to_custom_workspace_matrix();
    hlMultMatrixd(w2cw);

    for (const auto& entry : effects_) {
        ForceEffect* effect = entry.second;
        effect->DrawHaptics();
    }

    hlPopMatrix(); // world_to_custom_workspace
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

    // Get the proxy transform
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
    
    
    // Draw an indication of the maximum usable workspace of the Phantom.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScaled(custom_workspace_size_[0], custom_workspace_size_[1], custom_workspace_size_[2]);
    glDisable(GL_LIGHTING);
    if (is_in_custom_workspace()) {
        glColor3f(0.5, 0.5, 0.5);
    }
    else {
        glColor3f(0.8, 0.5, 0.5);
    }
    glutWireCube(1);
    glEnable(GL_LIGHTING);
    glPopMatrix();


    // Ask each force effect draw itself using the world_to_custom_workspace transform
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    hduMatrix w2cw = get_world_to_custom_workspace_matrix();
    glMultMatrixd(w2cw);


    for (const auto& entry : effects_) {
        ForceEffect* effect = entry.second;
        effect->DrawGraphics();
    }

    glPopMatrix(); // world_to_custom_workspace
}
