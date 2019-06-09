#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "glm/glm.hpp"

namespace slz
{

namespace
{

template <class intType, unsigned int precision>
class Float
{
public:

    static const unsigned int size = sizeof(intType);

    static intType from(float x)
    {
        return x * glm::pow(10, precision) + .5;
    }

    static float get(intType y)
    {
        return y / glm::pow(10, precision);
    }

    template <class vecType>
    static void serializeVec(vecType v, std::vector<unsigned char> &out)
    {
        for (int i = 0; i < vecType::length(); i++)
        {
            auto val = from(v[i]);

            int dstI = out.size();

            out.resize(out.size() + size, 0);

            memcpy(&out.at(dstI), &val, size);
        }
    }

    template <class vecType, class inType>
    static void deserializeVec(const inType *in, vecType &out)
    {
        for (int i = 0; i < vecType::length(); i++)
        {
            intType intVal;
            memcpy(&intVal, in + i * size, size);
            out[i] = get(intVal);
        }
    }
};

} // namespace

template <unsigned int precision>
using float_l = Float<int64, precision>;

template <unsigned int precision>
using float_m = Float<int32, precision>;

typedef float_m<4> float_m4;

template <unsigned int precision>
using float_s = Float<int16, precision>;

typedef float_s<2> float_s2;

template <unsigned int precision>
using float_xs = Float<int8, precision>;

typedef float_xs<1> float_xs1;

} // namespace slz

#endif
