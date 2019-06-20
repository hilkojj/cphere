#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <limits>

#include "glm/glm.hpp"
#include "utils/gu_error.h"

namespace slz
{

template <class any>
inline void add(const any &in, std::vector<unsigned char> &out)
{
    auto size = sizeof(any);
    int startI = out.size();
    out.resize(startI + size);

    memcpy(&out[startI], &in, size);
}

template <class any>
void get(const std::vector<unsigned char> &in, unsigned int index, any *dest)
{
    auto size = sizeof(any);

    if (in.size() < index + size)
        throw gu_err(
            "requested item (with size " + std::to_string(size)
            + ") does not fit in input-vector (of size " + std::to_string(in.size())
            + ") at index " + std::to_string(index));

    memcpy(dest, &in[index], size);
}

template <class any>
any get(const std::vector<unsigned char> &in, unsigned int index)
{
    any item;
    get(in, index, &item);
    return item;
}

template <class intType = uint32, int maxVal = 1, int minVal = 0>
class Float
{
  public:

    static const unsigned int 
        size = sizeof(intType), 
        range = maxVal - minVal;

    template <class vecType>
    static unsigned int vecSize()
    {
        return size * vecType::length();
    }

    static intType serialize(float x)
    {
        return ((x - minVal) / (float) range * std::numeric_limits<intType>::max() + .5);
    }

    static float deserialize(intType y)
    {
        return (y / (float) std::numeric_limits<intType>::max()) * range + minVal;
    }

    static float deserialize(const std::vector<unsigned char> &data, uint32 index)
    {
        return deserialize(slz::get<intType>(data, index));
    }

    template <class vecType>
    static void serializeVec(const vecType &v, std::vector<unsigned char> &out)
    {
        serializeVec(&v, out);
    }

    template <class vecType>
    static void serializeVec(const vecType *vPtr, std::vector<unsigned char> &out)
    {
        for (int i = 0; i < vecType::length(); i++)
        {
            intType val = serialize(vPtr->operator[](i));
            int dstI = out.size();

            out.resize(out.size() + size, 0);

            memcpy(&out.at(dstI), &val, size);
        }
    }

    template <class vecType, class inType>
    static void deserializeVec(const inType *in, vecType &out)
    {
        deserializeVec(in, &out);
    }

    template <class vecType, class inType>
    static void deserializeVec(const inType *in, vecType *out)
    {
        for (int i = 0; i < vecType::length(); i++)
        {
            intType intVal;
            memcpy(&intVal, in + i * size, size);
            out->operator[](i) = deserialize(intVal);
        }
    }

    template <class vecType>
    static void serializeVecs(const std::vector<vecType> &v, std::vector<unsigned char> &out)
    {
        serializeVecs(&v[0], v.size(), out);
    }

    template <class vecType>
    static void serializeVecs(const vecType *v, unsigned int n, std::vector<unsigned char> &out)
    {
        for (int i = 0; i < n; i++)
            serializeVec(v[i], out);
    }

    template <class vecType, class inType>
    static void deserializeVecs(const inType *in, unsigned int n, std::vector<vecType> &out, uint32 outOffset)
    {
        unsigned int offset = vecType::length() * size;
        out.resize(n + outOffset);
        for (int i = 0; i < n; i++)
            deserializeVec(in + i * offset, &out[outOffset + i]);
    }

    template <class vecType>
    static void deserializeVecs(const std::vector<unsigned char> &in, unsigned int startIndex, unsigned int n, std::vector<vecType> &out, uint32 outOffset)
    {
        if (in.size() < startIndex + n * vecSize<vecType>())
            throw gu_err(
                "requested " + std::to_string(n) + " items (with size " + std::to_string(vecSize<vecType>())
                + ") do not fit in input-vector (of size " + std::to_string(in.size())
                + ") starting at index " + std::to_string(startIndex));

        deserializeVecs(&in[startIndex], n, out, outOffset);
    }
};

} // namespace slz

#endif
