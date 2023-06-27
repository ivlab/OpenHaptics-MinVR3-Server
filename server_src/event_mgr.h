
#ifndef FORCE_SERVER_EVENT_MGR_H
#define FORCE_SERVER_EVENT_MGR_H

#include <minvr3.h>
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
class VREventCallbackSpecific : public VREventCallback
{
public:
    typedef void (T::*MethodType)(VREvent* event);

    VREventCallbackSpecific(T *obj, MethodType method) : obj_(obj), method_(method) {}
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
    EventMgr();
    virtual ~EventMgr();

    
    // Add a listener for ALL events
    template <class T>
    void AddListener(T* object_ptr, void (T::*object_method_ptr)(VREvent*)) {
        VREventCallback* cb = new VREventCallbackSpecific<T>(object_ptr, object_method_ptr);
        listeners_.push_back(cb);
    }
    
    // Add a listener for a specific event by name.  To listen for an event that starts with "substring", use
    // "substring*" as the event_name.
    template <class T>
    void AddListenerByName(const std::string &event_name, T* object_ptr, void (T::*object_method_ptr)(VREvent*)) {
        if (named_listeners_.find(event_name) != named_listeners_.end()) {
            std::vector<VREventCallback*> callbacks;
            named_listeners_.insert( { event_name, callbacks } );
        }
        VREventCallback* cb = new VREventCallbackSpecific<T>(object_ptr, object_method_ptr);
        named_listeners_[event_name].push_back(cb);
    }

    void QueueEvent(VREvent *e);
    void ProcessQueue();

    void QueueForClient(VREvent *e);
    void ProcessClientQueue(SOCKET *client_fd);
    
private:
    void clear_queue(std::vector<VREvent*> *event_queue);
    
    std::vector<VREvent*> queue_;
    std::vector<VREventCallback*> listeners_;
    std::map<std::string, std::vector<VREventCallback*>> named_listeners_;
    std::vector<VREvent*> client_queue_;
};

#endif
