// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.3.0 (2016/07/16)

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    cDivM(0.0f),
    gDivL(0.0f),
    mTime(0.0f),
    mState(Vector2<float>::Zero())
{
    for (int i = 0; i < 2; ++i)
    {
        mAux[i] = 0.0f;
    }
}

void PhysicsModule::Initialize(float time, float deltaTime, float theta, float thetaDot)
{
    mTime = time;

    // State variables.
    mState[0] = theta;
    mState[1] = thetaDot;

    // Auxiliary variables.
    mAux[0] = gDivL;
    mAux[1] = cDivM;

    // RK4 differential equation solver.
    std::function<Vector2<float>(float, Vector2<float> const&)> odeFunction
        =
        [this](float, Vector2<float> const& input) -> Vector2<float>
    {
        float thetaFunction = input[1];
        float thetaDotFunction = -(mAux[0] * sin(input[0]) + mAux[1] * input[1]);
        return Vector2<float>{ thetaFunction, thetaDotFunction };
    };

    mSolver = std::make_unique<Solver>(deltaTime, odeFunction);
}

void PhysicsModule::Update ()
{
    // Apply a single step of the ODE solver.
    mSolver->Update(mTime, mState, mTime, mState);
}
