#ifndef FORCE_SERVER_PHANTOM_H
#define FORCE_SERVER_PHANTOM_H

#include "open_haptics.h"

#include <map>
#include <string>

#include <minvr3.h>

#include "force_effect.h"

class EventMgr;

class Phantom {
public:
    Phantom(EventMgr* event_mgr);
    virtual ~Phantom();

    bool Init(const std::string &device_name = "Default PHANToM");    
    void RegisterForceEffect(ForceEffect* effect);
    void PollForInput();    
    void Reset();

    void OnWorldToWorkspaceTranslationUpdate(VREvent* e);
    void OnWorldToWorkspaceRotationUpdate(VREvent* e);
    void OnWorldToWorkspaceScaleUpdate(VREvent* e);

    void BeginHapticFrame();
    void DrawHaptics();
    void EndHapticFrame();

    // This should also be called once per frame from the main rendering loop.
    // Since it renders graphics only, it can safely be called within or outside
    // of the Begin/EndHapticFrame() pair. 
    void DrawGraphics();

    // Prints an error message and returns true if an error occurred, false is everything is ok
    static bool CheckHapticError();

    double* transform();
    double* position();
    double* rotation();

    double* custom_workspace_dims();
    double* custom_workspace_center();
    double* custom_workspace_size();

    bool is_primary_btn_down();
    bool is_in_custom_workspace();
    
    hduMatrix get_world_to_custom_workspace_matrix();

protected:
    EventMgr* event_mgr_;
    HHD hd_device_;
    HHLRC hl_context_;

    // user specified transformation from the world coordinates used by all of the haptic
    // effects into custom workspace coordinates
    hduVector3Dd world_to_custom_workspace_translation_;
    hduQuaternion world_to_custom_workspace_rotation_;
    hduVector3Dd world_to_custom_workspace_scale_;

    // usable space of the phantom device, in device coordinates (mm)
    HDdouble custom_workspace_dims_[6];
    HDdouble custom_workspace_center_[3];
    HDdouble custom_workspace_size_[3];

    // cached stylus state -- updated at beginning of each frame
    HDdouble transform_[16];
    HDdouble position_[3];
    HDdouble rotation_[4];
    HLboolean primary_down_;

    std::map<std::string, ForceEffect*> effects_;
};


#endif
