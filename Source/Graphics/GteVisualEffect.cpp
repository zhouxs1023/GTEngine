// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#include <GTEnginePCH.h>
#include <Graphics/GteVisualEffect.h>
using namespace gte;

VisualEffect::~VisualEffect()
{
}

VisualEffect::VisualEffect()
{
    mPVWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    SetPVWMatrix(Matrix4x4<float>::Identity());
}

VisualEffect::VisualEffect(std::shared_ptr<VisualProgram> const& program)
    :
    mProgram(program)
{
    mPVWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    SetPVWMatrix(Matrix4x4<float>::Identity());
}

void VisualEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    mPVWMatrixConstant = buffer;
    SetPVWMatrix(Matrix4x4<float>::Identity());
}
