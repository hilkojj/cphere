#include "server.h"
#include "client.h"

#include "files/file.h"
#include "../planet_generation/earth_generator.h"

Server::Server(Level level) : level(level)
{}

void Server::updateClients(double deltaTime)
{
    for (auto &c : clients)
        c->updateDone = false;
    for (auto &c : clients)
        c->update(this, events, deltaTime);

    done = true;
}

void Server::wait()
{
    // todo: this is busy waiting. must be a better way
    while (!done) continue;
    while (true)
    {
        bool doBreak = true;
        for (auto &c : clients) if (!c->updateDone) doBreak = false;
        if (doBreak) break;
    }
}

LocalServer::LocalServer(Level lvl, const char *loadFilePath) : Server(lvl)
{
    if (loadFilePath)
    {
        auto earthBin = File::readBinary(loadFilePath);
        level.earth.fromBinary(earthBin, [&]() {
            return earthMeshGenerator(&level.earth);
        });
    }
    else generateEarth(&level.earth);
}

void LocalServer::update(double deltaTime)
{
    done = false;
    

    done = true;
}
