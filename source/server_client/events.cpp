#include "events.h"

BaseGameEvent::BaseGameEvent(uint16 typeInt) : typeInt(typeInt)
{}

std::vector<unsigned char> *BaseGameEvent::getSerialized()
{
    if (serialized.empty())
    {
        slz::add(typeInt, serialized);
        serialize();
    }
    return &serialized;
}
