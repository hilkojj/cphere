#ifndef SERVER_H
#define SERVER_H

class Client;

#include "../level/level.h"
#include "events.h"

class Server
{
  public:

    Level level;

    Server(Level level);

    std::vector<std::shared_ptr<Client>> clients;

    virtual void update(double deltaTime) = 0;

    void updateClients(double deltaTime);

    // wait until (multithreaded) update() or updateClients() is done
    void wait();

  protected:
    bool done = false;
    std::vector<ServerEvent> events;
};

class LocalServer : public Server
{
  public:
    LocalServer(Level level, const char *loadFilePath = NULL);

    void update(double deltaTime) override;
    
};

#endif
