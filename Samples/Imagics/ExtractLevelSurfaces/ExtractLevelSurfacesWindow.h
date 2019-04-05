// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.0 (2019/03/11)

#pragma once

#include <GTEngine.h>
using namespace gte;

class ExtractLevelSurfacesWindow : public Window3
{
public:
    ExtractLevelSurfacesWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateMeshCubes();
    void CreateMeshTetrahedra();
    void UpdateConstants();

    std::shared_ptr<Node> mScene;
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mMeshCubes;
    std::shared_ptr<Visual> mMeshTetrahedra;
    std::shared_ptr<DirectionalLightEffect> mLightEffectCubes;
    std::shared_ptr<DirectionalLightEffect> mLightEffectTetrahedra;

    // An x-ray crystallography of a molecule.
    int mXBound, mYBound, mZBound;
    std::vector<uint8_t> mImage;

    // Shader parameters shared by both meshes.
    std::shared_ptr<Material> mMaterial;
    std::shared_ptr<Lighting> mLighting;
    Vector4<float> mLightWorldDirection;

    bool mUseCubes;
};
