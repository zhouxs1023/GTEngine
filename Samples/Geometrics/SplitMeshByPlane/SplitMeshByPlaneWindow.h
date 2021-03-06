// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.1 (2019/04/13)

#pragma once

#include <Applications/GteWindow3.h>
#include <Graphics/GteConstantColorEffect.h>
#include <Graphics/GteVertexColorEffect.h>
#include <Mathematics/GteHyperplane.h>
using namespace gte;

class SplitMeshByPlaneWindow : public Window3
{
public:
    SplitMeshByPlaneWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers) override;

private:
    void CreateScene();
    void Update();

    struct TorusVertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    // The application shows a torus split by a plane.  You can rotate the
    // orus using the virtual trackball to see how the mesh is partitioned.
    // The first array stores the torus vertices in model-space coordinates
    // and the second array stores the vertices transformed to world space.
    std::vector<Vector3<float>> mTorusVerticesMS;
    std::vector<Vector3<float>> mTorusVerticesWS;
    std::vector<int> mTorusIndices;
    Plane3<float> mPlane;

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mMeshTorus;
    std::shared_ptr<Visual> mMeshPlane;
    std::shared_ptr<VertexColorEffect> mTorusEffect;
    std::shared_ptr<ConstantColorEffect> mMeshEffect;

    bool mTorusMoved;
};
