// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.0 (2019/03/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class BSplineCurveReductionWindow : public Window3
{
public:
    BSplineCurveReductionWindow(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    void CreateScene();

    std::shared_ptr<Visual> mOriginal;
    std::shared_ptr<Visual> mReduced;
};
