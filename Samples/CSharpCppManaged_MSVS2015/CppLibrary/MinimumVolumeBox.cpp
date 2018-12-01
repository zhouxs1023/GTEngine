#include "MinimumVolumeBox.h"
#include <Mathematics/GteBSRational.h>
#include <Mathematics/GteUIntegerAP32.h>
#include <Mathematics/GteMinimumVolumeBox3.h>
using namespace gte;

typedef BSRational<UIntegerAP32> Rational;

MVB3::MVB3()
{
}

void MVB3::ComputeMinimumVolumeBoxFromPoints(uint32_t numThreads,
    int numPoints, double const* points, double center[3], double axis[9],
    double extent[3])
{
    if (numPoints > 0 && points)
    {
        MinimumVolumeBox3<double, Rational> mvb(numThreads);
        auto const* vpoints = reinterpret_cast<Vector3<double> const*>(points);
        auto box = mvb(numPoints, vpoints);
        for (uint32_t i = 0; i < 3; ++i)
        {
            center[i] = box.center[i];
            extent[i] = box.extent[i];
            for (uint32_t j = 0; j < 3; ++j)
            {
                axis[3 * i + j] = box.axis[i][j];
            }
        }
    }
    else
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            center[i] = 0.0;
            extent[i] = 0.0;
            for (uint32_t j = 0; j < 3; ++j)
            {
                axis[3 * i + j] = 0.0;
            }
        }
    }
}

void MVB3::ComputeMinimumVolumeBoxFromPolyhedron(uint32_t numThreads,
    int numPoints, double const* points, int numIndices, int const* indices,
    double center[3], double axis[9], double extent[3])
{
    if (numPoints > 0 && points && numIndices > 0 && indices)
    {
        MinimumVolumeBox3<double, Rational> mvb(numThreads);
        auto const* vpoints = reinterpret_cast<Vector3<double> const*>(points);
        auto box = mvb(numPoints, vpoints, numIndices, indices);
        for (uint32_t i = 0; i < 3; ++i)
        {
            center[i] = box.center[i];
            extent[i] = box.extent[i];
            for (uint32_t j = 0; j < 3; ++j)
            {
                axis[3 * i + j] = box.axis[i][j];
            }
        }
    }
    else
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            center[i] = 0.0;
            extent[i] = 0.0;
            for (uint32_t j = 0; j < 3; ++j)
            {
                axis[3 * i + j] = 0.0;
            }
        }
    }
}
