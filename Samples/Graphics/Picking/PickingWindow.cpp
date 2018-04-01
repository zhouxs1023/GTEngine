// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "PickingWindow.h"

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

    Window::Parameters parameters(L"PickingWindow", 0, 0, 768, 768);
    auto window = TheWindowSystem.Create<PickingWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<PickingWindow>(window);
    return 0;
}

PickingWindow::PickingWindow(Parameters& parameters)
    :
    Window3(parameters),
    mNumActiveSpheres(0)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { -16.0f, 0.0f, 2.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void PickingWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mTorus);
    mEngine->Draw(mDodecahedron);
    mEngine->Draw(mPoints);
    mEngine->Draw(mSegments);
    for (int i = 0; i < mNumActiveSpheres; ++i)
    {
        mEngine->Draw(mSphere[i]);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool PickingWindow::OnMouseClick(MouseButton button, MouseState state, int x,
    int y, unsigned int modifiers)
{
    if (!Window3::OnMouseClick(button, state, x, y, modifiers))
    {
        if (button == MOUSE_RIGHT && state == MOUSE_DOWN)
        {
            DoPick(x, mYSize - 1 - y);
        }
    }
    return true;
}

bool PickingWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Checkerboard.png") == "")
    {
        LogError("Cannot open file Checkerboard.png.");
        return false;
    }

    return true;
}

void PickingWindow::CreateScene()
{
    std::string path = mEnvironment.GetPath("Checkerboard.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, false);

    mScene = std::make_shared<Node>();

    VertexFormat vformat0;
    vformat0.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat0.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    // The torus and dodecahedron are created by the mesh factory in which
    // the 'visual' model bounds are computed.  The points and segments
    // primitives are created explicitly here, so we need to compute their
    // model bounds to be used by the picking system.
    MeshFactory mf;
    mf.SetVertexFormat(vformat0);

    mTorus = mf.CreateTorus(16, 16, 4.0f, 1.0f);
    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(
        mProgramFactory, texture, SamplerState::MIN_L_MAG_L_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP);
    mTorus->SetEffect(effect);
    mPVWMatrices.Subscribe(mTorus->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mTorus);

    mDodecahedron = mf.CreateDodecahedron();
    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP,
        SamplerState::CLAMP);
    mDodecahedron->SetEffect(effect);
    mPVWMatrices.Subscribe(mDodecahedron->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mDodecahedron);

    VertexFormat vformat1;
    vformat1.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat1, 4);
    Vector3<float>* vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { 1.0f, 1.0f, 4.0f };
    vertices[1] = { 1.0f, 2.0f, 5.0f };
    vertices[2] = { 2.0f, 2.0f, 6.0f };
    vertices[3] = { 2.0f, 1.0f, 7.0f };
    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, 4);
    std::shared_ptr<ConstantColorEffect> cceffect =
        std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.5f, 0.0f, 0.0f, 1.0f }));
    mPoints = std::make_shared<Visual>(vbuffer, ibuffer, cceffect);
    mPoints->UpdateModelBound();
    mPVWMatrices.Subscribe(mPoints->worldTransform, cceffect->GetPVWMatrixConstant());
    mScene->AttachChild(mPoints);

    vbuffer = std::make_shared<VertexBuffer>(vformat1, 4);
    vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { -1.0f, -1.0f, 4.0f };
    vertices[1] = { -1.0f, -2.0f, 5.0f };
    vertices[2] = { -2.0f, -1.0f, 6.0f };
    vertices[3] = { -2.0f, -2.0f, 7.0f };
    ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, 3,
        sizeof(int));
    ibuffer->SetSegment(0, 0, 1);
    ibuffer->SetSegment(1, 1, 2);
    ibuffer->SetSegment(2, 2, 3);
    cceffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.0f, 0.0f, 0.5f, 1.0f }));
    mSegments = std::make_shared<Visual>(vbuffer, ibuffer, cceffect);
    mSegments->UpdateModelBound();
    mPVWMatrices.Subscribe(mSegments->worldTransform, cceffect->GetPVWMatrixConstant());
    mScene->AttachChild(mSegments);

    for (int i = 0; i < SPHERE_BUDGET; ++i)
    {
        mSphere[i] = mf.CreateSphere(8, 8, 0.125f);
        cceffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
            Vector4<float>({ 0.0f, 0.0f, 0.0f, 1.0f }));
        mSphere[i]->SetEffect(cceffect);
        mPVWMatrices.Subscribe(mSphere[i]->worldTransform, cceffect->GetPVWMatrixConstant());
        mScene->AttachChild(mSphere[i]);
    }

    mTrackball.Attach(mScene);
    mTrackball.Update();
}

void PickingWindow::DoPick(int x, int y)
{
    int viewX, viewY, viewW, viewH;
    mEngine->GetViewport(viewX, viewY, viewW, viewH);
    Vector4<float> origin, direction;
    if (mCamera->GetPickLine(viewX, viewY, viewW, viewH, x, y, origin, direction))
    {
        // Use a ray for picking.
        float tmin = 0.0f, tmax = std::numeric_limits<float>::max();

        // Set the distance tolerance for point and segment primitives.
        mPicker.SetMaxDistance(0.0625f);

        // Request the results in model-space coordinates.  All the objects
        // in the scene have the same model space, so we can set the sphere
        // centers in model-space coordinates.
        mPicker(mScene, origin, direction, tmin, tmax);
        mNumActiveSpheres = std::min((int)mPicker.records.size(), (int)SPHERE_BUDGET);
        if (mNumActiveSpheres > 0)
        {
            // Place spheres at the picked locations.
            Matrix4x4<float> const& invWMatrix = mScene->worldTransform.GetHInverse();
            for (int i = 0; i < mNumActiveSpheres; ++i)
            {
#if defined(GTE_USE_MAT_VEC)
                Vector4<float> modelPosition = invWMatrix * mPicker.records[i].primitivePoint;
#else
                Vector4<float> modelPosition = mPicker.records[i].primitivePoint * invWMatrix;
#endif
                mSphere[i]->localTransform.SetTranslation(modelPosition);
            }

            mTrackball.Update();
            mPVWMatrices.Update();
        }
    }
}
