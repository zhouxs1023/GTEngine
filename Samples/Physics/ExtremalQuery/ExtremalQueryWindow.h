// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

// Expose this for the BSP-based query.  Comment it out for the projection-based query.
//#define USE_BSP_QUERY

// Uncomment this for timing information.
//#define MEASURE_TIMING_OF_QUERY

class ExtremalQueryWindow : public Window3
{
public:
    ExtremalQueryWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers) override;

private:
    void CreateScene();
    void CreateConvexPolyhedron(int numVertices);
    void CreateVisualConvexPolyhedron();
    void UpdateExtremePoints();

    std::unique_ptr<Polyhedron3<float>> mConvexPolyhedron;
    std::unique_ptr<ExtremalQuery3<float>> mExtremalQuery;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mConvexMesh, mMaxSphere, mMinSphere;
    std::shared_ptr<RasterizerState> mWireState;
};
