
#include "ambient_viscous.h"

#include "force_messages.h"
#include "phantom.h"

#include <iostream>

AmbientViscous::AmbientViscous(EventMgr* event_mgr) : gain_(0.8), magnitude_cap_(1.0), effect_id_(0),
    active_(false), active_buffered_(false), params_dirty_(false)
{
    std::string event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Start";
    event_mgr->AddListener(event_name, this, &AmbientViscous::OnStartEffect);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Stop";
    event_mgr->AddListener(event_name, this, &AmbientViscous::OnStopEffect);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetGain";
    event_mgr->AddListener(event_name, this, &AmbientViscous::OnGainChange);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetMagnitudeCap";
    event_mgr->AddListener(event_name, this, &AmbientViscous::OnMagnitudeCapChange);
}

AmbientViscous::~AmbientViscous() {
    
}

void AmbientViscous::OnGainChange(VREvent* e) {
    VREventFloat* e_gain = dynamic_cast<VREventFloat*>(e);
    if (e_gain != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        gain_ = e_gain->get_data();
        params_dirty_ = true;
    }
}

void AmbientViscous::OnMagnitudeCapChange(VREvent* e) {
    VREventFloat* e_mag = dynamic_cast<VREventFloat*>(e);
    if (e_mag != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        magnitude_cap_ = e_mag->get_data();
        params_dirty_ = true;
    }
}

void AmbientViscous::OnStartEffect(VREvent* e) {
    active_buffered_ = true;
}

void AmbientViscous::OnStopEffect(VREvent* e) {
    active_buffered_ = false;
}

void AmbientViscous::Reset() {
    active_buffered_ = false;
}

void AmbientViscous::DrawHaptics() {    
    // if a change in the active status has occurred since the last frame, then handle it
 // by either starting or stopping the OpenHaptics force effect
    if (active_buffered_ != active_) {
        if (!active_) {
            // start the effect
            if (!hlIsEffect(effect_id_)) {
                effect_id_ = hlGenEffects(1);
                Phantom::CheckHapticError();
            }
            hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
            hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
            hlStartEffect(HL_EFFECT_VISCOUS, effect_id_);
            Phantom::CheckHapticError();
            active_ = true;
        }
        else {
            // stop the effect
            hlStopEffect(effect_id_);
            Phantom::CheckHapticError();
            active_ = false;
        }
        active_buffered_ = active_;
    }
    // if we have new values for params && the effect is active, then update the params
    // note: an error is generated if you update before the effect is started.
    // note: this must happen in an else statement because an error is also generated if
    // you call hlUpdateEffect() in the same haptic frame where you called hlStartEffect()!
    else if ((params_dirty_) && (active_)) {
        hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
        hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
        hlUpdateEffect(effect_id_);
        Phantom::CheckHapticError();
        params_dirty_ = false;
    }
}

void AmbientViscous::DrawGraphics() {    
}

