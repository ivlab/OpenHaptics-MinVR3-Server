
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


Phantom::Phantom(EventMgr* event_mgr) : hd_device_(HD_INVALID_HANDLE), hl_context_(0), event_mgr_(event_mgr), primary_down_(false) {
    user_model_to_world_scale_ = hduVector3Dd(1, 1, 1);

    std::string event_name = ForceMessages::get_model_to_world_translation_event_name();
    event_mgr->AddListener(event_name, this, &Phantom::OnUserModelToWorldTranslationUpdate);
    event_name = ForceMessages::get_model_to_world_rotation_event_name();
    event_mgr->AddListener(event_name, this, &Phantom::OnUserModelToWorldRotationUpdate);
    event_name = ForceMessages::get_model_to_world_scale_event_name();
    event_mgr->AddListener(event_name, this, &Phantom::OnUserModelToWorldScaleUpdate);
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
    if (touch_space_position()[0] < -xabs) return false;
    if (touch_space_position()[0] > xabs) return false;
    if (touch_space_position()[1] < -yabs) return false;
    if (touch_space_position()[1] > yabs) return false;
    if (touch_space_position()[2] < -zabs) return false;
    if (touch_space_position()[2] > zabs) return false;
    return true;
}


double* Phantom::touch_space_transform() {
    return (double*)touch_space_transform_;
}

double* Phantom::touch_space_position() {
    return (double*)touch_space_position_;
}

double* Phantom::touch_space_rotation() {
    return (double*)touch_space_rotation_;
}

double* Phantom::user_space_transform() {
    return (double*)user_space_transform_;
}

double* Phantom::user_space_position() {
    return (double*)user_space_position_;
}

