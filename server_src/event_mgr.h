
#ifndef FORCE_SERVER_EVENT_MGR_H
#define FORCE_SERVER_EVENT_MGR_H

#include <minvr.h>
#include <vector>
#include <map>


// Functors allow for callback functions that are class methods
// in C++.  Google "C++ functors" for more information.
class VREventCallback
{
public:
    VREventCallback() {}
    virtual ~VREventCallback() {}
    virtual void Invoke(VREvent* event) = 0;
};

template <class T>
class VREventCallbackSpecific : public OnVREventCallback
{
public:
    typedef void (T::*MethodType)(VREvent* event);

    VREventCallbackSpecific(T *obj, MethodType method) {
        _obj = obj;
        _method = method;
    }

    virtual ~VREventCallbackSpecific() {}

    void Invoke(VREvent* event) {
        (obj_->*method_)(event);
    }
  
protected:
    T          *obj_;
    MethodType  method_;
};
// End functors



class EventMgr {
public:
    
    // Add a listener for ALL events
    template <class T>
    void AddListener(VREventCallback *func, T *this_ptr, void (T::*method)(VREvent*)) {
        VREventCallback *cb = new VREventCallbackSpecific<T>(this_ptr, method);
        listeners_.push_back(cb);
    }
    
    // Add a listener for a specific event by name.  To listen for an event that starts with "substring", use
    // "substring*" as the event_name.
    template <class T>
    void AddListener(const std::string &event_name, VREventCallback *func, T *this_ptr, void (T::*method)(VREvent*)) {
        if (!named_listeners_.contains(event_name)) {
            std::vector<VREventCallback*> callbacks;
            named_listeners_.insert(event_name, callbacks);
        }
        VREventCallback *cb = new VREventCallbackSpecific<T>(this_ptr, method);
        named_listeners_[event_name].push_back(cb);
    }

    void QueueEvent(VREvent *e);
    void ProcessQueue();

    void QueueForClient(VREvent *e);
    void ProcessClientQueue(SOCKET *client_fd);
    
private:
    void clear_queue(std::vector<VREvent*> event_queue);
    
    std::vector<VREvent*> queue_;
    std::vector<OnVREventCallback*> listeners_;
    std::map<std::string, std::vector<VREventCallback*>> named_listeners_;
    std::vector<VREvent*> client_queue_;
};

#endif
