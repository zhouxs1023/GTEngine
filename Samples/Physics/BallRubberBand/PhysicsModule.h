// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <Mathematics/GteVector2.h>
using namespace gte;

class PhysicsModule
{
public:
    // Construction.
    PhysicsModule();

    // Initialize the system.
    void Initialize(float time, float deltaTime,
        Vector2<float> const& initialPosition, Vector2<float> const& initialVelocity);

    // Apply a single step of the simulation.
    void Update ();

    // Access the current state.
    inline float GetTime () const;
    inline float GetDeltaTime () const;
    inline Vector2<float> const& GetPosition () const;
    inline Vector2<float> const& GetVelocity () const;
    inline float GetFrequency () const;

    // Physical constants.
    float springConstant;  // c
    float mass;  // m

private:
    void Evaluate ();

    // State variables.
    float mTime, mDeltaTime;
    Vector2<float> mPosition, mVelocity;

    // Auxiliary variables.
    Vector2<float> mInitialPosition;
    float mFrequency;  // sqrt(c/m)
    Vector2<float> mVelDivFreq;  // initialVelocity/frequency
};

inline float PhysicsModule::GetTime() const
{
    return mTime;
}

inline float PhysicsModule::GetDeltaTime() const
{
    return mDeltaTime;
}

inline Vector2<float> const& PhysicsModule::GetPosition() const
{
    return mPosition;
}

inline Vector2<float> const& PhysicsModule::GetVelocity() const
{
    return mVelocity;
}

inline float PhysicsModule::GetFrequency() const
{
    return mFrequency;
}
