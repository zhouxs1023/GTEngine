// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "LightsWindow.h"

int main(int, char const*[])
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    Window::Parameters parameters(L"LightsWindow", 0, 0, 1024, 768);
    auto window = TheWindowSystem.Create<LightsWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<LightsWindow>(window);
    return 0;
}

LightsWindow::LightsWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    mEngine->SetClearColor({ 0.0f, 0.25f, 0.75f, 1.0f });
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    CreateScene();

    Vector3<float> pos{ 16.0f, 0.0f, 8.0f };
    Vector3<float> dir = -pos;
    Normalize(dir);
    Vector3<float> up{ dir[2], 0.0f, -dir[0] };
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { pos[0], pos[1], pos[2] }, { dir[0], dir[1], dir[2] }, { up[0], up[1], up[2] });
    mPVWMatrices.Update();
}

void LightsWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mPlane[0]);
    mEngine->Draw(mPlane[1]);
    mEngine->Draw(mSphere[0]);
    mEngine->Draw(mSphere[1]);
    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    mEngine->Draw(8, 16, textColor, mCaption[mType]);
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool LightsWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':   // toggle wireframe
    case 'W':
        if (mEngine->GetRasterizerState() == mWireState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;

    case 'd':   // use directional lights
    case 'D':
        UseLightType(LDIR);
        return true;

    case 'p':   // use point lights
    case 'P':
        UseLightType(LPNT);
        return true;

    case 's':   // use spot lights
    case 'S':
        UseLightType(LSPT);
        return true;

    case 'i':   // decrease light intensity
        for (int lt = 0; lt < LNUM; ++lt)
        {
            for (int gt = 0; gt < GNUM; ++gt)
            {
                for (int st = 0; st < SNUM; ++st)
                {
                    auto effect = mEffect[lt][gt][st];
                    auto lighting = effect->GetLighting();
                    lighting->attenuation[3] -= 0.125f;
                    lighting->attenuation[3] = std::max(lighting->attenuation[3], 0.0f);
                    effect->UpdateLightingConstant();
                }
            }
        }
        return true;

    case 'I':   // increase light intensity
        for (int lt = 0; lt < LNUM; ++lt)
        {
            for (int gt = 0; gt < GNUM; ++gt)
            {
                for (int st = 0; st < SNUM; ++st)
                {
                    auto effect = mEffect[lt][gt][st];
                    auto lighting = effect->GetLighting();
                    lighting->attenuation[3] += 0.125f;
                    lighting->attenuation[3] = std::min(lighting->attenuation[3], 1.0f);
                    effect->UpdateLightingConstant();
                }
            }
        }
        return true;

    case 'a':   // decrease spot angle
        for (int gt = 0; gt < GNUM; ++gt)
        {
            for (int st = 0; st < SNUM; ++st)
            {
                auto effect = mEffect[LSPT][gt][st];
                auto lighting = effect->GetLighting();
                lighting->spotCutoff[0] -= 0.1f;
                lighting->spotCutoff[0] = std::max(lighting->spotCutoff[0], 0.0f);
                lighting->spotCutoff[1] = cos(lighting->spotCutoff[0]);
                lighting->spotCutoff[2] = sin(lighting->spotCutoff[0]);
                effect->UpdateLightingConstant();
            }
        }
        return true;

    case 'A':   // increase spot angle
        for (int gt = 0; gt < GNUM; ++gt)
        {
            for (int st = 0; st < SNUM; ++st)
            {
                auto effect = mEffect[LSPT][gt][st];
                auto lighting = effect->GetLighting();
                lighting->spotCutoff[0] -= 0.1f;
                lighting->spotCutoff[0] = std::min(lighting->spotCutoff[0], (float)GTE_C_HALF_PI);
                lighting->spotCutoff[1] = cos(lighting->spotCutoff[0]);
                lighting->spotCutoff[2] = sin(lighting->spotCutoff[0]);
                effect->UpdateLightingConstant();
            }
        }
        return true;

    case 'e':   // decrease spot exponent
        for (int gt = 0; gt < GNUM; ++gt)
        {
            for (int st = 0; st < SNUM; ++st)
            {
                auto effect = mEffect[LSPT][gt][st];
                auto lighting = effect->GetLighting();
                lighting->spotCutoff[3] *= 0.5f;
                effect->UpdateLightingConstant();
            }
        }
        return true;

    case 'E':   // increase spot exponent
        for (int gt = 0; gt < GNUM; ++gt)
        {
            for (int st = 0; st < SNUM; ++st)
            {
                auto effect = mEffect[LSPT][gt][st];
                auto lighting = effect->GetLighting();
                lighting->spotCutoff[3] *= 2.0f;
                effect->UpdateLightingConstant();
            }
        }
        return true;
    }
    return Window::OnCharPress(key, x, y);
}

