#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <limits>

#include "glm/glm.hpp"

namespace slz
{

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
            auto val = serialize(vPtr->operator[](i));

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
    static void deserializeVecs(const inType *in, unsigned int n, std::vector<vecType> &out)
    {
        unsigned int offset = vecType::length() * size, startI = out.size();
        out.resize(out.size() + n);
        for (int i = 0; i < n; i++)
            deserializeVec(in + i * offset, &out[startI + i]);
    }
};

template <class any>
void add(const any &in, std::vector<unsigned char> &out)
{
    auto size = sizeof(any);
    int startI = out.size();
    out.resize(startI + size);

    memcpy(&out[startI], &in, size);
}

} // namespace slz

#endif
