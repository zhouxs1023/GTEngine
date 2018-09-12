// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.15.0 (2018/07/25)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Mathematics/GteFunctions.h>
#include <Graphics/GteTrackcylinder.h>
using namespace gte;

Trackcylinder::Trackcylinder()
    :
    mXSize(0),
    mYSize(0),
    mX0(0),
    mY0(0),
    mX1(0),
    mY1(0),
    mInitialYaw(0.0f),
    mYaw(0.0f),
    mInitialPitch(0.0f),
    mPitch(0.0f),
    mActive(false),
    mValidTrackcylinder(false)
{
    mRoot = std::make_shared<Node>();
}

Trackcylinder::Trackcylinder(int xSize, int ySize, std::shared_ptr<Camera> const& camera)
    :
    mXSize(0),
    mYSize(0),
    mX0(0),
    mY0(0),
    mX1(0),
    mY1(0),
    mInitialYaw(0.0f),
    mYaw(0.0f),
    mInitialPitch(0.0f),
    mPitch(0.0f),
    mActive(false),
    mValidTrackcylinder(false)
{
    Set(xSize, ySize, camera);
    mRoot = std::make_shared<Node>();
}

void Trackcylinder::Set(int xSize, int ySize, std::shared_ptr<Camera> const& camera)
{
    if (xSize > 0 && ySize > 0 && camera)
    {
        mXSize = xSize;
        mYSize = ySize;
        mCamera = camera;
        mX0 = mXSize / 2;
        mY0 = mYSize / 2;
        mX1 = mX0;
        mY1 = mY0;
        mValidTrackcylinder = true;
    }
    else
    {
        LogError("Invalid Trackcylinder parameters.");
        mValidTrackcylinder = false;
    }
}

void Trackcylinder::Attach(std::shared_ptr<Spatial> const& object)
{
    if (mValidTrackcylinder && object)
    {
        mRoot->AttachChild(object);
    }
}

void Trackcylinder::Detach(std::shared_ptr<Spatial> const& object)
{
    if (mValidTrackcylinder && object)
    {
        mRoot->DetachChild(object);
    }
}

void Trackcylinder::DetachAll()
{
    mRoot->DetachAllChildren();
}

void Trackcylinder::SetInitialPoint(int x, int y)
{
    if (mValidTrackcylinder)
    {
        mX0 = x;
        mY0 = y;
        mInitialYaw = mYaw;
        mInitialPitch = mPitch;
    }
}

void Trackcylinder::SetFinalPoint(int x, int y)
{
    if (mValidTrackcylinder)
    {
        float const pi = (float)GTE_C_PI;
        float const halfPi = (float)GTE_C_HALF_PI;

        mX1 = x;
        mY1 = y;
        if (mX1 != mX0 || mY1 != mY0)
        {
            int dx = mX1 - mX0, dy = mY1 - mY0;
            float angle = dx * pi / mXSize;
            mYaw = mInitialYaw + angle;
            angle = -dy * pi / mYSize;
            mPitch = mInitialPitch + angle;
            mPitch = Function<float>::Clamp(mPitch, -halfPi, halfPi);

            // The angle order depends on camera {D=0, U=1, R=2}.
#if defined(GTE_USE_MAT_VEC)
            AxisAngle<4, float> yawAxisAngle(Vector4<float>::Unit(2), mYaw);
            Matrix4x4<float> yawRotate = Rotation<4, float>(yawAxisAngle);
            AxisAngle<4, float> pitchAxisAngle(Vector4<float>::Unit(1), mPitch);
            Matrix4x4<float> pitchRotate = Rotation<4, float>(pitchAxisAngle);
            Matrix4x4<float> rotate = pitchRotate * yawRotate;
#else
            AxisAngle<4, float> yawAxisAngle(Vector4<float>::Unit(2), -mYaw);
            Matrix4x4<float> yawRotate = Rotation<4, float>(yawAxisAngle);
            AxisAngle<4, float> pitchAxisAngle(Vector4<float>::Unit(1), -mPitch);
            Matrix4x4<float> pitchRotate = Rotation<4, float>(pitchAxisAngle);
            Matrix4x4<float> rotate = yawRotate * pitchRotate;
#endif
            mRoot->localTransform.SetRotation(rotate);
            mRoot->Update();
        }
    }
}

void Trackcylinder::Reset()
{
    mInitialYaw = 0.0f;
    mInitialPitch = 0.0f;
    mYaw = 0.0f;
    mPitch = 0.0f;
    mRoot->localTransform.MakeIdentity();
    mRoot->Update();
}
