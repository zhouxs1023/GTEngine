// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.0 (2019/02/06)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GteIKController.h>
using namespace gte;

IKController::IKController(size_t numJoints, size_t numGoals, size_t numIterations,
    bool orderEndToRoot)
    :
    mJoints(numJoints),
    mGoals(numGoals),
    mNumIterations(numIterations),
    mOrderEndToRoot(orderEndToRoot)
{
}

IKController::~IKController()
{
}

void IKController::InitializeGoal(size_t g, std::shared_ptr<Spatial> const& target,
    std::shared_ptr<Spatial> const& effector, float weight)
{
    if (g < mGoals.size())
    {
        mGoals[g].target = target.get();
        mGoals[g].effector = effector.get();
        mGoals[g].weight = weight;
        return;
    }
    LogError("Invalid index in SetGoal.");
}

void IKController::InitializeJoint(size_t j, std::shared_ptr<Spatial> const& object,
    std::vector<size_t> const& goalIndices)
{
    if (j < mJoints.size())
    {
        mJoints[j].object = object.get();
        mJoints[j].goalIndices = goalIndices;
        return;
    }
    LogError("Invalid index in SetGoal.");
}

void IKController::SetJointAllowTranslation(size_t j, int axis, bool allow)
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        mJoints[j].allowTranslation[axis] = allow;
        return;
    }
    LogError("Invalid index in SetJointAllowTranslation.");
}

void IKController::SetJointMinTranslation(size_t j, int axis, float minTranslation)
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        mJoints[j].minTranslation[axis] = minTranslation;
        return;
    }
    LogError("Invalid index in SetJointMinTranslation.");
}

void IKController::SetJointMaxTranslation(size_t j, int axis, float maxTranslation)
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        mJoints[j].maxTranslation[axis] = maxTranslation;
        return;
    }
    LogError("Invalid index in SetJointMaxTranslation.");
}

void IKController::SetJointAllowRotation(size_t j, int axis, bool allow)
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        mJoints[j].allowRotation[axis] = allow;
        return;
    }
    LogError("Invalid index in SetJointAllowRotation.");
}

void IKController::SetJointMinRotation(size_t j, int axis, float minRotation)
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        mJoints[j].minRotation[axis] = minRotation;
        return;
    }
    LogError("Invalid index in SetJointMinRotation.");
}

void IKController::SetJointMaxRotation(size_t j, int axis, float maxRotation)
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        mJoints[j].maxRotation[axis] = maxRotation;
        return;
    }
    LogError("Invalid index in SetJointMaxRotation.");
}

bool IKController::GetJointAllowTranslation(size_t j, int axis) const
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        return mJoints[j].allowTranslation[axis];
    }
    LogError("Invalid index in GetJointAllowTranslation.");
    return false;
}

float IKController::GetJointMinTranslation(size_t j, int axis) const
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        return mJoints[j].minTranslation[axis];
    }
    LogError("Invalid index in GetJointMinTranslation.");
    return -std::numeric_limits<float>::infinity();
}

float IKController::GetJointMaxTranslation(size_t j, int axis) const
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        return mJoints[j].maxTranslation[axis];
    }
    LogError("Invalid index in GetJointMaxTranslation.");
    return std::numeric_limits<float>::infinity();
}

bool IKController::GetJointAllowRotation(size_t j, int axis) const
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        return mJoints[j].allowRotation[axis];
    }
    LogError("Invalid index in GetJointAllowRotation.");
    return false;
}

float IKController::GetJointMinRotation(size_t j, int axis) const
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        return mJoints[j].minRotation[axis];
    }
    LogError("Invalid index in GetJointMinRotation.");
    return -std::numeric_limits<float>::infinity();
}

float IKController::GetJointMaxRotation(size_t j, int axis) const
{
    if (j < mJoints.size() && 0 <= axis && axis < 3)
    {
        return mJoints[j].maxRotation[axis];
    }
    LogError("Invalid index in GetJointMaxRotation.");
    return std::numeric_limits<float>::infinity();
}

