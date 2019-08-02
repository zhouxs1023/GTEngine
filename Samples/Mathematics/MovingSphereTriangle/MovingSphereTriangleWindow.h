// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.21.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow3.h>
#include <Mathematics/GteArbitraryPrecision.h>
#include <Mathematics/GteIntrSphere3Triangle3.h>
using namespace gte;

// Comment out this line to use the arbitrary-precision query for the
// intersection of a moving sphere and triangle.
#define USE_FLOATING_POINT_QUERY

class MovingSphereTriangleWindow : public Window3
{
public:
    MovingSphereTriangleWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void CreateTriangleFaces();
    void CreateHalfCylinders();
    void CreateSphereWedges();
    void CreateSpheres();
    void CreateMotionCylinder();
    void UpdateSphereVelocity();
    void UpdateSphereCenter();

    void CreateHalfCylinder(int i, Vector3<float> const& P0, Vector3<float> const& P1,
        Vector3<float> const& normal, float radius);

    void CreateSphere(int i, Vector3<float> const& C, float radius);

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<RasterizerState> mNoCullState;
    float mAlpha;

    std::shared_ptr<Node> mSSVNode;

    // Sphere wedges for the triangle vertices.
    std::array<std::shared_ptr<Visual>, 3> mVertexVisual;

    // Half cylinders for the triangle edges.
    std::array<std::shared_ptr<Visual>, 3> mEdgeVisual;

    // Triangle faces.
    std::array<std::shared_ptr<Visual>, 2> mFaceVisual;

    // The visual representation of mSphere.
    std::shared_ptr<Visual> mSphereVisual;
    std::shared_ptr<Visual> mSphereContactVisual;

    // The visual representation of mTriangle.
    std::shared_ptr<Visual> mTriangleVisual;

    // The visual representation of the moving path of the sphere.
    std::shared_ptr<Visual> mVelocityVisual;

    // The contact point representation.
    std::shared_ptr<Visual> mPointContactVisual;

    Sphere3<float> mSphere;
    Triangle3<float> mTriangle;
    Vector3<float> mTriangleNormal;
    Vector3<float> mSphereVelocity;
    Vector3<float> mTriangleVelocity;

#if defined(USE_FLOATING_POINT_QUERY)
    FIQuery<float, Sphere3<float>, Triangle3<float>> mQuery;
#else
    typedef BSRational<UIntegerAP32> Rational;
    FIQuery<Rational, Sphere3<Rational>, Triangle3<Rational>> mQuery;
#endif

    int mNumSamples0, mNumSamples1, mSample0, mSample1;
    float mDX, mDY, mDZ;
    std::string mMessage;
    bool mDrawSphereVisual;
};
