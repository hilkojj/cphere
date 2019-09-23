#ifndef CLIENT_H
#define CLIENT_H

#include "events.h"

class Client
{
  public:

    virtual void update(const Server *svr, const std::vector<ServerEvent> &events, double deltaTime) = 0;

    virtual ClientEvent getInputEvent() = 0;

  private:
    friend class Server;

    bool updateDone = false;
};

// class LocalClient : public Client
// {
//   public:
//     void feedEvents(const std::vector<ServerEvent> &events);

//     ClientEvent getInputEvent();
// };

#endif
