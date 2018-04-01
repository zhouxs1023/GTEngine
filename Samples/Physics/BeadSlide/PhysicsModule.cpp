// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    gravity(0.0f),
    mass(0.0f),
    mState(Vector2<float>::Zero()),
    mTime(0.0f)
{
    mAux[0] = 0.0f;
}

void PhysicsModule::Initialize(float time, float deltaTime, float q, float qDot)
{
    mTime = time;

    // State variables.
    mState[0] = q;
    mState[1] = qDot;

    // Auxiliary variable.
    mAux[0] = gravity;

    // RK4 differential equation solver.
    std::function<Vector2<float>(float, Vector2<float> const&)> odeFunction
        =
        [this](float, Vector2<float> const& input) -> Vector2<float>
        {
            float qSqr = input[0] * input[0];
            float qDotSqr = input[1] * input[1];
            float numer = -3.0f * mAux[0] * qSqr - 2.0f * input[0] * (2.0f + 9.0f * qSqr) * qDotSqr;
            float denom = 1.0f + qSqr * (4.0f + 9.0f * qSqr);
            float qDotFunction = numer / denom;

            // (q, dot(q))
            return Vector2<float>{ input[1], qDotFunction };
        };

    mSolver = std::make_unique<Solver>(deltaTime, odeFunction);
}

void PhysicsModule::Update()
{
    // Apply a single step of the ODE solver.
    if (mSolver)
    {
        mSolver->Update(mTime, mState, mTime, mState);
    }
}
