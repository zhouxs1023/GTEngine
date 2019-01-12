// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.18.0 (2018/10/28)

#include "NURBSSphereWindow.h"

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

    Window::Parameters parameters(L"NURBSSphereWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<NURBSSphereWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<NURBSSphereWindow>(window);
    return 0;
}

NURBSSphereWindow::NURBSSphereWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;
    mEngine->SetRasterizerState(mNoCullWireState);

    CreateScene();

    InitializeCamera(60.0f, GetAspectRatio(), 0.001f, 100.0f, 0.001f, 0.001f,
        { 4.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mTrackball.Update();
    mPVWMatrices.Update();
}

void NURBSSphereWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mCurrentVisual);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool NURBSSphereWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullSolidState == mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullSolidState);
        }
        return true;

    case '0':
        mCurrentVisual = mEighthSphereVisual;
        return true;
    case '1':
        mCurrentVisual = mHalfSphereVisual;
        return true;
    case '2':
        mCurrentVisual = mFullSphereVisual;
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

void NURBSSphereWindow::CreateScene()
{
    CreateEighthSphere();
    CreateHalfSphere();
    CreateFullSphere();
    mCurrentVisual = mEighthSphereVisual;
}

void NURBSSphereWindow::CreateEighthSphere()
{
    const int density = 32;
    Vector3<float> values[6];

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, density * density);
    Vector3<float>* vertices = vbuffer->Get<Vector3<float>>();
    memset(vbuffer->GetData(), 0, vbuffer->GetNumBytes());
    for (int iv = 0; iv <= density - 1; ++iv)
    {
        float v = (float)iv / (float)(density - 1);
        for (int iu = 0; iu + iv <= density - 1; ++iu)
        {
            float u = (float)iu / (float)(density - 1);
            mEighthSphere.Evaluate(u, v, 0, values);
            vertices[iu + density * iv] = values[0];
        }
    }

    std::vector<int> indices;
    for (int iv = 0; iv <= density - 2; ++iv)
    {
        // two triangles per square
        int iu, j0, j1, j2, j3;
        for (iu = 0; iu + iv <= density - 3; ++iu)
        {
            j0 = iu + density * iv;
            j1 = j0 + 1;
            j2 = j0 + density;
            j3 = j2 + 1;
            indices.push_back(j0);
            indices.push_back(j1);
            indices.push_back(j2);
            indices.push_back(j1);
            indices.push_back(j3);
            indices.push_back(j2);
        }

        // last triangle in row is singleton
        j0 = iu + density * iv;
        j1 = j0 + 1;
        j2 = j0 + density;
        indices.push_back(j0);
        indices.push_back(j1);
        indices.push_back(j2);
    }

    uint32_t numTriangles = (uint32_t)(indices.size() / 3);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(int));
    memcpy(ibuffer->GetData(), indices.data(), indices.size() * sizeof(int));

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    mEighthSphereVisual = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mEighthSphereVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mEighthSphereVisual);
}

void NURBSSphereWindow::CreateHalfSphere()
{
    const int density = 32;
    Vector3<float> values[6];

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mHalfSphereVisual = mf.CreateRectangle(density, density, 1.0f, 1.0f);
    auto vbuffer = mHalfSphereVisual->GetVertexBuffer();
    Vector3<float>* vertices = vbuffer->Get<Vector3<float>>();
    for (int iv = 0; iv < density; ++iv)
    {
        float v = (float)iv / (float)(density - 1);
        for (int iu = 0; iu < density; ++iu)
        {
            float u = (float)iu / (float)(density - 1);
            mHalfSphere.Evaluate(u, v, 0, values);
            vertices[iu + density * iv] = values[0];
        }
    }

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    mHalfSphereVisual->SetEffect(effect);
    mPVWMatrices.Subscribe(mHalfSphereVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mHalfSphereVisual);
}

void NURBSSphereWindow::CreateFullSphere()
{
    const int density = 32;
    Vector3<float> values[6];

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mFullSphereVisual = mf.CreateRectangle(density, density, 1.0f, 1.0f);
    auto vbuffer = mFullSphereVisual->GetVertexBuffer();
    Vector3<float>* vertices = vbuffer->Get<Vector3<float>>();
    for (int iv = 0; iv < density; ++iv)
    {
        float v = (float)iv / (float)(density - 1);
        for (int iu = 0; iu < density; ++iu)
        {
            float u = (float)iu / (float)(density - 1);
            mFullSphere.Evaluate(u, v, 0, values);
            vertices[iu + density * iv] = values[0];
        }
    }

    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 0.0f, 1.0f, 1.0f });
    mFullSphereVisual->SetEffect(effect);
    mPVWMatrices.Subscribe(mFullSphereVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mFullSphereVisual);
}
