#include "MinimumVolumeBox.h"

namespace CLI
{
    MVB3::MVB3()
        :
        ManagedObject(new gte::MVB3())
    {
    }

    void MVB3::ComputeMinimumVolumeBoxFromPoints(unsigned int numThreads,
        int numPoints, array<double>^ points, array<double>^ center,
        array<double>^ axis, array<double>^ extent)
    {
        pin_ptr<double> pinPoints = &points[0];
        pin_ptr<double> pinCenter = &center[0];
        pin_ptr<double> pinAxis = &axis[0];
        pin_ptr<double> pinExtent = &extent[0];

        mInstance->ComputeMinimumVolumeBoxFromPoints(numThreads, numPoints,
            pinPoints, pinCenter, pinAxis, pinExtent);
    }

    void MVB3::ComputeMinimumVolumeBoxFromPolyhedron(unsigned int numThreads,
        int numPoints, array<double>^ points, int numIndices,
        array<int>^ indices, array<double>^ center, array<double>^ axis,
        array<double>^ extent)
    {
        pin_ptr<double> pinPoints = &points[0];
        pin_ptr<int> pinIndices = &indices[0];
        pin_ptr<double> pinCenter = &center[0];
        pin_ptr<double> pinAxis = &axis[0];
        pin_ptr<double> pinExtent = &extent[0];

        mInstance->ComputeMinimumVolumeBoxFromPolyhedron(numThreads, numPoints,
            pinPoints, numIndices, pinIndices, pinCenter, pinAxis, pinExtent);
    }
}
