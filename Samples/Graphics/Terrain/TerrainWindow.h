// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.1 (2019/04/17)

#pragma once

#include <Applications/GteWindow3.h>
#include <LowLevel/GteTimer.h>
#include <Graphics/GteTerrain.h>
using namespace gte;

class TerrainWindow : public Window3
{
public:
    TerrainWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnKeyDown(int key, int x, int y) override;
    virtual bool OnKeyUp(int key, int x, int y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateTerrain();
    void CreateSkyDome();
    void UpdateScene();

    // Turret-based camera motion.
    class TerrainCameraRig : public CameraRig
    {
    public:
        void Initialize(std::shared_ptr<Camera> const& camera,
            float translationSpeed, float rotationSpeed,
            std::shared_ptr<Terrain> const& terrain, float heightAboveTerrain);

    private:
        virtual void MoveForward();
        virtual void MoveBackward();
        virtual void MoveUp();
        virtual void MoveDown();

        std::shared_ptr<Camera> mCamera;
        std::shared_ptr<Terrain> mTerrain;
        float mHeightAboveTerrain;
    };

    struct SkyDomeVertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    struct TerrainVertex
    {
        Vector3<float> position;
        Vector2<float> baseTCoord, detailTCoord;
    };

    // scene -> skyDome
    //       -> terrain -> terrainPage(0,0)
    //                  -> :
    //                  -> terrainPage(0,7)
    //                  -> terrainPage(1,0)
    //                  -> :
    //                  -> terrainPage(1,7)
    //                  -> :
    //                  -> terrainPage(7,7)
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mSkyDome;
    std::shared_ptr<Terrain> mTerrain;
    Culler mCuller;
    std::array<float, 4> mTextColor;
    TerrainCameraRig mTerrainCameraRig;

    // Support for clamping the frame rate.
    Timer mMotionTimer;
    double mLastUpdateTime;

};
