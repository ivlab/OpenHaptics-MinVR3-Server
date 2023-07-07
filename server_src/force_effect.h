
#ifndef FORCE_SERVER_FORCE_EFFECT_H
#define FORCE_SERVER_FORCE_EFFECT_H

#include <string>

class ForceEffect {
public:
    virtual const std::string Name() const = 0;
    virtual void DrawHaptics() = 0;
    virtual void DrawGraphics() = 0;
    virtual void Reset() = 0;

    ForceEffect() {}
    virtual ~ForceEffect() {}
};

#endif
