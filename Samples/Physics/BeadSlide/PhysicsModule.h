// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <Mathematics/GteVector2.h>
#include <Mathematics/GteOdeRungeKutta4.h>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    // Construction.
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(float time, float deltaTime, float q, float qDot);

    // Take a single step of the solver.
    void Update();

    // Access the current state.
    inline float GetTime () const;
    inline float GetQ () const;
    inline float GetQDot () const;

    // Physical constants.
    float gravity;
    float mass;

private:
    // State and auxiliary variables.
    Vector2<float> mState;
    float mTime, mAux[1];

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<float, Vector2<float>> Solver;
    std::unique_ptr<Solver> mSolver;
};

inline float PhysicsModule::GetTime() const
{
    return mTime;
}

inline float PhysicsModule::GetQ() const
{
    return mState[0];
}

inline float PhysicsModule::GetQDot() const
{
    return mState[1];
}
