// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.21.0 (2019/01/21)

#include "MovingSphereTriangleWindow.h"

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

    Window::Parameters parameters(L"MovingSphereTriangleWindow", 0, 0, 768, 768);
    auto window = TheWindowSystem.Create<MovingSphereTriangleWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<MovingSphereTriangleWindow>(window);
    return 0;
}

MovingSphereTriangleWindow::MovingSphereTriangleWindow(Parameters& parameters)
    :
    Window3(parameters),
    mAlpha(0.5f),
    mNumSamples0(128),
    mNumSamples1(64),
    mSample0(0),
    mSample1(0),
    mDX(0.1f),
    mDY(0.1f),
    mDZ(0.1f),
    mDrawSphereVisual(true)
{
    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;
    mEngine->SetRasterizerState(mNoCullState);

    CreateScene();

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 8.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mTrackball.Update();
    mPVWMatrices.Update();
}

void MovingSphereTriangleWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    // This is not the correct drawing order, but it is close enough for
    // demonstrating the moving sphere-triangle intersection query.
    mEngine->SetBlendState(mBlendState);

    if (mDrawSphereVisual)
    {
        mEngine->Draw(mSphereVisual);
    }
    mEngine->Draw(mVelocityVisual);
    if (mSphereContactVisual->culling != CullingMode::CULL_ALWAYS)
    {
        mEngine->Draw(mPointContactVisual);
        if (mDrawSphereVisual)
        {
            mEngine->Draw(mSphereContactVisual);
        }
    }

    mEngine->Draw(mTriangleVisual);
    for (int i = 0; i < 3; ++i)
    {
        mEngine->Draw(mVertexVisual[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        mEngine->Draw(mEdgeVisual[i]);
    }
    mEngine->Draw(mFaceVisual[0]);
    mEngine->Draw(mFaceVisual[1]);

    mEngine->SetDefaultBlendState();

    std::array<float, 4> const black{ 0.0f, 0.0f, 0.0f, 1.0f };
    mEngine->Draw(8, mYSize - 8, black, mTimer.GetFPS());
    mEngine->Draw(8, 24, black, mMessage);
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MovingSphereTriangleWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullState == mEngine->GetRasterizerState())
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;

        // Manually launch the intersection query.
    case ' ':
        UpdateSphereCenter();
        return true;

        // Modify theta in [0,2*pi].
    case 'a':
        mSample0 = (mSample0 + mNumSamples0 - 1) % mNumSamples0;
        UpdateSphereVelocity();
        return true;
    case 'A':
        mSample0 = (mSample0 + 1) % mNumSamples0;
        UpdateSphereVelocity();
        return true;

        // Modify phi in [0,pi].
    case 'b':
        mSample1 = (mSample1 + mNumSamples1 - 1) % mNumSamples1;
        UpdateSphereVelocity();
        return true;
    case 'B':
        mSample1 = (mSample1 + 1) % mNumSamples1;
        UpdateSphereVelocity();
        return true;

        // Translate the sphere.
    case 'x':
        mSphere.center[0] -= mDX;
        UpdateSphereCenter();
        return true;
    case 'X':
        mSphere.center[0] += mDX;
        UpdateSphereCenter();
        return true;
    case 'y':
        mSphere.center[1] -= mDY;
        UpdateSphereCenter();
        return true;
    case 'Y':
        mSphere.center[1] += mDY;
        UpdateSphereCenter();
        return true;
    case 'z':
        mSphere.center[2] -= mDZ;
        UpdateSphereCenter();
        return true;
    case 'Z':
        mSphere.center[2] += mDZ;
        UpdateSphereCenter();
        return true;

        // Toggle the drawing of the moving sphere.
    case 's':
    case 'S':
        mDrawSphereVisual = !mDrawSphereVisual;
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void MovingSphereTriangleWindow::CreateScene()
{
    mSphere.center = { 1.0f, 1.0f, 1.0f };
    mSphere.radius = 0.25f;
    mSphereVelocity = { 0.0f, 0.0f, -1.0f };

    mTriangle.v[0] = { 0.0f, 0.0f, 0.0f };
    mTriangle.v[1] = { 1.0f, 0.0f, 0.0f };
    mTriangle.v[2] = { 2.0f, 2.0f, 0.0f };
    mTriangleNormal = { 0.0f, 0.0f, 1.0f };
    mTriangleVelocity = { 0.0f, 0.0f, 0.0f };

    mSSVNode = std::make_shared<Node>();
    mTrackball.Attach(mSSVNode);

    CreateTriangleFaces();
    CreateHalfCylinders();
    CreateSphereWedges();
    CreateSpheres();
    CreateMotionCylinder();
    UpdateSphereVelocity();
}

void MovingSphereTriangleWindow::CreateTriangleFaces()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 3);
    auto* vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = mTriangle.v[0];
    vertices[1] = mTriangle.v[1];
    vertices[2] = mTriangle.v[2];
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 1);

    Vector4<float> color{ 0.75f, 0.75f, 0.75f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mTriangleVisual = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mTriangleVisual->worldTransform, effect->GetPVWMatrixConstant());
    mSSVNode->AttachChild(mTriangleVisual);

    Vector3<float> triangleNormal{ 0.0f, 0.0f, 1.0f };
    color = Vector4<float>{ 1.0f, 0.0f, 0.0f, mAlpha };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mFaceVisual[0] = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mFaceVisual[0]->localTransform.SetTranslation(0.0f, 0.0f, mSphere.radius);
    mPVWMatrices.Subscribe(mFaceVisual[0]->worldTransform, effect->GetPVWMatrixConstant());
    mSSVNode->AttachChild(mFaceVisual[0]);

    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mFaceVisual[1] = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mFaceVisual[1]->localTransform.SetTranslation(0.0f, 0.0f, -mSphere.radius);
    mPVWMatrices.Subscribe(mFaceVisual[1]->worldTransform, effect->GetPVWMatrixConstant());
    mSSVNode->AttachChild(mFaceVisual[1]);
}

