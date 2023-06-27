
#include "phantom.h"

#include <iostream>

#include "event_mgr.h"
#include "force_messages.h"


void HLCALLBACK Button1DownCallback(HLenum event, HLuint object, HLenum thread, HLcache *cache, void *userdata) {
    ((EventMgr*)userdata)->QueueEvent(new VREvent(ForceMessages::get_primary_btn_down_message_name()));
    ((EventMgr*)userdata)->QueueForClient(new VREvent(ForceMessages::get_primary_btn_down_message_name()));
}

void HLCALLBACK Button1UpCallback(HLenum event, HLuint object, HLenum thread, HLcache *cache, void *userdata) {
    ((EventMgr*)userdata)->QueueEvent(new VREvent(ForceMessages::get_primary_btn_up_message_name()));
    ((EventMgr*)userdata)->QueueForClient(new VREvent(ForceMessages::get_primary_btn_up_message_name()));
}

void HLCALLBACK MotionCallback(HLenum event, HLuint object, HLenum thread, HLcache *cache, void *userdata) {
    double pos[3];
    hlGetDoublev(HL_PROXY_POSITION, pos);
    ((EventMgr*)userdata)->QueueEvent(new VREventVector3(ForceMessages::get_position_update_message_name(), (float)pos[0], (float)pos[1], (float)pos[2]));
    ((EventMgr*)userdata)->QueueForClient(new VREventVector3(ForceMessages::get_position_update_message_name(), (float)pos[0], (float)pos[1], (float)pos[2]));

    double rot[4];
    hlGetDoublev(HL_PROXY_ROTATION, rot);
    ((EventMgr*)userdata)->QueueEvent(new VREventQuaternion(ForceMessages::get_rotation_update_message_name(), (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3]));
    ((EventMgr*)userdata)->QueueForClient(new VREventQuaternion(ForceMessages::get_rotation_update_message_name(), (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3]));
}



Phantom::Phantom(EventMgr* event_mgr): event_mgr_(event_mgr) {
    
}

Phantom::~Phantom() {
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
    // DEVICE-LEVEL INITIALIZATION
    HDErrorInfo error;
    hd_device_ = hdInitDevice(HD_DEFAULT_DEVICE);
    if (HD_DEVICE_ERROR(error = hdGetError())) {
        std::cerr << "Failed to initialize " << device_name << ". HD error code: " << error.errorCode << std::endl;
        return false;
    }
    if (HD_SUCCESS != hdGetError().errorCode)
    {
        std::cerr << "Failed to initialize " << device_name << ". HD error code: " << error.errorCode << std::endl;
        return false;
    }

    // Create a haptic rendering context and activate it.
    hl_context_ = hlCreateContext(hd_device_);
    hlMakeCurrent(hl_context_);
    
    // setup callbacks for input from the phantom
    hlAddEventCallback(HL_EVENT_1BUTTONDOWN, HL_OBJECT_ANY, HL_CLIENT_THREAD, &Button1DownCallback, event_mgr_);
    hlAddEventCallback(HL_EVENT_1BUTTONUP, HL_OBJECT_ANY, HL_CLIENT_THREAD, &Button1UpCallback, event_mgr_);
    hlAddEventCallback(HL_EVENT_MOTION, HL_OBJECT_ANY, HL_CLIENT_THREAD, &MotionCallback, event_mgr_);
    // set thresholds for how much the phantom must before a new motion event is fired
    hlEventd(HL_EVENT_MOTION_LINEAR_TOLERANCE, 1.0);   // default 1 mm
    hlEventd(HL_EVENT_MOTION_ANGULAR_TOLERANCE, 0.02); // default 0.02 radians

    
    // SETUP HAPTIC WORKSPACE
    hlDisable(HL_USE_GL_MODELVIEW);
    
    // For the Phantom Premium A 1.0, units in mm
    // Max reported extents       = -260, -106, -61, 260, 400, 119
    // Reasonable working volume  = -130,  -70, -40, 130, 190,  90
   
    // Map this working volume to the world coordinates specified
    hlMatrixMode(HL_TOUCHWORKSPACE);
    hlLoadIdentity();

    // Make the origin the center of the reasonable working volume
    //hlTranslated(0.0, 60.0, 25.0);
    // Convert feet to mm
    //hlScaled(304.8, 304.8, 304.8);

    // Apply a uniform scale to the working volume
    double scale = 1.0;
    hlScaled(1.0/scale, 1.0/scale, 1.0/scale);

    // INITIALIZE FORCE SERVER EFFECTS
    
    
    
    return true;
}

void Phantom::Reset() {
    
}

void Phantom::PollForInput() {
    hlCheckEvents();
}

void Phantom::Draw() {
    hlBeginFrame();
   
    // RENDER THE HAPTIC SCENE
    //hlBeginShape(HL_SHAPE_DEPTH_BUFFER, gMyShapeId);
    //glBegin(GL_POLYGON);
    //glVertex3f(0.25, 0.25, 0.0);
    //glVertex3f(0.75, 0.25, 0.0);
    //glVertex3f(0.75, 0.75, 0.0);
    //glVertex3f(0.25, 0.75, 0.0);
    //glEnd();
    //hlEndShape();

    
    
    hlEndFrame();
}

void Phantom::RegisterForceEffect(const std::string &effect_name, ForceEffect *effect) {
    effects_[effect_name] = effect;
}

void Phantom::OnStartForceEffect(VREvent* event) {
    VREventString* e_start = dynamic_cast<VREventString*>(event);
    if (effects_.find(e_start->get_data()) == effects_.end()) {
        std::cerr << "Phantom::OnStartForceEffect(): Error, unknown effect: " << e_start->get_data() << std::endl;
        return;
    }
    effects_[e_start->get_data()]->Start();
}

void Phantom::OnStopForceEffect(VREvent* event) {
    VREventString* e_stop = dynamic_cast<VREventString*>(event);
    if (effects_.find(e_stop->get_data()) == effects_.end()) {
        std::cerr << "Phantom::OnStopForceEffect(): Error, unknown effect: " << e_stop->get_data() << std::endl;
        return;
    }
    effects_[e_stop->get_data()]->Stop();
}
