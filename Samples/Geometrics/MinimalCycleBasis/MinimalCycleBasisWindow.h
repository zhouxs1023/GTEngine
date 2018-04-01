// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2017/04/01)

#pragma once

#include <GTEngine.h>
using namespace gte;

class MinimalCycleBasisWindow : public Window2
{
public:
    MinimalCycleBasisWindow(Parameters& parameters);

    typedef BSNumber<UIntegerAP32> Rational;

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    void DrawTree(std::shared_ptr<MinimalCycleBasis<Rational>::Tree> const& tree);

    std::vector<std::array<Rational, 2>> mPositions;
    std::vector<std::array<int, 2>> mEdges;
    std::vector<std::array<float, 2>> mFPositions;
    std::vector<std::array<int, 2>> mSPositions;

    std::vector<std::shared_ptr<MinimalCycleBasis<Rational>::Tree>> mForest;
    bool mDrawRawData;
};