void MovingSphereTriangleWindow::CreateHalfCylinders()
{
    for (int i = 0; i < 3; ++i)
    {
        CreateHalfCylinder(i, mTriangle.v[i], mTriangle.v[(i+1) % 3],
            mTriangleNormal, mSphere.radius);
    }

}

void MovingSphereTriangleWindow::CreateSphereWedges()
{
    for (int i = 0; i < 3; ++i)
    {
        CreateSphere(i, mTriangle.v[i], mSphere.radius);
    }
}

void MovingSphereTriangleWindow::CreateHalfCylinder(int i, Vector3<float> const& P0,
    Vector3<float> const& P1, Vector3<float> const& normal, float radius)
{
    Vector3<float> E = P1 - P0;
    Vector3<float> V = UnitCross(E, normal);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    unsigned int const density = 32;
    mEdgeVisual[i] = mf.CreateRectangle(density, density, 1.0f, 1.0f);
    auto vbuffer = mEdgeVisual[i]->GetVertexBuffer();
    auto ibuffer = mEdgeVisual[i]->GetIndexBuffer();
    Vector3<float>* vertices = vbuffer->Get<Vector3<float>>();
    for (unsigned int row = 0; row < density; ++row)
    {
        float z = (float)row / ((float)density - 1.0f);
        for (unsigned int col = 0; col < density; ++col)
        {
            float angle = (float)GTE_C_PI * (float)col / ((float)density - 1.0f);
            float cs = std::cos(angle), sn = std::sin(angle);
            Vector3<float> P = P0 + z * E + radius * (cs * normal + sn * V);
            *vertices++ = P;
        }
    }

    Vector4<float> color{ 1.0f, 0.0f, 1.0f, mAlpha };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mEdgeVisual[i]->SetEffect(effect);
    mPVWMatrices.Subscribe(mEdgeVisual[i]->worldTransform, effect->GetPVWMatrixConstant());
    mSSVNode->AttachChild(mEdgeVisual[i]);
}

void MovingSphereTriangleWindow::CreateSphere(int i, Vector3<float> const& C, float radius)
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    unsigned int const density = 32;
    mVertexVisual[i] = mf.CreateSphere(density, density, radius);
    mVertexVisual[i]->localTransform.SetTranslation(C);

    Vector4<float> color{ 0.5f, 0.5f, 1.0f, mAlpha };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mVertexVisual[i]->SetEffect(effect);
    mPVWMatrices.Subscribe(mVertexVisual[i]->worldTransform, effect->GetPVWMatrixConstant());
    mSSVNode->AttachChild(mVertexVisual[i]);
}

void MovingSphereTriangleWindow::CreateSpheres()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mSphereVisual = mf.CreateSphere(16, 16, mSphere.radius);
    Vector4<float> color{ 0.75f, 0.75f, 0.75f, mAlpha };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mSphereVisual->SetEffect(effect);
    mSphereVisual->localTransform.SetTranslation(mSphere.center);
    mPVWMatrices.Subscribe(mSphereVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mSphereVisual);

    mSphereContactVisual = mf.CreateSphere(16, 16, mSphere.radius);
    color = { 0.25f, 0.25f, 0.25f, mAlpha };
    mSphereContactVisual->culling = CullingMode::CULL_ALWAYS;
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mSphereContactVisual->SetEffect(effect);
    mSphereContactVisual->localTransform.SetTranslation(mSphere.center);
    mPVWMatrices.Subscribe(mSphereContactVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mSphereContactVisual);

    mPointContactVisual = mf.CreateSphere(8, 8, mSphere.radius / 8.0f);
    color = { 0.0f, 0.0f, 0.0f, mAlpha };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mPointContactVisual->SetEffect(effect);
    mPointContactVisual->localTransform.SetTranslation(mSphere.center);
    mPVWMatrices.Subscribe(mPointContactVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mPointContactVisual);
}

