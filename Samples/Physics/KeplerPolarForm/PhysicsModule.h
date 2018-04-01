// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <Mathematics/GteVector4.h>
#include <Mathematics/GteOdeRungeKutta4.h>
#include <Mathematics/GteConstants.h>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    // Construction.
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(float time, float deltaTime, float theta, float thetaDot,
        float radius, float radiusDot);

    // Member access.
    float GetPeriod() const;

    // Apply a single step of the solver.
    void Update();

    // Access the current state.
    inline float GetTime() const;
    inline float GetTheta() const;
    inline float GetThetaDot() const;
    inline float GetRadius() const;
    inline float GetRadiusDot() const;

    // physical constants
    float gravity;
    float mass;

private:
    // State and auxiliary variables.
    Vector4<float> mState;
    float mTime, mAux[5];

    // Ellipse parameters.
    float mEccentricity, mRho, mMajorAxis, mMinorAxis;

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<float, float> Solver;
    std::unique_ptr<Solver> mSolver;
};

inline float PhysicsModule::GetTime() const
{
    return mTime;
}

inline float PhysicsModule::GetTheta() const
{
    return mState[0];
}

inline float PhysicsModule::GetThetaDot() const
{
    return mState[1];
}

inline float PhysicsModule::GetRadius() const
{
    return mState[2];
}

inline float PhysicsModule::GetRadiusDot() const
{
    return mState[3];
}
