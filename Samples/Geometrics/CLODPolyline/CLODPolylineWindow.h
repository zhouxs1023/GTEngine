// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.1 (2019/04/13)

#pragma once

#include <Applications/GteWindow2.h>
#include <Mathematics/GteCLODPolyline.h>
using namespace gte;

class CLODPolylineWindow : public Window2
{
public:
    CLODPolylineWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    inline void Get(Vector<3, float> const& vertex, int& x, int& y)
    {
        float fsize = static_cast<float>(mXSize);
        x = static_cast<int>(0.25f * fsize * (vertex[0] + 2.0f));
        y = mXSize - 1 - static_cast<int>(0.25f * fsize * (vertex[1] + 2.0f));
    }

    std::unique_ptr<CLODPolyline<3, float>> mPolyline;
};