bool IKController::Update(double applicationTime)
{
    if (!Controller::Update(applicationTime))
    {
        return false;
    }

    // Make sure effectors are all current in world space.  It is assumed
    // that the joints form a chain, so the world transforms of joint I
    // are the parent transforms for the joint I+1.
    for (auto& joint : mJoints)
    {
        joint.UpdateWorldSRT();
    }

    // Update joints one-at-a-time to meet goals.  As each joint is updated,
    // the nodes occurring in the chain after that joint must be made current
    // in world space.
    int numJoints = static_cast<int>(mJoints.size());
    if (mOrderEndToRoot)
    {
        for (size_t iter = 0; iter < mNumIterations; ++iter)
        {
            for (int k = 0; k < numJoints; ++k)
            {
                int r = numJoints - 1 - k;
                auto& joint = mJoints[r];

                for (int axis = 0; axis < 3; ++axis)
                {
                    if (joint.allowTranslation[axis])
                    {
                        if (joint.UpdateLocalT(axis, mGoals))
                        {
                            for (int j = r; j < numJoints; ++j)
                            {
                                mJoints[j].UpdateWorldRT();
                            }
                        }
                    }
                }

                for (int axis = 0; axis < 3; ++axis)
                {
                    if (joint.allowRotation[axis])
                    {
                        if (joint.UpdateLocalR(axis, mGoals))
                        {
                            for (int j = r; j < numJoints; ++j)
                            {
                                mJoints[j].UpdateWorldRT();
                            }
                        }
                    }
                }
            }
        }
    }
    else  // order root to end
    {
        for (size_t iter = 0; iter < mNumIterations; ++iter)
        {
            for (int k = 0; k < numJoints; ++k)
            {
                auto& joint = mJoints[k];

                for (int axis = 0; axis < 3; ++axis)
                {
                    if (joint.allowTranslation[axis])
                    {
                        if (joint.UpdateLocalT(axis, mGoals))
                        {
                            for (int j = k; j < numJoints; ++j)
                            {
                                mJoints[j].UpdateWorldRT();
                            }
                        }
                    }
                }

                for (int axis = 0; axis < 3; ++axis)
                {
                    if (joint.allowRotation[axis])
                    {
                        if (joint.UpdateLocalR(axis, mGoals))
                        {
                            for (int j = k; j < numJoints; ++j)
                            {
                                mJoints[j].UpdateWorldRT();
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

IKController::Goal::Goal()
    :
    target(nullptr),
    effector(nullptr),
    weight(0.0f)
{
}

IKController::Joint::Joint()
    :
    object(nullptr)
{
    for (size_t i = 0; i < 3; ++i)
    {
        allowTranslation[i] = false;
        minTranslation[i] = -std::numeric_limits<float>::infinity();
        maxTranslation[i] = std::numeric_limits<float>::infinity();
        allowRotation[i] = false;
        minRotation[i] = static_cast<float>(-GTE_C_PI);
        maxRotation[i] = static_cast<float>(GTE_C_PI);
    }
}

void IKController::Joint::UpdateWorldSRT()
{
    Spatial const* parent = object->GetParent();
    if (parent)
    {
        object->worldTransform = parent->worldTransform * object->localTransform;
    }
    else
    {
        object->worldTransform = object->localTransform;
    }
}

void IKController::Joint::UpdateWorldRT()
{
    Spatial const* parent = object->GetParent();
    auto const& olxfrm = object->localTransform;
    auto& owxfrm = object->worldTransform;

    if (parent)
    {
        auto const& pwxfrm = parent->worldTransform;
        owxfrm.SetRotation(pwxfrm.GetRotation() * olxfrm.GetRotation());
        owxfrm.SetTranslation(pwxfrm * olxfrm.GetTranslationW1());
    }
    else
    {
        owxfrm.SetRotation(olxfrm.GetRotation());
        owxfrm.SetTranslation(olxfrm.GetTranslation());
    }
}

Vector3<float> IKController::Joint::GetAxis(int axis)
{
    const Spatial* parent = object->GetParent();
    if (parent)
    {
        Vector4<float> U = parent->worldTransform.GetRotation().GetCol(axis);
        return HProject(U);
    }
    else
    {
        return Vector3<float>::Unit(axis);
    }
}

bool IKController::Joint::UpdateLocalT(int axis, std::vector<Goal> const& goals)
{
    Vector3<float> U = GetAxis(axis);
    float numer = 0.0f;
    float denom = 0.0f;
    float oldNorm = 0.0f;
    for (auto g : goalIndices)
    {
        auto const& goal = goals[g];
        Vector3<float> GmE = goal.GetTargetPosition() - goal.GetEffectorPosition();
        oldNorm += Dot(GmE, GmE);
        numer += goal.weight * Dot(U, GmE);
        denom += goal.weight;
    }

    if (denom == 0.0f)
    {
        return false;
    }

    // Desired distance to translate along axis(i).
    float t = numer / denom;

    // Clamp to range.
    Vector3<float> trn = object->localTransform.GetTranslation();
    float desired = trn[axis] + t;
    if (desired > minTranslation[axis])
    {
        if (desired < maxTranslation[axis])
        {
            trn[axis] = desired;
        }
        else
        {
            t = maxTranslation[axis] - trn[axis];
            trn[axis] = maxTranslation[axis];
        }
    }
    else
    {
        t = minTranslation[axis] - trn[axis];
        trn[axis] = minTranslation[axis];
    }

    // Test whether step should be taken.
    float newNorm = 0.0f;
    Vector3<float> step = t * U;
    for (auto g : goalIndices)
    {
        auto const& goal = goals[g];
        Vector3<float> newE = goal.GetEffectorPosition() + step;
        Vector3<float> diff = goal.GetTargetPosition() - newE;
        newNorm += Dot(diff, diff);
    }
    if (newNorm >= oldNorm)
    {
        // Translation does not get effector closer to goal.
        return false;
    }

    // Update the local translation.
    object->localTransform.SetTranslation(trn);
    return true;
}

bool IKController::Joint::UpdateLocalR(int axis, std::vector<Goal> const& goals)
{
    Vector3<float> U = GetAxis(axis);
    float numer = 0.0f;
    float denom = 0.0f;

    float oldNorm = 0.0f;
    for (auto g : goalIndices)
    {
        auto const& goal = goals[g];
        Vector3<float> EmP = goal.GetEffectorPosition() - object->worldTransform.GetTranslation();
        Vector3<float> GmP = goal.GetTargetPosition() - object->worldTransform.GetTranslation();
        Vector3<float> GmE = goal.GetTargetPosition() - goal.GetEffectorPosition();
        oldNorm += Dot(GmE, GmE);
        Vector3<float> UxEmP = Cross(U, EmP);
        Vector3<float> UxUxEmP = Cross(U, UxEmP);
        numer += goal.weight * Dot(GmP, UxEmP);
        denom -= goal.weight * Dot(GmP, UxUxEmP);
    }

    if (numer * numer + denom * denom == 0.0f)
    {
        return false;
    }

    // Desired angle to rotate about axis(i).
    float theta = std::atan2(numer, denom);

    // Factor local rotation into Euler angles.
    EulerAngles<float> euler =
        Rotation<4, float>(object->localTransform.GetRotation())(0, 1, 2);

    // Clamp to range.
    float desired = euler.angle[axis] + theta;
    if (desired > minRotation[axis])
    {
        if (desired < maxRotation[axis])
        {
            euler.angle[axis] = desired;
        }
        else
        {
            theta = maxRotation[axis] - euler.angle[axis];
            euler.angle[axis] = maxRotation[axis];
        }
    }
    else
    {
        theta = minRotation[axis] - euler.angle[axis];
        euler.angle[axis] = minRotation[axis];
    }

    // Test whether step should be taken.
    float newNorm = 0.0f;
    Matrix3x3<float> rotate = Rotation<3, float>(AxisAngle<3, float>(U, theta));
    for (auto g : goalIndices)
    {
        auto const& goal = goals[g];
        Vector3<float> EmP = goal.GetEffectorPosition() - object->worldTransform.GetTranslation();
        Vector3<float> newE = object->worldTransform.GetTranslation() + rotate * EmP;
        Vector3<float> GmE = goal.GetTargetPosition() - newE;
        newNorm += Dot(GmE, GmE);
    }

    if (newNorm >= oldNorm)
    {
        // Rotation does not get effector closer to goal.
        return false;
    }

    // Update the local rotation.
    rotate = Rotation<3, float>(euler);
    object->localTransform.SetRotation(rotate);
    return true;
}
