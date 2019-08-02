// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/04/16)

#pragma once

#include <Applications/GteWindow3.h>
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
