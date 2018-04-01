// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

#define DEMONSTRATE_VIEWPORT_BOUNDING_RECTANGLE
#define DEMONSTRATE_POST_PROJECTION_REFLECTION

class BillboardNodesWindow : public Window3
{
public:
    BillboardNodesWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers) override;

private:
    bool SetEnvironment();
    void CreateScene();

    // All triangle meshes have this common vertex format.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    Culler mCuller;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Texture2> mGroundTexture;
    std::shared_ptr<Texture2> mSkyTexture;

    // Billboard 0 has a rectangle attached.  Billboard 1 has a torus
    // attached.
    std::shared_ptr<Visual> mGround, mRectangle, mTorus;
    std::shared_ptr<BillboardNode> mBillboard0, mBillboard1;

#if defined(DEMONSTRATE_VIEWPORT_BOUNDING_RECTANGLE)
    // Compute the bounding rectangle in normalized display coordinates
    // [-1,1]^2 for the torus.
    void ComputeTorusBoundingRectangle();

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<RasterizerState> mNoCullState;
#endif

#if defined(DEMONSTRATE_POST_PROJECTION_REFLECTION)
    std::shared_ptr<RasterizerState> mCullCWState;
#endif
};