void MovingSphereTriangleWindow::CreateMotionCylinder()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    auto vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { 0.0f, 0.0f, 0.0f };
    vertices[1] = { 0.0f, 0.0f, 1000.0f };
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);
    Vector4<float> color{ 0.0f, 1.0f, 0.0f, mAlpha };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color);
    mVelocityVisual = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mVelocityVisual->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mVelocityVisual);
}

void MovingSphereTriangleWindow::UpdateSphereVelocity()
{
    float angle0 = mSample0 * (float)GTE_C_TWO_PI / mNumSamples0;
    float angle1 = mSample1 * (float)GTE_C_PI / mNumSamples1;
    float cs0 = std::cos(angle0), sn0 = std::sin(angle0);
    float cs1 = std::cos(angle1), sn1 = std::sin(angle1);
    mSphereVelocity = { cs0 * sn1, sn0 * sn1, cs1 };

    Vector3<float> basis[3];
    basis[0] = mSphereVelocity;
    ComputeOrthogonalComplement(1, basis);
    Matrix3x3<float> rotate;
    rotate.SetCol(0, basis[1]);
    rotate.SetCol(1, basis[2]);
    rotate.SetCol(2, basis[0]);
    mVelocityVisual->localTransform.SetRotation(rotate);
    mVelocityVisual->localTransform.SetTranslation(mSphere.center);
    mVelocityVisual->Update();

#if defined(USE_FLOATING_POINT_QUERY)
    auto result = mQuery(mSphere, mSphereVelocity, mTriangle, mTriangleVelocity);
    bool intersect = (result.intersectionType != 0);
    if (intersect)
    {
        Vector3<float> P = result.contactPoint;
        mSphereContactVisual->culling = CullingMode::CULL_DYNAMIC;
        mSphereContactVisual->localTransform.SetTranslation(P);
        mSphereContactVisual->Update();
        mPointContactVisual->localTransform.SetTranslation(P);
        mPointContactVisual->Update();
        mMessage = "T = " + std::to_string(result.contactTime) + ", P = (" +
            std::to_string(P[0]) + ", " +
            std::to_string(P[1]) + ", " +
            std::to_string(P[2]) + ")";
    }
    else
    {
        mSphereContactVisual->culling = CullingMode::CULL_ALWAYS;
        mMessage = "";
    }
#else
    Sphere3<Rational> rSphere;
    Triangle3<Rational> rTriangle;
    Vector3<Rational> rSphereVelocity, rTriangleVelocity;
    rSphere.center = { mSphere.center[0], mSphere.center[1], mSphere.center[2] };
    rSphere.radius = mSphere.radius;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            rTriangle.v[i][j] = mTriangle.v[i][j];
        }
        rSphereVelocity[i] = mSphereVelocity[i];
        rTriangleVelocity[i] = mTriangleVelocity[i];
    }

    auto result = mQuery(rSphere, rSphereVelocity, rTriangle, rTriangleVelocity);
    bool intersect = (result.intersectionType != 0);
    if (intersect)
    {
        float contactTime = result.field.Convert(result.contactTime);
        Vector3<float> P
        {
            result.field.Convert(result.contactPoint[0]),
            result.field.Convert(result.contactPoint[1]),
            result.field.Convert(result.contactPoint[2])
        };

        mSphereContactVisual->culling = CullingMode::CULL_DYNAMIC;
        mSphereContactVisual->localTransform.SetTranslation(P);
        mSphereContactVisual->Update();
        mPointContactVisual->localTransform.SetTranslation(P);
        mPointContactVisual->Update();
        mMessage = "T = " + std::to_string(contactTime) + ", P = (" +
            std::to_string(P[0]) + ", " +
            std::to_string(P[1]) + ", " +
            std::to_string(P[2]) + ")";
    }
    else
    {
        mSphereContactVisual->culling = CullingMode::CULL_ALWAYS;
        mMessage = "";
    }
#endif

    mPVWMatrices.Update();
    mTrackball.Update();
}

void MovingSphereTriangleWindow::UpdateSphereCenter()
{
    mSphereVisual->localTransform.SetTranslation(mSphere.center);
    mSphereVisual->Update();
    UpdateSphereVelocity();
}