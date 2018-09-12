// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.15.0 (2018/07/25)

#pragma once

#include <Graphics/GteCamera.h>
#include <Graphics/GteNode.h>

namespace gte
{

class GTE_IMPEXP Trackcylinder
{
public:
    // Construction.  The window rectangle is assumed to be defined in
    // right-handed coordinates, so if you use a window client rectangle
    // for the trackball and this rectangle is in left-handed coordinates,
    // you must reflect the y-values in SetInitialPoint and SetFinalPoint
    // by (ySize - 1 - y).  A root node is used to represent the trackcylinder
    // orientation.  Objects may be attached and detached as desired.
    Trackcylinder();
    Trackcylinder(int xSize, int ySize, std::shared_ptr<Camera> const& camera);

    // Member access.  The Set function is for deferred construction after
    // a default construction of a trackball.
    void Set(int xSize, int ySize, std::shared_ptr<Camera> const& camera);
    inline int GetXSize() const;
    inline int GetYSize() const;
    inline std::shared_ptr<Camera> const& GetCamera() const;
    inline std::shared_ptr<Node> const& GetRoot() const;

    // The rotations are applied as pitch*yaw*vector.
    inline float GetYaw() const;
    inline float GetPitch() const;

    // The root node is the top-level node of a hierarchy whose local
    // transformation is the trackball orientation relative to the specified
    // camera.  The camera directions {D,U,R} act as the world coordinate
    // system.
    void Attach(std::shared_ptr<Spatial> const& object);
    void Detach(std::shared_ptr<Spatial> const& object);
    void DetachAll();
    inline void Update(double applicationTime = 0.0);

    // Set the arc on the sphere along which the trackball rotates.  The
    // standard use is to set the initial point via a mouse button click on a
    // window rectangle.  Mark the trackball as active and set final points
    // based on the locations of the dragged mouse.  Once the mouse button is
    // released, mark the trackball as inactive.
    inline void SetActive(bool active);
    inline bool GetActive() const;

    // Set the initial point of the arc.  The current trackball orientation is
    // recorded.  On construction, the initial point is set to the rectangle
    // center.
    void SetInitialPoint(int x, int y);

    // Set the final point of the arc.  The trackball orientation is updated
    // by the incremental rotation implied by the arc endpoints.
    void SetFinalPoint(int x, int y);

    // Reset the trackcylinder rotation to the identity.
    void Reset();

protected:
    // The window rectangle is mXSize-by-mYSize.  Let the initial point be
    // (x0,y0) and the final point be (x1,y1).  Let dx = x1 - x0 and
    // dy = y1 - y0.  If dx >= dy, then the yaw angle in [-pi,pi] is modified
    // according to yaw = pi*dx/xSize.  If dy > dx, then the pitch angle in
    // [-pi/2,pi/2] is modified according to pitch = -pi*dy/ySize but is
    // clamped to [-pi/2,pi/2].
    int mXSize, mYSize, mX0, mY0, mX1, mY1;
    std::shared_ptr<Camera> mCamera;
    std::shared_ptr<Node> mRoot;
    float mInitialYaw, mYaw;  // rotation about z-axis
    float mInitialPitch, mPitch;  // rotation about y-axis
    bool mActive, mValidTrackcylinder;
};


inline int Trackcylinder::GetXSize() const
{
    return mXSize;
}

inline int Trackcylinder::GetYSize() const
{
    return mYSize;
}

inline std::shared_ptr<Camera> const& Trackcylinder::GetCamera() const
{
    return mCamera;
}

inline std::shared_ptr<Node> const& Trackcylinder::GetRoot() const
{
    return mRoot;
}

inline float Trackcylinder::GetYaw() const
{
    return mYaw;
}

inline float Trackcylinder::GetPitch() const
{
    return mPitch;
}

inline void Trackcylinder::Update(double applicationTime)
{
    mRoot->Update(applicationTime);
}

inline void Trackcylinder::SetActive(bool active)
{
    mActive = active;
}

inline bool Trackcylinder::GetActive() const
{
    return mActive;
}

}
