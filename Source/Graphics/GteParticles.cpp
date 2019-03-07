// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.0 (2019/02/05)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GteParticles.h>
#include <Mathematics/GteVector2.h>
using namespace gte;

Particles::~Particles()
{
}

Particles::Particles(std::vector<Vector4<float>> const& positionSize,
    float sizeAdjust, VertexFormat const& vformat)
    :
    mPositionSize(positionSize),
    mSizeAdjust(sizeAdjust),
    mNumActive(static_cast<unsigned int>(positionSize.size()))
{
    unsigned int texOffset = IsValid(vformat);
    if (texOffset == std::numeric_limits<unsigned int>::max())
    {
        // IsValid(...) will generate logging messages depending on the
        // specific failure condition.
        return;
    }

    unsigned int numParticles = mNumActive;
    unsigned int numVertices = 4 * numParticles;
    mVBuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    mVBuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    unsigned int vertexSize = vformat.GetVertexSize();
    int index = vformat.GetIndex(VA_TEXCOORD, 0);
    unsigned int offset = vformat.GetOffset(index);
    char* tcoords = mVBuffer->GetData() + offset;
    std::array<Vector2<float>, 4> commonTCD =
    {
        Vector2<float>{ 0.0f, 0.0f },
        Vector2<float>{ 1.0f, 0.0f },
        Vector2<float>{ 1.0f, 1.0f },
        Vector2<float>{ 0.0f, 1.0f }
    };
    for (unsigned int i = 0; i < numParticles; ++i)
    {
        for (unsigned int j = 0; j < 4; ++j)
        {
            Vector2<float>& tcoord = *reinterpret_cast<Vector2<float>*>(tcoords);
            tcoord = commonTCD[j];
            tcoords += vertexSize;
        }
    }

    mIBuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2 * numParticles, sizeof(unsigned int));
    auto* indices = mIBuffer->Get<unsigned int>();
    for (unsigned int i = 0; i < numParticles; ++i)
    {
        unsigned int iFI = 4 * i;
        unsigned int iFIp1 = iFI + 1;
        unsigned int iFIp2 = iFI + 2;
        unsigned int iFIp3 = iFI + 3;
        *indices++ = iFI;
        *indices++ = iFIp1;
        *indices++ = iFIp2;
        *indices++ = iFI;
        *indices++ = iFIp2;
        *indices++ = iFIp3;
    }

    UpdateModelBound();
}

void Particles::SetSizeAdjust(float sizeAdjust)
{
    if (sizeAdjust > 0.0f)
    {
        mSizeAdjust = sizeAdjust;
    }
    else
    {
        LogError("Invalid size-adjust parameter.");
        mSizeAdjust = 1.0f;
    }
}

void Particles::SetNumActive(unsigned int numActive)
{
    unsigned int numParticles = static_cast<unsigned int>(mPositionSize.size());
    if (0 <= numActive && numActive <= numParticles)
    {
        mNumActive = numActive;
    }
    else
    {
        mNumActive = numParticles;
    }

    mVBuffer->SetNumActiveElements(4 * mNumActive);
    mIBuffer->SetNumActiveElements(6 * mNumActive);
}

void Particles::GenerateParticles(std::shared_ptr<Camera> const& camera)
{
    // Get access to the positions.
    VertexFormat vformat = mVBuffer->GetFormat();
    unsigned int vertexSize = vformat.GetVertexSize();
    char* vertices = mVBuffer->GetData();

    // Get camera axis directions in model space of particles.
    Matrix4x4<float> inverse = worldTransform.GetHInverse();
    Vector4<float> UpR = inverse * (camera->GetUVector() + camera->GetRVector());
    Vector4<float> UmR = inverse * (camera->GetUVector() - camera->GetRVector());

    // Generate quadrilaterals as pairs of triangles.
    for (unsigned int i = 0; i < mNumActive; ++i)
    {
        Vector4<float> posSize = mPositionSize[i];
        Vector3<float> position{ posSize[0], posSize[1], posSize[2] };
        float trueSize = mSizeAdjust * posSize[3];
        Vector3<float> scaledUpR = HProject(trueSize * UpR);
        Vector3<float> scaledUmR = HProject(trueSize * UmR);

        Vector3<float>& pos0 = *reinterpret_cast<Vector3<float>*>(vertices);
        pos0 = position - scaledUpR;
        vertices += vertexSize;

        Vector3<float>& pos1 = *reinterpret_cast<Vector3<float>*>(vertices);
        pos1 = position - scaledUmR;
        vertices += vertexSize;

        Vector3<float>& pos2 = *reinterpret_cast<Vector3<float>*>(vertices);
        pos2 = position + scaledUpR;
        vertices += vertexSize;

        Vector3<float>& pos3 = *reinterpret_cast<Vector3<float>*>(vertices);
        pos3 = position + scaledUmR;
        vertices += vertexSize;
    }

    UpdateModelBound();
}

unsigned int Particles::IsValid(VertexFormat const& vformat) const
{
    // Validate the vertex position.
    int index = vformat.GetIndex(VA_POSITION, 0);
    if (index < 0)
    {
        LogError("Vertex format does not have VA_POSITION.");
        return std::numeric_limits<unsigned int>::max();
    }

    DFType posType = vformat.GetType(index);
    if (posType != DF_R32G32B32_FLOAT && posType != DF_R32G32B32A32_FLOAT)
    {
        LogError("Invalid position type.");
        return std::numeric_limits<unsigned int>::max();
    }

    unsigned int offset = vformat.GetOffset(index);
    if (offset != 0)
    {
        LogAssert(offset != 0, "Position offset must be 0.");
        return std::numeric_limits<unsigned int>::max();
    }

    // Validate the vertex texture coordinate that is used for drawing the
    // billboards.
    index = vformat.GetIndex(VA_TEXCOORD, 0);
    if (index < 0)
    {
        LogError("Vertex format does not have VA_TEXCOORD.");
        return std::numeric_limits<unsigned int>::max();
    }

    DFType texType = vformat.GetType(index);
    if (texType != DF_R32G32_FLOAT)
    {
        LogError("Invalid texture coordinate type.");
        return std::numeric_limits<unsigned int>::max();
    }

    unsigned int texOffset = (posType == DF_R32G32B32_FLOAT ?
        3 * sizeof(float) : 4 * sizeof(float));
    offset = vformat.GetOffset(index);
    if (offset != texOffset)
    {
        LogError("Texture coordinate must immediately follow position.");
        return std::numeric_limits<unsigned int>::max();
    }

    return texOffset;
}
