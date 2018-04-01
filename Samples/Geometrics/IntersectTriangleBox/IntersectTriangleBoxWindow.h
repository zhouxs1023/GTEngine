// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class IntersectTriangleBoxWindow : public Window3
{
public:
    IntersectTriangleBoxWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseMotion(MouseButton button, int x, int y,
        unsigned int modifiers) override;

private:
    void CreateScene();
    void UpdateVertices(Matrix4x4<float> const& wMatrix);
    void DrawSorted();
    void DoIntersectionQueries();
    void GenerateMessage();

    OrientedBox3<float> mBox;
    Triangle3<float> mTriangle;

    std::shared_ptr<Node> mBoxNode;
    std::shared_ptr<Node> mTriangleNode;

    // In model space, the box is centered at the origin with axes in the
    // coordinate-axis directions.  The faces are stored in the order
    // +x, -x, +y, -y, +z, -z.
    std::array<std::shared_ptr<Visual>, 6> mFaceMesh;
    std::shared_ptr<Visual> mOutsideTriangleMesh;
    std::shared_ptr<Visual> mInsideTriangleMesh;

    // Box colors.  The red effect indicates the box and triangle
    // intersect.  The blue effect indicates the box and triangle do not
    // intersect.
    std::array<std::shared_ptr<ConstantColorEffect>, 6> mRedEffect;
    std::array<std::shared_ptr<ConstantColorEffect>, 6> mBlueEffect;

    // Triangle colors.  The green effect is for that part of the triangle
    // (if any) outside the box.  The gray effect is for that part of the
    // triangle (if any) inside the box.
    std::shared_ptr<ConstantColorEffect> mGreenEffect;
    std::shared_ptr<ConstantColorEffect> mGrayEffect;

    // All color effects have some transparency.
    std::shared_ptr<BlendState> mBlendState;

    // The polygons are double sided.
    std::shared_ptr<DepthStencilState> mNoWriteState;
    std::shared_ptr<RasterizerState> mNoCullState;

    // Use key 'b' to toggle between the box attached to trackball and the
    // box detached from trackball.  Use key 't' similarly for the triangle.
    bool mBoxAttached, mTriangleAttached;
    std::string mMessage;
    float mDeltaTrn;
};
