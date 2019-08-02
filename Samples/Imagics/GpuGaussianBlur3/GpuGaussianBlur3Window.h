// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.1 (2019/04/19)

#pragma once

#include <Applications/GteWindow3.h>
using namespace gte;

class GpuGaussianBlur3Window : public Window3
{
public:
    GpuGaussianBlur3Window(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateImages();
    bool CreateShaders();

    inline int Map3Dto1D(int x, int y, int z) const
    {
        int u = x + (z % 8) * 128;
        int v = y + (z / 8) * 128;
        return u + 1024 * v;
    }

    inline void Map3Dto2D(int x, int y, int z, int& u, int& v) const
    {
        u = x + (z % 8) * 128;
        v = y + (z / 8) * 128;
    }

    inline int Map2Dto1D(int u, int v) const
    {
        return u + 1024 * v;
    }

    inline void Map2Dto3D(int u, int v, int& x, int& y, int& z) const
    {
        x = u % 128;
        y = v % 128;
        z = (u / 128) + (v / 128) * 8;
    }

    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mImage[2];
    std::shared_ptr<Texture2> mMaskTexture;
    std::shared_ptr<Texture2> mZNeighborTexture;
    std::shared_ptr<Texture2> mNeumannOffsetTexture;
    std::shared_ptr<ConstantBuffer> mWeightBuffer;
    std::shared_ptr<ComputeProgram> mGaussianBlurProgram;
    std::shared_ptr<ComputeProgram> mBoundaryDirichletProgram;
    std::shared_ptr<ComputeProgram> mBoundaryNeumannProgram;
    unsigned int mNumXThreads, mNumYThreads;
    unsigned int mNumXGroups, mNumYGroups;
};