void LightsWindow::CreateScene()
{
    // Copper color for the planes.
    Vector4<float> planeAmbient{ 0.2295f, 0.08825f, 0.0275f, 1.0f };
    Vector4<float> planeDiffuse{ 0.5508f, 0.2118f, 0.066f, 1.0f };
    Vector4<float> planeSpecular{ 0.580594f, 0.223257f, 0.0695701f, 51.2f };

    // Gold color for the spheres.
    Vector4<float> sphereAmbient{ 0.24725f, 0.2245f, 0.0645f, 1.0f };
    Vector4<float> sphereDiffuse{ 0.34615f, 0.3143f, 0.0903f, 1.0f };
    Vector4<float> sphereSpecular{ 0.797357f, 0.723991f, 0.208006f, 83.2f };

    // Various parameters shared by the lighting constants.  The geometric
    // parameters are dynamic, modified by UpdateConstants() whenever the
    // camera or scene moves.  These include camera model position, light
    // model position, light model direction, and model-to-world matrix.
    // The typecasts on cos/sin are to avoid an incorrect g++ warning with
    // Fedora 21 Linux.
    Vector4<float> darkGray{ 0.1f, 0.1f, 0.1f, 1.0f };
    Vector4<float> lightGray{ 0.75f, 0.75f, 0.75f, 1.0f };
    float angle = 0.125f*(float)GTE_C_PI;
    Vector4<float> lightSpotCutoff{ angle, (float)cos(angle), (float)sin(angle), 1.0f };

    mLightWorldPosition[SVTX] = { 4.0f, 4.0f - 8.0f, 8.0f, 1.0f };
    mLightWorldPosition[SPXL] = { 4.0f, 4.0f + 8.0f, 8.0f, 1.0f };
    mLightWorldDirection = { -1.0f, -1.0f, -1.0f, 0.0f };
    Normalize(mLightWorldDirection);

    std::shared_ptr<Material> material[LNUM][GNUM];
    std::shared_ptr<Lighting> lighting[LNUM][GNUM];
    std::shared_ptr<LightCameraGeometry> geometry[LNUM][GNUM];
    for (int lt = 0; lt < LNUM; ++lt)
    {
        for (int gt = 0; gt < GNUM; ++gt)
        {
            material[lt][gt] = std::make_shared<Material>();
            lighting[lt][gt] = std::make_shared<Lighting>();
            geometry[lt][gt] = std::make_shared<LightCameraGeometry>();
        }
    }

    // Initialize the directional lighting constants.
    material[LDIR][GPLN]->ambient = planeAmbient;
    material[LDIR][GPLN]->diffuse = planeDiffuse;
    material[LDIR][GPLN]->specular = planeSpecular;
    lighting[LDIR][GPLN]->ambient = lightGray;
    material[LDIR][GSPH]->ambient = sphereAmbient;
    material[LDIR][GSPH]->diffuse = sphereDiffuse;
    material[LDIR][GSPH]->specular = sphereSpecular;
    lighting[LDIR][GSPH]->ambient = lightGray;

    // Initialize the point lighting constants.
    material[LPNT][GPLN]->ambient = planeAmbient;
    material[LPNT][GPLN]->diffuse = planeDiffuse;
    material[LPNT][GPLN]->specular = planeSpecular;
    lighting[LPNT][GPLN]->ambient = darkGray;
    material[LPNT][GSPH]->ambient = sphereAmbient;
    material[LPNT][GSPH]->diffuse = sphereDiffuse;
    material[LPNT][GSPH]->specular = sphereSpecular;
    lighting[LPNT][GSPH]->ambient = darkGray;

    // Initialize the spot lighting constants.
    material[LSPT][GPLN]->ambient = planeAmbient;
    material[LSPT][GPLN]->diffuse = planeDiffuse;
    material[LSPT][GPLN]->specular = planeSpecular;
    lighting[LSPT][GPLN]->ambient = darkGray;
    lighting[LSPT][GPLN]->spotCutoff = lightSpotCutoff;
    material[LSPT][GSPH]->ambient = sphereAmbient;
    material[LSPT][GSPH]->diffuse = sphereDiffuse;
    material[LSPT][GSPH]->specular = sphereSpecular;
    lighting[LSPT][GSPH]->ambient = darkGray;
    lighting[LSPT][GSPH]->spotCutoff = lightSpotCutoff;

    // Create the effects.
    for (int gt = 0; gt < GNUM; ++gt)
    {
        for (int st = 0; st < SNUM; ++st)
        {
            mEffect[LDIR][gt][st] = std::make_shared<DirectionalLightEffect>(
                mProgramFactory, mUpdater, st,
                material[LDIR][gt], lighting[LDIR][gt], geometry[LDIR][gt]);

            mEffect[LPNT][gt][st] = std::make_shared<PointLightEffect>(
                mProgramFactory, mUpdater, st,
                material[LPNT][gt], lighting[LPNT][gt], geometry[LPNT][gt]);

            mEffect[LSPT][gt][st] = std::make_shared<SpotLightEffect>(
                mProgramFactory, mUpdater, st,
                material[LSPT][gt], lighting[LSPT][gt], geometry[LSPT][gt]);
        }
    }

    // Create the planes and spheres.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mPlane[SVTX] = mf.CreateRectangle(128, 128, 8.0f, 8.0f);
    mPlane[SVTX]->localTransform.SetTranslation(0.0f, -8.0f, 0.0f);
    mTrackball.Attach(mPlane[SVTX]);

    mPlane[SPXL] = mf.CreateRectangle(128, 128, 8.0f, 8.0f);
    mPlane[SPXL]->localTransform.SetTranslation(0.0f, +8.0f, 0.0f);
    mTrackball.Attach(mPlane[SPXL]);

    mSphere[SVTX] = mf.CreateSphere(64, 64, 2.0f);
    mSphere[SVTX]->localTransform.SetTranslation(0.0f, -8.0f, 2.0f);
    mTrackball.Attach(mSphere[SVTX]);

    mSphere[SPXL] = mf.CreateSphere(64, 64, 2.0f);
    mSphere[SPXL]->localTransform.SetTranslation(0.0f, +8.0f, 2.0f);
    mTrackball.Attach(mSphere[SPXL]);

    mTrackball.Update();

    mCaption[LDIR] = "Directional Light (left per vertex, right per pixel)";
    mCaption[LPNT] = "Point Light (left per vertex, right per pixel)";
    mCaption[LSPT] = "Spot Light (left per vertex, right per pixel)";

    UseLightType(LDIR);
}

