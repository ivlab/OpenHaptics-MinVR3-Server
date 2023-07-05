
#ifndef FORCE_SERVER_FORCE_EFFECT_H
#define FORCE_SERVER_FORCE_EFFECT_H

#include <string>

/**
   To create a new ForceEffect, inherit this base class and override the pure virtual functions.  Then, add a line in the Phantom::Phantom() constructor
   to register your ForceEffect with the Phantom device.  The Phantom class will then manage starting, stopping, and calling Draw*() to render the
   effect when active.  If your effect would like to respond to ADDITIONAL commands sent as VREvents from a client, such as changes in effect
   parameters, then it is free to do that by registering a VREvent listener with the EventMgr.  The functions in that static ForceMessages class can
   help to build consistent event name strings.  All of the (currently) existing force effects include parameters that can be set with VREvents and
   provide good examples of how to support this.
 */
class ForceEffect {
public:
    virtual const std::string Name() const = 0;
    virtual void OnStartEffect() = 0;
    virtual void OnStopEffect() = 0;
    virtual void DrawHaptics() = 0;
    virtual void DrawGraphics() = 0;
    
    ForceEffect() {}
    virtual ~ForceEffect() {}
};

#endif
