// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class TriangulationECWindow : public Window2
{
public:
    TriangulationECWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void ClearAll();
    void UnindexedSimplePolygon();  // mExample = 0
    void IndexedSimplePolygon();    // mExample = 1
    void OneNestedPolygon();        // mExample = 2
    void TwoNestedPolygons();       // mExample = 3
    void TreeOfNestedPolygons();    // mExample = 4

    // The inputs (i0,i1,i2) are a permutation of (0,1,2).  The goal is to
    // trap algorithm errors due to order of inner polygons.
    // mExample = 5, 6, 7, 8, 9, 10
    void FourBoxesThreeNested(int i0, int i1, int i2);

    typedef BSRational<UIntegerAP32> Rational;
    typedef TriangulateEC<float, Rational> Triangulator;

    std::vector<Vector2<float>> mPositions;
    std::vector<int> mOuter, mInner0, mInner1, mInner2;
    std::shared_ptr<Triangulator::Tree> mTree;
    std::vector<Vector2<float>> mFillSeeds;
    std::vector<std::array<int, 3>> mTriangles;
    int mExample;
};
