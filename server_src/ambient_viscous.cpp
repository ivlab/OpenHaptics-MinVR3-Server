
#include "ambient_viscous.h"

#include "force_messages.h"
#include <iostream>

AmbientViscous::AmbientViscous(EventMgr* event_mgr) : gain_(0.8), magnitude_cap_(1.0) {
    std::string gain_event_name = ForceMessages::get_force_effect_param_event_name(Name(), "Gain");
    event_mgr->AddListener(gain_event_name, this, &AmbientViscous::OnGainChange);

    std::string mag_event_name = ForceMessages::get_force_effect_param_event_name(Name(), "MagnitudeCap");
    event_mgr->AddListener(mag_event_name, this, &AmbientViscous::OnMagnitudeCapChange);
}

AmbientViscous::~AmbientViscous() {
    
}

void AmbientViscous::OnGainChange(VREvent* e) {
    VREventFloat* e_gain = dynamic_cast<VREventFloat*>(e);
    if (e_gain != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        gain_ = e_gain->get_data();
    }
}

void AmbientViscous::OnMagnitudeCapChange(VREvent* e) {
    VREventFloat* e_mag = dynamic_cast<VREventFloat*>(e);
    if (e_mag != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        magnitude_cap_ = e_mag->get_data();
    }
}


void AmbientViscous::Init() {
    std::cout << "INIT" << std::endl;
    effect_id_ = hlGenEffects(1);
}

void AmbientViscous::OnStartEffect() {
    std::cout << "START" << std::endl;

    hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
    hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
    hlStartEffect(HL_EFFECT_VISCOUS, effect_id_);
}

void AmbientViscous::OnStopEffect() {
    std::cout << "STOP" << std::endl;

    hlStopEffect(effect_id_);
}

void AmbientViscous::DrawHaptics() {
    std::cout << "DRAW" << std::endl;
    hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
    hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
    hlUpdateEffect(effect_id_);
}

void AmbientViscous::DrawGraphics() {
    
}

