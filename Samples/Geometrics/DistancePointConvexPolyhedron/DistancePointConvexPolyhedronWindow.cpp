// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.5.0 (2016/11/25)

#include "DistancePointConvexPolyhedronWindow.h"

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

    Window::Parameters parameters(L"DistancePointConvexPolyhedronWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<DistancePointConvexPolyhedronWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<DistancePointConvexPolyhedronWindow>(window);
    return 0;
}

DistancePointConvexPolyhedronWindow::DistancePointConvexPolyhedronWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;
    mEngine->SetBlendState(mBlendState);

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 5000.0f, 0.01f, 0.01f,
        { 0.0f, 0.0f, -24.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    DoQuery();
    mPVWMatrices.Update();
}

void DistancePointConvexPolyhedronWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mPointMesh);
    mEngine->Draw(mPolyhedronMesh);
    mEngine->Draw(mSegment);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool DistancePointConvexPolyhedronWindow::OnCharPress(unsigned char key, int x, int y)
{
    float const delta = 0.1f;
    Quaternion<float> q, incr;

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

    case ' ':
        DoQuery();
        return true;

    case 'x':  // decrement x-center of box
        Translate(0, -delta);
        return true;

    case 'X':  // increment x-center of box
        Translate(0, +delta);
        return true;

    case 'y':  // decrement y-center of box
        Translate(1, -delta);
        return true;

    case 'Y':  // increment y-center of box
        Translate(1, +delta);
        return true;

    case 'z':  // decrement z-center of box
        Translate(2, -delta);
        return true;

    case 'Z':  // increment z-center of box
        Translate(2, +delta);
        return true;

    case 'p':  // rotate about axis[0]
        Rotate(0, -delta);
        return true;

    case 'P':  // rotate about axis[0]
        Rotate(0, +delta);
        return true;

    case 'r':  // rotate about axis[1]
        Rotate(1, -delta);
        return true;

    case 'R':  // rotate about axis[1]
        Rotate(1, +delta);
        return true;

    case 'h':  // rotate about axis[2]
        Rotate(2, -delta);
        return true;

    case 'H':  // rotate about axis[2]
        Rotate(2, +delta);
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

void DistancePointConvexPolyhedronWindow::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mPoint = { 1.0f, 1.0f, 1.0f };
    mPointMesh = mf.CreateSphere(8, 8, 0.0625f);
    mPointMesh->localTransform.SetTranslation(mPoint);
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.0f, 0.5f, 0.0f, 0.5f }));
    mPointMesh->SetEffect(effect);
    mPVWMatrices.Subscribe(mPointMesh->worldTransform, effect->GetPVWMatrixConstant());

    mRedEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.5f, 0.0f, 0.0f, 0.5f }));

    mBlueEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.0f, 0.0f, 0.5f, 0.5f }));

    mPolyhedronMesh = mf.CreateIcosahedron();
    mPolyhedronMesh->SetEffect(mBlueEffect);
    mPVWMatrices.Subscribe(mPolyhedronMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());

    auto vbuffer = mPolyhedronMesh->GetVertexBuffer();
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    mPolyhedron.vertices.resize(vbuffer->GetNumElements());
    memcpy(mPolyhedron.vertices.data(), vbuffer->GetData(), vbuffer->GetNumBytes());

    auto ibuffer = mPolyhedronMesh->GetIndexBuffer();
    mPolyhedron.indices.resize(ibuffer->GetNumElements());
    memcpy(mPolyhedron.indices.data(), ibuffer->GetData(), ibuffer->GetNumBytes());

    mPolyhedronCenter = { 0.0f, 0.0f, 0.0f };
    mPolyhedron.GeneratePlanes();
    mPolyhedron.GenerateAlignedBox();

    vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.0f, 0.0f, 0.0f, 1.0f }));
    mSegment = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mSegment->worldTransform, effect->GetPVWMatrixConstant());

    mTrackball.Attach(mPointMesh);
    mTrackball.Attach(mPolyhedronMesh);
    mTrackball.Attach(mSegment);
    mTrackball.Update();
}

void DistancePointConvexPolyhedronWindow::Translate(int direction, float delta)
{
    for (auto& vertex : mPolyhedron.vertices)
    {
        vertex[direction] += delta;
    }
    mPolyhedronCenter[direction] += delta;
    mPolyhedron.GeneratePlanes();
    mPolyhedron.GenerateAlignedBox();

    auto vbuffer = mPolyhedronMesh->GetVertexBuffer();
    memcpy(vbuffer->GetData(), mPolyhedron.vertices.data(), vbuffer->GetNumBytes());
    mEngine->Update(vbuffer);

    DoQuery();
    mPVWMatrices.Update();
}

void DistancePointConvexPolyhedronWindow::Rotate(int direction, float delta)
{
    Matrix3x3<float> rotate = Rotation<3, float>(
        AxisAngle<3, float>(Vector3<float>::Unit(direction), delta));
    for (auto& vertex : mPolyhedron.vertices)
    {
        vertex = mPolyhedronCenter + rotate * (vertex - mPolyhedronCenter);
    }
    mPolyhedron.GeneratePlanes();
    mPolyhedron.GenerateAlignedBox();

    auto vbuffer = mPolyhedronMesh->GetVertexBuffer();
    memcpy(vbuffer->GetData(), mPolyhedron.vertices.data(), vbuffer->GetNumBytes());
    mEngine->Update(vbuffer);

    DoQuery();
    mPVWMatrices.Update();
}

void DistancePointConvexPolyhedronWindow::DoQuery()
{
    mPVWMatrices.Unsubscribe(mPolyhedronMesh->worldTransform);

    auto result = mQuery(mPoint, mPolyhedron);
    float const epsilon = 1e-04f;
    if (result.distance > epsilon)
    {
        mPolyhedronMesh->SetEffect(mRedEffect);
        mPVWMatrices.Subscribe(mPolyhedronMesh->worldTransform, mRedEffect->GetPVWMatrixConstant());
    }
    else
    {
        mPolyhedronMesh->SetEffect(mBlueEffect);
        mPVWMatrices.Subscribe(mPolyhedronMesh->worldTransform, mBlueEffect->GetPVWMatrixConstant());
    }

    Vector3<float>* vertices = mSegment->GetVertexBuffer()->Get<Vector3<float>>();
    vertices[0] = result.closestPoint[0];
    vertices[1] = result.closestPoint[1];
    mEngine->Update(mSegment->GetVertexBuffer());
}
