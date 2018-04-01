// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.1.0 (2016/06/28)

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    gravity(0.0f),
    mass(0.0f),
    length(0.0f),
    inertia1(0.0f),
    inertia3(0.0f),
    mTime(0.0f),
    mState(Vector3<float>::Zero())
{
    for (int i = 0; i < 5; ++i)
    {
        mAux[i] = 0.0f;
    }
}

void PhysicsModule::Initialize(float time, float deltaTime, float theta,
    float phi, float psi, float angVel1, float angVel2, float angVel3)
{
    mTime = time;

    float cosPhi = cos(phi), sinPhi = sin(phi);
    float cosPsi = cos(psi), sinPsi = sin(psi);

    // State variables.
    mState[0] = theta;
    mState[1] = phi;
    mState[2] = psi;

    // Auxiliary variables.
    mAux[0] = mass * gravity * length / inertia1;  // alpha
    mAux[1] = angVel1 * angVel1 + angVel2 * angVel2 + 2.0f * cosPhi * mAux[0];  // beta
    mAux[2] = angVel3 * inertia3 / inertia1;  // epsilon
    mAux[3] = sinPhi * (angVel1 * sinPsi + angVel2 * cosPsi) + cosPhi * mAux[2];  // delta
    mAux[4] = angVel3;

    // RK4 differential equation solver.
    std::function<Vector3<float>(float, Vector3<float> const&)> odeFunction
        =
        [this](float, Vector3<float> const& input) -> Vector3<float>
        {
            float cs = cos(input[1]);
            float invSin = 1.0f / sin(input[1]);
            float numer = mAux[3] - mAux[2] * cs;
            float fraction = numer * invSin;
            float arg = mAux[1] - 2.0f * mAux[0] * cs - fraction * fraction;
            float thetaDotFunction = fraction * invSin;
            float phiDotFunction = sqrt(fabs(arg));
            float psiDotFunction = mAux[4] - cs * thetaDotFunction;
            return Vector3<float>{thetaDotFunction, phiDotFunction, psiDotFunction};
        };

    mSolver = std::make_unique<Solver>(deltaTime, odeFunction);
}

Matrix4x4<float> PhysicsModule::GetBodyAxes() const
{
    float cosTheta = cos(mState[0]);
    float sinTheta = sin(mState[0]);
    float cosPhi = cos(mState[1]);
    float sinPhi = sin(mState[1]);
    float cosPsi = cos(mState[2]);
    float sinPsi = sin(mState[2]);

    Vector3<float> N{ cosTheta, sinTheta, 0.0f };
    Vector3<float> axis3{ sinTheta * sinPhi, -cosTheta * sinPhi, cosPhi };
    Vector3<float> axis3xN = Cross(axis3, N);
    Vector3<float> axis1 = cosPsi * N + sinPsi * axis3xN;
    Vector3<float> axis2 = cosPsi * axis3xN - sinPsi * N;

    Matrix4x4<float> rotate;
#if defined(GTE_USE_MAT_VEC)
    rotate.SetCol(0, HLift(axis1, 0.0f));
    rotate.SetCol(1, HLift(axis2, 0.0f));
    rotate.SetCol(2, HLift(axis3, 0.0f));
    rotate.SetCol(3, Vector4<float>::Unit(3));
#else
    rotate.SetRow(0, HLift(axis1, 0.0f));
    rotate.SetRow(1, HLift(axis2, 0.0f));
    rotate.SetRow(2, HLift(axis3, 0.0f));
    rotate.SetRow(3, Vector4<float>::Unit(3));
#endif
    return rotate;
}

void PhysicsModule::Update()
{
    // Apply a single step to the ODE solver.
    if (mSolver)
    {
        mSolver->Update(mTime, mState, mTime, mState);
    }
}
