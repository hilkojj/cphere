#ifndef EVENTS_H
#define EVENTS_H

#include <memory>
#include <vector>

#include "../serialization.h"

class Server;

enum GameEventsServer
{
    LEVEL_READY
};

enum GameEventsClient
{
    TEST
};

class BaseGameEvent
{
  public:

    BaseGameEvent(uint16 typeInt);

    std::vector<unsigned char> *getSerialized();

    virtual void fromSerialized(std::vector<unsigned char> *data) = 0;

  private:
    std::vector<unsigned char> serialized;

    uint16 typeInt;

    virtual void serialize() = 0;

};

class BaseServerEvent : public BaseGameEvent
{
  public:
    virtual GameEventsServer type() const = 0;
};

class BaseClientEvent : public BaseGameEvent
{
  public:
    virtual GameEventsClient type() const = 0;

    virtual void handleServerSide(Server &server) = 0;
};

typedef std::shared_ptr<BaseServerEvent> ServerEvent;
typedef std::shared_ptr<BaseClientEvent> ClientEvent;

#endif
