// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.0 (2019/03/22)

#pragma once

#include <GTEngine.h>
using namespace gte;

class GeodesicHeightFieldWindow : public Window3
{
public:
    GeodesicHeightFieldWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int x, int y, unsigned int modifiers) override;

private:
    bool SetEnvironment();
    void CreateScene();

    struct Vertex
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;
    };

    std::function<void(int, int)> mDrawCallback;

    std::shared_ptr<RasterizerState> mNoCullState, mNoCullWireState;
    std::shared_ptr<Visual> mMesh;
    std::shared_ptr<Texture2> mTexture;
    Vector4<float> mLightWorldDirection;
    Picker mPicker;

    std::unique_ptr<BSplineSurface<3, double>> mSurface;
    std::unique_ptr<BSplineGeodesic<double>> mGeodesic;

    int mSelected;
    std::array<int, 2> mXIntr, mYIntr;
    std::array<GVector<double>, 2> mPoint;
    std::vector<GVector<double>> mPath;
    int mPathQuantity;
    double mDistance, mCurvature;

    std::array<float, 4> mTextColor;
};