void LightsWindow::UseLightType(int type)
{
    mPVWMatrices.Unsubscribe(mPlane[SVTX]->worldTransform);
    mPlane[SVTX]->SetEffect(mEffect[type][GPLN][SVTX]);
    mPVWMatrices.Subscribe(mPlane[SVTX]->worldTransform, mEffect[type][GPLN][SVTX]->GetPVWMatrixConstant());

    mPVWMatrices.Unsubscribe(mPlane[SPXL]->worldTransform);
    mPlane[SPXL]->SetEffect(mEffect[type][GPLN][SPXL]);
    mPVWMatrices.Subscribe(mPlane[SPXL]->worldTransform, mEffect[type][GPLN][SPXL]->GetPVWMatrixConstant());

    mPVWMatrices.Unsubscribe(mSphere[SVTX]->worldTransform);
    mSphere[SVTX]->SetEffect(mEffect[type][GSPH][SVTX]);
    mPVWMatrices.Subscribe(mSphere[SVTX]->worldTransform, mEffect[type][GSPH][SVTX]->GetPVWMatrixConstant());

    mPVWMatrices.Unsubscribe(mSphere[SPXL]->worldTransform);
    mSphere[SPXL]->SetEffect(mEffect[type][GSPH][SPXL]);
    mPVWMatrices.Subscribe(mSphere[SPXL]->worldTransform, mEffect[type][GSPH][SPXL]->GetPVWMatrixConstant());

    mType = type;

    mPVWMatrices.Update();
}

