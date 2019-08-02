// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.1 (2019/04/19)

#pragma once

#include <Applications/GteWindow2.h>
using namespace gte;

class GpuGaussianBlur2Window : public Window2
{
public:
    GpuGaussianBlur2Window(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateImages();
    bool CreateShaders();

    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mImage[2];
    std::shared_ptr<Texture2> mMaskTexture;
    std::shared_ptr<Texture2> mOffsetTexture;
    std::shared_ptr<ConstantBuffer> mWeightBuffer;
    std::shared_ptr<ComputeProgram> mGaussianBlurProgram;
    std::shared_ptr<ComputeProgram> mBoundaryDirichletProgram;
    std::shared_ptr<ComputeProgram> mBoundaryNeumannProgram;
    unsigned int mNumXThreads, mNumYThreads;
    unsigned int mNumXGroups, mNumYGroups;
};
