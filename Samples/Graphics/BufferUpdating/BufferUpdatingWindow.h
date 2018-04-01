// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class BufferUpdatingWindow : public Window3
{
public:
    BufferUpdatingWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The surface is a height field of NUM_SAMPLES-by-NUM_SAMPLES vertices.
    enum { NUM_SAMPLES = 1024 };
    std::shared_ptr<Visual> mSurface;
    std::shared_ptr<RasterizerState> mWireState;
};
