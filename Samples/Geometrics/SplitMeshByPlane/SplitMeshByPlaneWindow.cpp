// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.0 (2019/04/08)

#include "SplitMeshByPlaneWindow.h"

#include <Mathematics/GteSplitMeshByPlane.h>

namespace gte
{
    template class SplitMeshByPlane<float>;
}

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

    Window::Parameters parameters(L"SplitMeshByPlaneWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<SplitMeshByPlaneWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<SplitMeshByPlaneWindow>(window);
    return 0;
}

SplitMeshByPlaneWindow::SplitMeshByPlaneWindow(Parameters& parameters)
    :
    Window3(parameters),
    mTorusMoved(false)
{
    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 1000.0f, 0.01f, 0.001f,
        { 16.0f, 0.0f, 4.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    CreateScene();
    mTrackball.Update();
    mPVWMatrices.Update();
    Update();
}

void SplitMeshByPlaneWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    if (mTorusMoved)
    {
        Update();
    }

    mEngine->ClearBuffers();

    // Draw the torus in wireframe when it is active.
    mEngine->Draw(mMeshTorus);

    // Always draw the plane in wireframe.
    auto save = mEngine->GetRasterizerState();
    mEngine->SetRasterizerState(mWireState);
    mEngine->Draw(mMeshPlane);
    mEngine->SetRasterizerState(save);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool SplitMeshByPlaneWindow::OnCharPress(unsigned char key, int x, int y)
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
    }
    return Window3::OnCharPress(key, x, y);
}

bool SplitMeshByPlaneWindow::OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers)
{
    mTorusMoved = Window3::OnMouseMotion(button, x, y, modifiers);
    return mTorusMoved;
}

void SplitMeshByPlaneWindow::CreateScene()
{
    // The plane is fixed at z = 0.
    mPlane.normal = { 0.0f, 0.0f, 1.0f };
    mPlane.constant = 0.0f;

    // The plane has a single color (green).
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mMeshPlane = mf.CreateRectangle(32, 32, 16.0f, 16.0f);
    mMeshEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 1.0f, 0.0f, 1.0f });
    mMeshPlane->SetEffect(mMeshEffect);
    mPVWMatrices.Subscribe(mMeshPlane->worldTransform, mMeshEffect->GetPVWMatrixConstant());
    //mTrackball.Attach(mMeshPlane);

    // The torus will generally be 2-colored (red and blue).
    vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    mf.SetVertexFormat(vformat);

    // Get the positions and indices for a torus.
    mMeshTorus = mf.CreateTorus(64, 64, 4.0f, 1.0f);
    mTorusEffect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mMeshTorus->SetEffect(mTorusEffect);
    mPVWMatrices.Subscribe(mMeshTorus->worldTransform, mTorusEffect->GetPVWMatrixConstant());
    mTrackball.Attach(mMeshTorus);

    auto vbuffer = mMeshTorus->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<TorusVertex>();
    mTorusVerticesMS.resize(numVertices);
    mTorusVerticesWS.resize(numVertices);
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        mTorusVerticesMS[i] = vertices[i].position;
        mTorusVerticesWS[i] = mTorusVerticesMS[i];
        vertices[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    auto ibuffer = mMeshTorus->GetIndexBuffer();
    int numIndices = ibuffer->GetNumElements();
    auto const* indices = ibuffer->Get<int>();
    mTorusIndices.resize(numIndices);
    memcpy(mTorusIndices.data(), indices, mTorusIndices.size() * sizeof(int));
}

void SplitMeshByPlaneWindow::Update()
{
    // Transform the model-space vertices to world space.
    for (size_t i = 0; i < mTorusVerticesMS.size(); ++i)
    {
        mTorusVerticesWS[i] = HProject(
            mMeshTorus->worldTransform * HLift(mTorusVerticesMS[i], 1.0f));
    }

    // Partition the torus mesh.
    std::vector<Vector3<float>> clipVertices;
    std::vector<int> negIndices, posIndices;
    SplitMeshByPlane<float> splitter;
    splitter(mTorusVerticesWS, mTorusIndices, mPlane, clipVertices, negIndices, posIndices);

    // Replace the torus vertex buffer.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    unsigned int numVertices = static_cast<unsigned int>(clipVertices.size());
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    mMeshTorus->SetVertexBuffer(vbuffer);
    auto* vertices = vbuffer->Get<TorusVertex>();
    Matrix4x4<float> inverse = mMeshTorus->worldTransform.GetHInverse();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        // Transform the world-space vertex to model space.
        vertices[i].position = HProject(inverse * HLift(clipVertices[i], 1.0f));
        vertices[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    // Modify the vertex color based on which submesh the vertices lie.
    unsigned int negQuantity = static_cast<unsigned int>(negIndices.size());
    for (unsigned int i = 0; i < negQuantity; ++i)
    {
        // Set the negative mesh color to blue.
        vertices[negIndices[i]].color[2] = 1.0f;
    }
    unsigned int posQuantity = static_cast<unsigned int>(posIndices.size());
    for (unsigned int i = 0; i < posQuantity; ++i)
    {
        // Set the positive mesh color to red.
        vertices[posIndices[i]].color[0] = 1.0f;
    }

    // To display the triangles generated by the split.
    unsigned int numTriangles = (negQuantity + posQuantity) / 3;
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(unsigned int));
    mMeshTorus->SetIndexBuffer(ibuffer);
    auto* indices = ibuffer->Get<int>();
    memcpy(indices, negIndices.data(), negQuantity * sizeof(int));
    memcpy(indices + negQuantity, posIndices.data(), posQuantity * sizeof(int));
}
