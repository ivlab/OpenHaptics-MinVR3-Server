
#include "phantom.h"

#include <iostream>

#include "event_mgr.h"
#include "force_messages.h"
#include "graphics_headers.h"


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



Phantom::Phantom(EventMgr* event_mgr) : hd_device_(HD_INVALID_HANDLE), hl_context_(0), event_mgr_(event_mgr), initialized_(false) {
    event_mgr_->AddListener(ForceMessages::get_force_effect_start_event_name(), this, &Phantom::OnStartForceEffect);
    event_mgr_->AddListener(ForceMessages::get_force_effect_stop_event_name(), this, &Phantom::OnStopForceEffect);
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

    
    /*
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
    */
    
    hlTouchableFace(HL_FRONT);
    
    initialized_ = CheckHapticError();
    return initialized_;
}

void Phantom::RegisterForceEffect(const std::string& effect_name, ForceEffect* effect) {
    effects_[effect_name] = effect;
}

void Phantom::UpdateHapticWorkspace() {
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


void Phantom::BeginHapticFrame() {
    hlBeginFrame();
}

void Phantom::PollForInput() {
    hlCheckEvents();
}

void Phantom::Reset() {
    for (const auto &entry : active_effects_) {
        ForceEffect* effect = entry.second;
        effect->OnStopEffect();
    }
    active_effects_.clear();
    CheckHapticError();
}

void Phantom::DrawHaptics() {
    
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
    
    for (const auto& entry : active_effects_) {
        ForceEffect* effect = entry.second;
        effect->DrawHaptics();
        CheckHapticError();
    }
}


void Phantom::OnStartForceEffect(VREvent* event) {
    VREventString* e_start = dynamic_cast<VREventString*>(event);
    std::string effect_name = e_start->get_data();
    if (effects_.find(effect_name) == effects_.end()) {
        std::cerr << "Phantom::OnStartForceEffect(): Warning, ignoring unknown effect: " << effect_name << std::endl;
        return;
    }
    std::cout << "STARTING: " << effect_name << std::endl;
    // add the effect to the list of active effects
    active_effects_[effect_name] = effects_[effect_name];
    // start the effect
    active_effects_[effect_name]->OnStartEffect();
    CheckHapticError();
}

void Phantom::OnStopForceEffect(VREvent* event) {
    VREventString* e_stop = dynamic_cast<VREventString*>(event);
    std::string effect_name = e_stop->get_data();
    if (effects_.find(effect_name) == effects_.end()) {
        std::cerr << "Phantom::OnStopForceEffect(): Warning, ignoring unknown effect: " << effect_name << std::endl;
        return;
    }
    if (active_effects_.find(effect_name) == active_effects_.end()) {
        std::cerr << "Phantom::OnStopForceEffect(): Warning, cannot stop an effect that is not currently running: " << effect_name << std::endl;
        return;
    }
    std::cout << "STOPPING: " << effect_name << std::endl;

    // stop the effect
    active_effects_[effect_name]->OnStopEffect();
    // remove it from the list of active effects
    active_effects_.erase(effect_name);
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

    
    

    GLUquadricObj* qobj = gluNewQuadric();

    // DRAW HAPTIC CURSOR
    static const double kCursorRadius = 5;
    static const double kCursorHeight = 160;
    static const int kCursorTess = 15;
    static const double kCursorScale = 1.0;
    GLfloat mat_ambient_gray[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat mat_ambient_red[] = { 1.0, 0.0, 0.0, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_gray);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixd(transform());


    if (is_primary_btn_down()) {
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_red);
    }
    else {
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_gray);
    }
    glTranslated(0.0, 0.0, kCursorHeight / 5.0);
    gluCylinder(qobj, 0.0, kCursorRadius, kCursorHeight / 5.0, kCursorTess, kCursorTess);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_gray);
    glTranslated(0.0, 0.0, kCursorHeight / 5.0);
    gluCylinder(qobj, kCursorRadius, kCursorRadius, kCursorHeight * 4.0 / 5.0, kCursorTess, kCursorTess);

    glTranslated(0.0, 0.0, kCursorHeight * 4.0 / 5.0);
    gluDisk(qobj, 0.0, kCursorRadius, kCursorTess, 1);

    glPopMatrix();


    // shadow of the cursor
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, -299.5f, 0.0f);
    glScalef(1.0f, 0.0f, 1.0f);
    glMultMatrixd(transform());
    glColor3f(0.0f, 0.0f, 0.0f);
    glTranslated(0.0, 0.0, kCursorHeight / 5.0);
    gluCylinder(qobj, 0.0, kCursorRadius, kCursorHeight / 5.0, kCursorTess, kCursorTess);
    glTranslated(0.0, 0.0, kCursorHeight / 5.0);
    gluCylinder(qobj, kCursorRadius, kCursorRadius, kCursorHeight * 4.0 / 5.0, kCursorTess, kCursorTess);
    gluDisk(qobj, 0.0, kCursorRadius, kCursorTess, 1);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    gluDeleteQuadric(qobj);

    
    
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
    
    
    
    for (const auto& entry : active_effects_) {
        ForceEffect* effect = entry.second;
        effect->DrawGraphics();
    }
}
