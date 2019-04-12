// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.0 (2019/04/10)

#include "IntersectInfiniteCylindersWindow.h"

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

    Window::Parameters parameters(L"IntersectInfiniteCylindersWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<IntersectInfiniteCylindersWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<IntersectInfiniteCylindersWindow>(window);
    return 0;
}

IntersectInfiniteCylindersWindow::IntersectInfiniteCylindersWindow(Parameters& parameters)
    :
    Window3(parameters),
    mC0(4.0f),
    mW1(3.0f / 5.0f),
    mW2(4.0f / 5.0f),
    mRadius0(3.0f),
    mRadius1(2.0f),
    mHeight(100.0f),
    mAngle(std::atan2(mW1, mW2))
{
    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f});
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.01f, 0.001f,
        { 0.0f, -16.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
}

void IntersectInfiniteCylindersWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mCylinder0->culling == CullingMode::CULL_NEVER)
    {
        mEngine->Draw(mCylinder0);
    }

    if (mCylinder1->culling == CullingMode::CULL_NEVER)
    {
        mEngine->Draw(mCylinder1);
    }

    mEngine->Draw(mCurve0);
    mEngine->Draw(mCurve1);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectInfiniteCylindersWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
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

    case '0':
        if (mCylinder0->culling == CullingMode::CULL_NEVER)
        {
            mCylinder0->culling = CullingMode::CULL_ALWAYS;
        }
        else
        {
            mCylinder0->culling = CullingMode::CULL_NEVER;
        }
        return true;

    case '1':
        if (mCylinder1->culling == CullingMode::CULL_NEVER)
        {
            mCylinder1->culling = CullingMode::CULL_ALWAYS;
        }
        else
        {
            mCylinder1->culling = CullingMode::CULL_NEVER;
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void IntersectInfiniteCylindersWindow::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    // Create the canonical cylinder.
    mCylinder0 = mf.CreateCylinderOpen(32, 128, mRadius0, mHeight);
    mCylinder0->culling = CullingMode::CULL_NEVER;
    Vector4<float> red{ 0.5f, 0.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, red);
    mCylinder0->SetEffect(effect);
    mPVWMatrices.Subscribe(mCylinder0->worldTransform, effect->GetPVWMatrixConstant());

    // Create the other cylinder.
    mCylinder1 = mf.CreateCylinderOpen(32, 128, mRadius1, mHeight);
    mCylinder1->culling = CullingMode::CULL_NEVER;
    mCylinder1->localTransform.SetRotation(
        AxisAngle<4, float>(Vector4<float>::Unit(0), -mAngle));
    mCylinder1->localTransform.SetTranslation(mC0, 0.0f, 0.0f);
    Vector4<float> blue{ 0.0f, 0.0f, 0.5f, 1.0f };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, blue);
    mCylinder1->SetEffect(effect);
    mPVWMatrices.Subscribe(mCylinder1->worldTransform, effect->GetPVWMatrixConstant());

    // Create the intersection curve.
    unsigned int numVertices = 1024;
    float const minTheta = static_cast<float>(2.0 * GTE_C_PI / 3.0f);
    float const maxTheta = static_cast<float>(4.0 * GTE_C_PI / 3.0f);
    float multiplier = (maxTheta - minTheta) / static_cast<float>(numVertices - 1);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, numVertices - 1);
    Vector4<float> green{ 0.0f, 0.5f, 0.0f, 1.0f };

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto* vertices = vbuffer->Get<Vector3<float>>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        float theta = minTheta + multiplier * i;
        float cs = std::cos(theta);
        float sn = std::sin(theta);
        float tmp = mC0 + mRadius1 * cs;
        float discr = std::fabs(mRadius0 * mRadius0 - tmp * tmp);
        float t = (-mRadius1 * mW2 * sn - std::sqrt(discr)) / mW1;
        vertices[i][0] = mC0 + mRadius1 * cs;
        vertices[i][1] = +mRadius1 * sn * mW2 + t * mW1;
        vertices[i][2] = -mRadius1 * sn * mW1 + t * mW2;
    }
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, green);
    mCurve0 = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mCurve0->worldTransform, effect->GetPVWMatrixConstant());

    vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vertices = vbuffer->Get<Vector3<float>>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        float theta = minTheta + multiplier * i;
        float cs = std::cos(theta);
        float sn = std::sin(theta);
        float tmp = mC0 + mRadius1 * cs;
        float discr = std::fabs(mRadius0 * mRadius0 - tmp * tmp);
        float t = (-mRadius1 * mW2 * sn + std::sqrt(discr)) / mW1;
        vertices[i][0] = mC0 + mRadius1 * cs;
        vertices[i][1] = +mRadius1 * sn * mW2 + t * mW1;
        vertices[i][2] = -mRadius1 * sn * mW1 + t * mW2;
    }
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, green);
    mCurve1 = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mCurve1->worldTransform, effect->GetPVWMatrixConstant());

    mTrackball.Attach(mCylinder0);
    mTrackball.Attach(mCylinder1);
    mTrackball.Attach(mCurve0);
    mTrackball.Attach(mCurve1);
    mTrackball.Update();
    mPVWMatrices.Update();
}
