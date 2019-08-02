// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.1 (2019/04/19)

#pragma once

#include <Applications/GteWindow2.h>
#include <Imagics/GteCurveExtractorSquares.h>
#include <Imagics/GteCurveExtractorTriangles.h>
using namespace gte;

class ExtractLevelCurvesWindow : public Window2
{
public:
    ExtractLevelCurvesWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseClick(int button, int state, int x, int y, unsigned int modifiers) override;
    virtual bool OnMouseMotion(int button, int x, int y, unsigned int modifiers) override;

private:
    bool SetEnvironment();
    void ExtractLevelCurves(int x, int y);

    // The original image is 256x256 of int16_t with pixel values in
    // [0,1023); that is, only 10 bits are used per pixel.  The enlarged
    // image is 768x768 of R8G8B8A8 color, which is used for initializing
    // the background of the window.  The level curves are drawn on top
    // of the background.
    std::vector<int16_t> mOriginal;
    std::vector<uint32_t> mEnlarged;

    std::unique_ptr<CurveExtractorSquares<int16_t, double>> mExtractorSquares;
    std::unique_ptr<CurveExtractorTriangles<int16_t, double>> mExtractorTriangles;
    std::vector<std::array<double, 2>> mVertices;
    std::vector<CurveExtractor<int16_t, double>::Edge> mEdges;

    // When true, use mExtractorSquares.  When false, use mExtractorTriangles.
    bool mUseSquares;

    // For left-mouse-drag operations.
    bool mMouseDown;
};
