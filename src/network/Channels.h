//
// Created by cjw on 21-4-1.
//

#ifndef SERVERPROJECT_CHANNELS_H
#define SERVERPROJECT_CHANNELS_H

#include "Dispatcher.h"
#include <functional>
#include <cstdint>

class Channels
{
  public:
    enum EventOption
    {
        kEventAdd = 0,
        kEventMod,
        kEventDel
    };

    explicit Channels(Dispatcher::Ptr&loop,int fd);
    ~Channels();

    virtual int HandleRead(){return 0;};
    virtual int HandleWrite(){return 0;};
    virtual int HandleError(){return 0;};

    void HandleEvents(uint32_t events);
    void UpdateEventOption(EventOption event_option, uint32_t event_state);

    void EnableRead();
    void DisableRead();
    void EnableWrite();
    void DisableWrite();

    
    EventOption GetEventOption() { return event_option_; }
    uint32_t GetEvents() { return events_; }

    // void SetFd(int fd){fd_=fd;}
    int GetFd(){return fd_;}
    
    Dispatcher::Ptr disp_;
  
  private:
    int fd_;
    EventOption event_option_;
    uint32_t events_;
};

#endif  // SERVERPROJECT_CHANNELS_H
