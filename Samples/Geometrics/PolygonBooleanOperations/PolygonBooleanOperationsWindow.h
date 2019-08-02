// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.1 (2019/04/13)

#pragma once

#include <Applications/GteWindow2.h>
#include <Mathematics/GteBSPPolygon2.h>
using namespace gte;

class PolygonBooleanOperationsWindow : public Window2
{
public:
    PolygonBooleanOperationsWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    std::unique_ptr<BSPPolygon2<double>> ConstructInvertedEll();
    std::unique_ptr<BSPPolygon2<double>> ConstructPentagon();
    std::unique_ptr<BSPPolygon2<double>> ConstructSquare();
    std::unique_ptr<BSPPolygon2<double>> ConstructSShape();
    std::unique_ptr<BSPPolygon2<double>> ConstructPolyWithHoles();

    void DoBoolean();
    void DrawPolySolid(BSPPolygon2<double>& polygon, unsigned int color);

    double mEpsilon;
    BSPPolygon2<double> mIntersection, mUnion, mDiff01, mDiff10, mXor;
    std::unique_ptr<BSPPolygon2<double>> mPoly0;
    std::unique_ptr<BSPPolygon2<double>> mPoly1;
    BSPPolygon2<double>* mActive;
    int mChoice;
    double mSize;
};