double* Phantom::user_space_rotation() {
    return (double*)user_space_rotation_;
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

void Phantom::OnUserModelToWorldTranslationUpdate(VREvent* e) {
    VREventVector3* e_vec3 = dynamic_cast<VREventVector3*>(e);
    if (e_vec3 != NULL) { // should always pass
        user_model_to_world_translation_[0] = e_vec3->x();
        user_model_to_world_translation_[1] = e_vec3->y();
        user_model_to_world_translation_[2] = e_vec3->z();
        RebuildUserModelToWorld();
    }
}

void Phantom::OnUserModelToWorldRotationUpdate(VREvent* e) {
    VREventQuaternion* e_quat = dynamic_cast<VREventQuaternion*>(e);
    if (e_quat != NULL) { // should always pass
        user_model_to_world_rotation_[0] = e_quat->x();
        user_model_to_world_rotation_[1] = e_quat->y();
        user_model_to_world_rotation_[2] = e_quat->z();
        user_model_to_world_rotation_[3] = e_quat->w();
        RebuildUserModelToWorld();
    }
}

void Phantom::OnUserModelToWorldScaleUpdate(VREvent* e) {
    VREventVector3* e_vec3 = dynamic_cast<VREventVector3*>(e);
    if (e_vec3 != NULL) { // should always pass
        user_model_to_world_scale_[0] = e_vec3->x();
        user_model_to_world_scale_[1] = e_vec3->y();
        user_model_to_world_scale_[2] = e_vec3->z();
        RebuildUserModelToWorld();
    }
}

void Phantom::RebuildUserModelToWorld() {
    hduMatrix T, R, S;
    T = hduMatrix::createTranslation(user_model_to_world_translation_);
    user_model_to_world_rotation_.toRotationMatrix(R);
    S = hduMatrix::createScale(user_model_to_world_scale_);
    user_model_to_world_matrix_ = T * R * S;
    user_world_to_model_matrix_ = user_model_to_world_matrix_.getInverse();
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

    // tell OpenHaptics to ignore the OpenGL modelview matrix.  Our OpenGL program is really just for
    // displaying some simple graphics to help us debug and we want these decoupled from the OpenGL
    // camera parameters.
    //hlDisable(HL_USE_GL_MODELVIEW);
    
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




void Phantom::PollForInput() {
    // proxy coordinates are returned in world coordinates, but we want touch coordinates
    hlCheckEvents();

    // cache latest values at the beginning of each frame
    hlGetBooleanv(HL_BUTTON1_STATE, &primary_down_);



    hlGetDoublev(HL_PROXY_POSITION, touch_space_position_);
    hlGetDoublev(HL_PROXY_ROTATION, touch_space_rotation_);
    hlGetDoublev(HL_PROXY_TRANSFORM, touch_space_transform_);

    // std::cout << touch_space_position_[0] << "  " << touch_space_transform_.get(3,0) << std::endl;

    
    // note: as we have defined it, Touch Space == World Space, so transforming by the inverse of
    // the user's model-to-world will transform the stylus pos/rot into the user's Model Space.
    
    // user_space_transform = user_world_to_model_matrix * touch_space_transform_;
    user_space_transform_ = user_world_to_model_matrix_;
    user_space_transform_.multLeft(touch_space_transform_);
    
    // extract rotation as a hduQuaternion
    hduMatrix user_space_transform_rot_only = user_space_transform_;
    user_space_transform_rot_only.getRotationMatrix(user_space_transform_rot_only);
    user_space_rotation_ = hduQuaternion(user_space_transform_rot_only);

    // extract position from 3rd column
    user_space_position_[0] = user_space_transform_.get(3,0);
    user_space_position_[1] = user_space_transform_.get(3,1);
    user_space_position_[2] = user_space_transform_.get(3,2);

    // generate a stylus position move event if the movement since the last reported event is >= 1 mm
    hduVector3Dd delta_pos = touch_space_position_ - last_reported_position_;
    if (delta_pos.magnitude() >= 1.0) {
        event_mgr_->QueueEvent(new VREventVector3(ForceMessages::get_position_update_event_name(),
           (float)(user_space_position_[0]), (float)user_space_position_[1], (float)user_space_position_[2]));
        event_mgr_->QueueForClient(new VREventVector3(ForceMessages::get_position_update_event_name(),
           (float)user_space_position_[0], (float)user_space_position_[1], (float)user_space_position_[2]));
        last_reported_position_ = touch_space_position_;
    }
    
    // generate a stylus rotation move event if the movement since the last reported event is >= 0.02 radians
    hduQuaternion delta_rot = last_reported_rotation_.inverse() * touch_space_rotation_;
    float angle = abs(2.0 * atan2(delta_rot.v().magnitude(), delta_rot.s()));
    if (angle >= 0.2) {
        event_mgr_->QueueEvent(new VREventQuaternion(ForceMessages::get_rotation_update_event_name(),
            (float)user_space_rotation_.v()[0], (float)user_space_rotation_.v()[1], (float)user_space_rotation_.v()[2], (float)user_space_rotation_.s()));
        event_mgr_->QueueForClient(new VREventQuaternion(ForceMessages::get_rotation_update_event_name(),
            (float)user_space_rotation_.v()[0], (float)user_space_rotation_.v()[1], (float)user_space_rotation_.v()[2], (float)user_space_rotation_.s()));
        last_reported_rotation_ = touch_space_rotation_;
    }
}

void Phantom::Reset() {
    for (const auto &entry : effects_) {
        ForceEffect* effect = entry.second;
        effect->Reset();
    }
}


void Phantom::DrawHaptics() {
    // Important: OpenHaptics does some auto-magical saving of the OpenGL state at the beginning of each
    // frame, even if HL_USE_GL_MODELVIEW is disabled!  To make sure this does not break the matrix math
    // in the haptic rendering and reported current transformation of the stylus, make sure that the
    // **OpenGL ModelView** matrix is set to the Identity before calling DrawHaptics()!!
    hlBeginFrame();
    
    // The detailed comments are intentional because the way that OpenHaptics tries to fit haptics
    // rendering into the model used by OpenGL is confusing and not well described in their docs.
    
    // The TouchWorkspace matrix converts from Touch Space to the raw device coordinates of the Phantom,
    // which OpenHaptics calls Workspace coordinates.  OpenHaptics makes it possible for us to query
    // the dimensions of the Max Workspace and the Usable Workspace, but after using the device, the
    // reported dimensions are not actually ideal, so we define our own custom dimensions in the Init()
    // function.  The translation applied here simply centers the origin of Touch Space within the
    // bounds of our custom workspace dimensions.  This is needed because, the usable volume of the
    // phantom is not symmetrical in the vertical and depth dimensions, i.e., the Workspace origin is
    // higher than the center of the volume of the usable working space of the Phantom device.
    hlMatrixMode(HL_TOUCHWORKSPACE);
    hlLoadIdentity();
    hlTranslated(custom_workspace_center()[0], custom_workspace_center()[1], custom_workspace_center()[2]);
        
    // The ViewTouch matrix converts from View Space to Touch Space.  OpenHaptics treats this as analogous
    // to the OpenGL Projection matrix.  It can be used to map the View Frustum to Touch Space, but this
    // server treats that haptic space as fixed at the origin, not coupled to the camera.  So, our ViewTouch
    // is just the identity.
    hlMatrixMode(HL_VIEWTOUCH);
    hlLoadIdentity();
    
    // As in OpenGL, the ModelView matrix is really two matrices: (1) the Model matrix, and (2) the View matrix.
    // The View matrix converts from World Space to View Space, and the Model matrix converts from Model Space
    // to World Space, i.e., ModelView = WorldToView * ModelToWorld
    glMatrixMode(GL_MODELVIEW);
    
    // The View matrix, a.k.a. WorldToView matrix, is the identity for this server because we are not coupling
    // the haptics to the camera.  So, the first matrix on the HL_MODELVIEW stack is the identity.
    glLoadIdentity(); // ModelView = WorldToView

    // Note: At this point, almost all of the transforms are the identity.  So, we have made World Space equal
    // View Space, which equals Touch Space, which is almost equal to Workspace Space, the only difference
    // between Touch Space and the device's Workspace Space being a translation to center the origin.
    
    // As in OpenGL, Model matrix, a.k.a. ModelToWorld matix, is the one that programmers use to transform
    // a particular model to place it within the World.  The user's "model" in this case is the geometry
    // rendered in the ForceEffects.  Users can specify the ForceEffects geometry in whatever Model Space
    // coordinates they like as long as they set this ModelToWorld matrix properly to transform their
    // geometry into World Space.  Remember, for us World Space == View Space == Touch Space.  So, the
    // ModelToWorld matrix is essentially a ModelToTouch matrix.  It should transform whatever coordinate
    // system is used to define the force effects into Touch Space, where the units are in real-world mm
    // and the origin is at the center of the usable bounds of the phantom device.  As in OpenGL, the
    // ModelToWorld matrix is applied to the ModelView matrix stack.
    glMultMatrixd(user_model_to_world_matrix_); // ModelView = WorldToView * ModelToWorld

    for (const auto& entry : effects_) {
        ForceEffect* effect = entry.second;
        effect->DrawHaptics();
    }
    
    hlEndFrame();
    CheckHapticError();
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
    // The GLUT application in main.cpp handles camera setup for graphics rendering.  So, comparing
    // the DrawHaptics() routine above with this one, the OpenGL Viewport (analogous to TouchWorkspace)
    // and OpenGL Projection (analogous to ViewTouch) are already set and managed by the GLUT app.
    // Additionally, the ViewMatrix, which accounts for camera pos, look, and up, should already be
    // added to the OpenGL ModelView matrix stack.  Before rendering graphics, we just need to
    // apply the Model matrix to the ModelView stack.
    
    // For the first set of models we will draw, the Model (a.k.a. ModelToWorld) matrix will be the
    // identity.  Since, these geometries are defined in World Space coordinates.
    glMatrixMode(GL_MODELVIEW);
    // no need to actually multiply by the identity, but for clarity, we could do:
    // glMultMatrixd(identity); // ModelView = WorldToView * ModelToWorld

    // Draw the bounds of the custom workspace
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
    


    // Draw a simple cursor (code adapted from OpenHaptics examples)
    // sizes in mm
    static const double kCursorRadius = 8.0;
    static const double kCursorHeight = 20.0;

    // Push state
    glMatrixMode(GL_MODELVIEW);
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
    glPushMatrix();

    glMultMatrixd(touch_space_transform_);

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

    
    // Now, we can apply the user-set ModelToWorld matrix and draw all the effects in the user's
    // model space.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixd(user_model_to_world_matrix_);

    for (const auto& entry : effects_) {
        ForceEffect* effect = entry.second;
        effect->DrawGraphics();
    }

    glPopMatrix();
}
