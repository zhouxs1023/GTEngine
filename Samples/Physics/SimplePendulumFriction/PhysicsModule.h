// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.3.0 (2016/07/16)

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
    void Initialize(float time, float deltaTime, float theta, float thetaDot);

    // Apply a single step of the solver.
    void Update();

    // Access the current state.
    inline float GetTheta() const;

    // pendulum parameters
    float cDivM;  // friction_coefficient / mass
    float gDivL;  // gravity_constant / pendulum_length

private:
    // State and auxiliary variables.
    float mTime;
    Vector2<float> mState;
    float mAux[2];

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<float, Vector2<float>> Solver;
    std::unique_ptr<Solver> mSolver;
};

inline float PhysicsModule::GetTheta() const
{
    return mState[0];
}
