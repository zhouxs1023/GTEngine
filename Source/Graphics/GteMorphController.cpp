// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.0 (2019/02/02)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GteMorphController.h>
#include <Graphics/GteVisual.h>
using namespace gte;

MorphController::~MorphController()
{
}

MorphController::MorphController(size_t numTargets, size_t numVertices, size_t numTimes,
    Updater const& postUpdate)
    :
    mNumTargets(numTargets),
    mNumVertices(numVertices),
    mNumTimes(numTimes),
    mVertices(numTargets * numVertices),
    mTimes(numTimes),
    mWeights(numTimes * mNumTargets),
    mLastIndex(0),
    mPostUpdate(postUpdate)
{
    LogAssert(numTargets > 0 && numVertices > 0 && numTimes > 0, "Invalid input.");
}

void MorphController::SetVertices(size_t target, std::vector<Vector3<float>> const& vertices)
{
    if (target < mNumTargets && vertices.size() >= mNumVertices)
    {
        std::copy(vertices.begin(), vertices.end(), mVertices.begin() + target * mNumVertices);
        return;
    }
    LogError("Invalid target or input vertices array is too small.");
}

void MorphController::SetTimes(std::vector<float> const& times)
{
    if (times.size() >= mNumTimes)
    {
        std::copy(times.begin(), times.end(), mTimes.begin());
        return;
    }
    LogError("Input times array is too small.");
}

void MorphController::SetWeights(size_t key, std::vector<float> const& weights)
{
    if (key < mNumTimes && weights.size() >= mNumTargets)
    {
        std::copy(weights.begin(), weights.end(), mWeights.begin() + key * mNumTargets);
        return;
    }
    LogError("Invalid key or input weights array is too small.");
}

void MorphController::GetVertices(size_t target, std::vector<Vector3<float>>& vertices)
{
    if (target < mNumTargets)
    {
        vertices.resize(mNumVertices);
        auto begin = mVertices.begin() + target * mNumVertices;
        auto end = begin + mNumVertices;
        std::copy(begin, end, vertices.begin());
    }
    LogError("Invalid target.");
}

void MorphController::GetTimes(std::vector<float>& times)
{
    times.resize(mNumTimes);
    std::copy(mTimes.begin(), mTimes.end(), times.begin());
}

void MorphController::GetWeights(size_t key, std::vector<float>& weights)
{
    if (key < mNumTimes)
    {
        weights.resize(mNumTargets);
        auto begin = mWeights.begin() + key * mNumTargets;
        auto end = begin + mNumTargets;
        std::copy(begin, end, weights.begin());
    }
    LogError("Invalid key.");
}

bool MorphController::Update(double applicationTime)
{
    // The key interpolation uses linear interpolation.  To get higher-order
    // interpolation, you need to provide a more sophisticated key (Bezier
    // cubic or TCB spline, for example).

    if (!Controller::Update(applicationTime))
    {
        return false;
    }

    // Get access to the vertex buffer to store the blended targets.
    Visual* visual = reinterpret_cast<Visual*>(mObject);
    auto vbuffer = visual->GetVertexBuffer();
    VertexFormat vformat = vbuffer->GetFormat();

    // Initialize the 3-tuple positions (x,y,z) to zero for accumulation.
    unsigned int numVertices = vbuffer->GetNumElements();
    char* combination = vbuffer->GetData();
    size_t vertexSize = static_cast<size_t>(vformat.GetVertexSize());
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        Vector3<float>& vertex = *reinterpret_cast<Vector3<float>*>(combination);
        vertex = { 0.0f, 0.0f, 0.0f };
        combination += vertexSize;
    }

    // Look up the bounding keys.
    float ctrlTime = static_cast<float>(GetControlTime(applicationTime));
    float normTime;
    size_t key0, key1;
    GetKeyInfo(ctrlTime, normTime, key0, key1);
    float oneMinusNormTime = 1.0f - normTime;

    // Compute the weighted combination.
    float const* weights0 = &mWeights[key0 * mNumTargets];
    float const* weights1 = &mWeights[key1 * mNumTargets];
    Vector3<float> const* vertices = mVertices.data();
    float wsum0 = 0.0f, wsum1 = 0.0f;
    for (size_t n = 0; n < mNumTargets; ++n)
    {
        float w = oneMinusNormTime * weights0[n] + normTime * weights1[n];
        wsum0 += weights0[n];
        wsum1 += weights1[n];
        combination = vbuffer->GetData();
        for (size_t m = 0; m < mNumVertices; ++m)
        {
            Vector3<float>& position = *reinterpret_cast<Vector3<float>*>(combination);
            position += w * (*vertices++);  // += w * mVertices[m + mNumTargets * n];
            combination += vertexSize;
        }
    }

    visual->UpdateModelBound();
    visual->UpdateModelNormals();
    mPostUpdate(vbuffer);
    return true;
}

void MorphController::SetObject(ControlledObject* object)
{
    // Verify that the object satisfies the preconditions that allow a
    // MorphController to be attached to it.
    Visual* visual = dynamic_cast<Visual*>(object);
    if (!visual)
    {
        LogError("Object is not of type Visual.");
        return;
    }

    auto vbuffer = visual->GetVertexBuffer();
    if (vbuffer->GetNumElements() != mNumVertices)
    {
        LogError("Mismatch in number of vertices.");
        return;
    }

    // The vertex buffer for a Visual controlled by a MorphController must
    // have 3-tuple or 4-tuple float-valued position that occurs at the
    // beginning (offset 0) of the vertex structure.
    VertexFormat vformat = vbuffer->GetFormat();
    int index = vformat.GetIndex(VA_POSITION, 0);
    if (index < 0)
    {
        LogError("Vertex format does not have VA_POSITION.");
        return;
    }

    DFType type = vformat.GetType(index);
    if (type != DF_R32G32B32_FLOAT && type != DF_R32G32B32A32_FLOAT)
    {
        LogError("Invalid position type.");
        return;
    }

    unsigned int offset = vformat.GetOffset(index);
    if (offset != 0)
    {
        LogError("Position offset must be 0.");
        return;
    }

    Controller::SetObject(object);
}

void MorphController::GetKeyInfo(float ctrlTime, float& normTime, size_t& key0, size_t& key1)
{
    if (ctrlTime <= mTimes[0])
    {
        normTime = 0.0f;
        mLastIndex = 0;
        key0 = 0;
        key1 = 0;
        return;
    }

    if (ctrlTime >= mTimes[mNumTimes - 1])
    {
        normTime = 0.0f;
        mLastIndex = mNumTimes - 1;
        key0 = mLastIndex;
        key1 = mLastIndex;
        return;
    }

    size_t nextIndex;
    if (ctrlTime > mTimes[mLastIndex])
    {
        nextIndex = mLastIndex + 1;
        while (ctrlTime >= mTimes[nextIndex])
        {
            mLastIndex = nextIndex;
            ++nextIndex;
        }

        key0 = mLastIndex;
        key1 = nextIndex;
        normTime = (ctrlTime - mTimes[key0]) / (mTimes[key1] - mTimes[key0]);
    }
    else if (ctrlTime < mTimes[mLastIndex])
    {
        nextIndex = mLastIndex - 1;
        while (ctrlTime <= mTimes[nextIndex])
        {
            mLastIndex = nextIndex;
            --nextIndex;
        }

        key0 = nextIndex;
        key1 = mLastIndex;
        normTime = (ctrlTime - mTimes[key0]) / (mTimes[key1] - mTimes[key0]);
    }
    else
    {
        normTime = 0.0f;
        key0 = mLastIndex;
        key1 = mLastIndex;
    }
}
