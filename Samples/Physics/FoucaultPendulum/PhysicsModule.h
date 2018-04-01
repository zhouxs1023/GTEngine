// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.1.0 (2016/06/25)

#pragma once

#include <Mathematics/GteMatrix4x4.h>
#include <Mathematics/GteOdeRungeKutta4.h>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    // Construction.
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(float time, float deltaTime, float theta, float phi,
        float thetaDot, float phiDot);

    // The orientation of the pendulum.
    Matrix4x4<float> GetOrientation() const;

    // Apply a single step of the solver.
    void Update();

    // The pendulum parameters.
    float angularSpeed;  // w
    float latitude;  // lat
    float gDivL;  // g/L

private:
    // State and auxiliary variables.
    float mTime;
    Vector4<float> mState;
    float mAux[3];

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<float, Vector4<float>> Solver;
    std::unique_ptr<Solver> mSolver;
};

