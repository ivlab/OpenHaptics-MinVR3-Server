
#include "event_mgr.h"

EventMgr::EventMgr() {
    
}

EventMgr::~EventMgr() {

}

void EventMgr::QueueEvent(VREvent *e) {
    queue_.push_back(e);
}

void EventMgr::ProcessQueue() {
    for (int j=0; j<queue_.size(); j++) {
        // invoke listeners that listen for all events
        for (int i=0; i<listeners_.size(); i++) {
          listeners_[i]->Invoke(queue_[j]);
        }
        // if the event name matches, invoke listeners that only listen for a specific named event
        if (named_listeners_.find(queue_[j]->get_name()) != named_listeners_.end()) {
            for (int k=0; k<named_listeners_[queue_[j]->get_name()].size(); k++) {
                named_listeners_[queue_[j]->get_name()][k]->Invoke(queue_[j]);
            }
        }
        // if the event name matches, invoke listeners that listen for a name that "starts with" a substring
        // this is denoted by a * as the last character in the key of the entry in the named_listeners_ map
        else {
            for (const auto &entry : named_listeners_) {
                if (entry.first[entry.first.length()-1] == '*') {
                    std::string entry_base_name = entry.first.substr(0, entry.first.length()-1);
                    if (queue_[j]->get_name().rfind(entry_base_name, 0) == 0) {
                        for (int k=0; k<entry.second.size(); k++) {
                            entry.second[k]->Invoke(queue_[j]);
                        }
                    }
                }
            }
        }
    }
    clear_queue(&queue_);
}

void EventMgr::QueueForClient(VREvent *e) {
    client_queue_.push_back(e);
}

void EventMgr::ProcessClientQueue(SOCKET *client_fd) {
    if (*client_fd != INVALID_SOCKET) {
        for (int i=0; i<client_queue_.size(); i++) {
            MinVR3Net::SendVREvent(client_fd, *client_queue_[i]);
        }
    }
    clear_queue(&client_queue_);
}

void EventMgr::clear_queue(std::vector<VREvent*> *event_queue) {
    for (int j=0; j<event_queue->size(); j++) {
        delete (*event_queue)[j];
    }
    event_queue->clear();
}