void LightsWindow::UpdateConstants()
{
    // The pvw-matrices are updated automatically whenever the camera moves
    // or the trackball is rotated, which happens before this call.  Here we
    // need to update the camera model position, light model position, and
    // light model direction.

    // Compute the model-to-world transforms for the planes and spheres.
    Matrix4x4<float> wMatrix[GNUM][SNUM];
    Matrix4x4<float> rotate = mTrackball.GetOrientation();
#if defined(GTE_USE_MAT_VEC)
    wMatrix[GPLN][SVTX] = rotate * mPlane[SVTX]->worldTransform;
    wMatrix[GPLN][SPXL] = rotate * mPlane[SPXL]->worldTransform;
    wMatrix[GSPH][SVTX] = rotate * mSphere[SVTX]->worldTransform;
    wMatrix[GSPH][SPXL] = rotate * mSphere[SPXL]->worldTransform;
#else
    wMatrix[GPLN][SVTX] = mPlane[SVTX]->worldTransform * rotate;
    wMatrix[GPLN][SPXL] = mPlane[SPXL]->worldTransform * rotate;
    wMatrix[GSPH][SVTX] = mSphere[SVTX]->worldTransform * rotate;
    wMatrix[GSPH][SPXL] = mSphere[SPXL]->worldTransform * rotate;
#endif

    // Compute the world-to-model transforms for the planes and spheres.
    Matrix4x4<float> invWMatrix[GNUM][SNUM];
    for (int gt = 0; gt < GNUM; ++gt)
    {
        for (int st = 0; st < SNUM; ++st)
        {
            invWMatrix[gt][st] = Inverse(wMatrix[gt][st]);
        }
    }

    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    for (int lt = 0; lt < LNUM; ++lt)
    {
        for (int gt = 0; gt < GNUM; ++gt)
        {
            for (int st = 0; st < SNUM; ++st)
            {
                auto effect = mEffect[lt][gt][st];
                auto lighting = mEffect[lt][gt][st]->GetLighting();
                auto geometry = mEffect[lt][gt][st]->GetGeometry();
                auto const& invwmat = invWMatrix[gt][st];

#if defined(GTE_USE_MAT_VEC)
                geometry->lightModelPosition = invwmat * mLightWorldPosition[st];
                geometry->lightModelDirection = invwmat * mLightWorldDirection;
                geometry->cameraModelPosition = invwmat * cameraWorldPosition;
#else
                geometry->lightModelPosition = mLightWorldPosition[st] * invwmat;
                geometry->lightModelDirection = mLightWorldDirection * invwmat;
                geometry->cameraModelPosition = cameraWorldPosition * invwmat;
#endif
                effect->UpdateGeometryConstant();
            }
        }
    }
}
