#pragma once

#include <GTEngine.h>
using namespace gte;

class MovingSphereBoxWindow : public Window3
{
public:
    MovingSphereBoxWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    enum { DENSITY = 32 };
    void CreateScene();
    void CreateRoundedBoxVertices();
    void CreateRoundedBoxEdges();
    void CreateRoundedBoxFaces();
    void CreateBox();

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<RasterizerState> mNoCullState;
    float mAlpha;

    // Octants of spheres for the rounded box corners.
    std::array<std::shared_ptr<Visual>, 8> mVertexVisual;
    std::array<Vector4<float>, 8> mVNormal;

    // Quarter cylinders for the rounded box edges.
    std::array<std::shared_ptr<Visual>, 12> mEdgeVisual;
    std::array<Vector4<float>, 12> mENormal;

    // Rectangles for the rounded box faces.
    std::array<std::shared_ptr<Visual>, 6> mFaceVisual;
    std::array<Vector4<float>, 6> mFNormal;

    // The visual representation of mBox.
    std::shared_ptr<Visual> mBoxVisual;

    // The visual representation of mSphere.
    std::shared_ptr<Visual> mSphereVisual;

    // The visual representation of the semi-infinite cylinder
    // that indicates the moving path of the sphere.
    std::shared_ptr<Visual> mVelocityVisual;  // ray
    std::shared_ptr<Visual> mCylinderVisual;  // finite cone with large height

    AlignedBox3<float> mBox;
    Sphere3<float> mSphere;
    FIQuery<float, AlignedBox3<float>, Sphere3<float>> mQuery;
    Vector3<float> mSphereVelocity;
};
