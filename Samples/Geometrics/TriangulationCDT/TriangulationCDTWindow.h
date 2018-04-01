// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class TriangulationCDTWindow : public Window2
{
public:
    TriangulationCDTWindow(Parameters& parameters);

    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void DrawTriangulation();

    void UnindexedSimplePolygon();  // key = '0'
    void IndexedSimplePolygon();    // key = '1'
    void OneNestedPolygon();        // key = '2'
    void TwoNestedPolygons();       // key = '3'
    void TreeOfNestedPolygons();    // key = '4'

    typedef BSNumber<UIntegerAP32> Rational;
    typedef TriangulateCDT<float, Rational> Triangulator;
    typedef PlanarMesh<float, Rational, Rational> PlanarMesher;
    std::vector<Vector2<float>> mPoints;
    std::unique_ptr<Triangulator> mTriangulator;
    std::unique_ptr<PlanarMesher> mPMesher;
};
