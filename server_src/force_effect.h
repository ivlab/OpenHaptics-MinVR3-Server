
#ifndef FORCE_SERVER_FORCE_EFFECT_H
#define FORCE_SERVER_FORCE_EFFECT_H

#include <string>

class ForceEffect {
public:
    ForceEffect() : active_(false) {}
    virtual ~ForceEffect() {}
    
    bool is_active() const { return active_; }
    
    void Start() {
        active_ = true;
        OnStartEffect();
    }
    
    void Stop() {
        active_ = false;
        OnStopEffect();
    }
    
    virtual const std::string Name() const = 0;
    virtual void Init() = 0;
    virtual void OnStartEffect() = 0;
    virtual void OnStopEffect() = 0;
    virtual void DrawHaptics() = 0;
    virtual void DrawGraphics() = 0;
    
protected:
    bool active_;
};

#endif
