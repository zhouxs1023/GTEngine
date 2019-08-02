// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.18.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow3.h>
using namespace gte;

class FitConeWindow : public Window3
{
public:
    FitConeWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void CreatePoints(std::vector<Vector3<double>> const& X);

    void CreateGNCone(std::vector<Vector3<double>> const& X,
        Vector3<double>& coneVertex, Vector3<double>& coneAxis, double& coneAngle);

    void CreateLMCone(std::vector<Vector3<double>> const& X,
        Vector3<double>& coneVertex, Vector3<double>& coneAxis, double& coneAngle);

    std::shared_ptr<Visual> CreateConeMesh(std::vector<Vector3<double>> const& X,
        Vector3<double> const& coneVertex, Vector3<double> const& coneAxis,
        double coneAngle, Vector4<float> const& color);

    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mPoints;
    std::shared_ptr<Visual> mGNCone;
    std::shared_ptr<Visual> mLMCone;
    std::array<float, 4> mTextColor;
    Vector3<float> mCenter;
};
