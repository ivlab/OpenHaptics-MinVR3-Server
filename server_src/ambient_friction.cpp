
#include "ambient_friction.h"

#include "force_messages.h"
#include "phantom.h"

AmbientFriction::AmbientFriction(EventMgr* event_mgr) : gain_(0.15), magnitude_cap_(0.75), effect_id_(0), 
    start_this_frame_(false), stop_this_frame_(false), update_this_frame_(false), active_(false) 
{
    std::string event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Start";
    event_mgr->AddListener(event_name, this, &AmbientFriction::OnStartEffect);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Stop";
    event_mgr->AddListener(event_name, this, &AmbientFriction::OnStopEffect);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetGain";
    event_mgr->AddListener(event_name, this, &AmbientFriction::OnGainChange);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetMagnitudeCap";
    event_mgr->AddListener(event_name, this, &AmbientFriction::OnMagnitudeCapChange);
}

AmbientFriction::~AmbientFriction() {
    
}

void AmbientFriction::OnGainChange(VREvent* e) {
    VREventFloat* e_gain = dynamic_cast<VREventFloat*>(e);
    if (e_gain != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        gain_ = e_gain->get_data();
        update_this_frame_ = true;
    }
}

void AmbientFriction::OnMagnitudeCapChange(VREvent* e) {
    VREventFloat* e_mag = dynamic_cast<VREventFloat*>(e);
    if (e_mag != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        magnitude_cap_ = e_mag->get_data();
        update_this_frame_ = true;
    }
}


void AmbientFriction::OnStartEffect(VREvent* e) {
    if (stop_this_frame_) {
        // if stop this frame was already set, then this is a really fast stop/start sequence within a single frame!
        // just do nothing in this case
        start_this_frame_ = false;
        stop_this_frame_ = false;
    }
    else {
        start_this_frame_ = true;
    }
}

void AmbientFriction::OnStopEffect(VREvent* e) {
    if (start_this_frame_) {
        // if start this frame was already set, then this is a really fast start/stop sequence within a single frame!
        // just do nothing in this case
        start_this_frame_ = false;
        stop_this_frame_ = false;
    }
    else {
        stop_this_frame_ = true;
    }
}

void AmbientFriction::DrawHaptics() {
    // OpenHaptics generates an error if hlUpdateEffect() is called in the same frame as hlStartEffect()
// So, we pick one thing to do each frame: either start, stop, or update.  The effect params are
// set before a start, so this works well even if start and update were called in the same frame.
// If stop and update were called in the same frame, then we just stop, there is no point to doing
// an update.  The logic for if start and stop were called in the same frame is included in the
// OnStart/Stop routines above.

    if (start_this_frame_) {
        if (active_) {
            std::cerr << "AmbientFriction::DrawHaptics() Warning: Trying to start effect that is already active." << std::endl;
        }
        else {
            if (!hlIsEffect(effect_id_)) {
                effect_id_ = hlGenEffects(1);
                Phantom::CheckHapticError();
            }
            hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
            hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
            hlStartEffect(HL_EFFECT_FRICTION, effect_id_);
            Phantom::CheckHapticError();
            active_ = true;
            //std::cout << "STARTED VISCOUS" << std::endl;
        }
    }
    else if (stop_this_frame_) {
        if (!active_) {
            std::cerr << "AmbientFriction::DrawHaptics() Warning: Trying to stop effect that is not active." << std::endl;
        }
        else {
            hlStopEffect(effect_id_);
            Phantom::CheckHapticError();
            active_ = false;
            //std::cout << "STOPPED VISCOUS" << std::endl;
        }
    }
    else if (update_this_frame_) {
        if (active_) {
            // only update if the effect has already been started, otherwise, it will be started with the latest parameters
            // whenever start is called.
            hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
            hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
            hlUpdateEffect(effect_id_);
            Phantom::CheckHapticError();
            //std::cout << "UPDATED VISCOUS" << std::endl;
        }
    }

    // reset dirty flags
    start_this_frame_ = false;
    update_this_frame_ = false;
    stop_this_frame_ = false;
}

void AmbientFriction::DrawGraphics() {
}
