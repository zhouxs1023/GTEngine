// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "VertexCollapseMeshWindow.h"
#include <iostream>

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

    Window::Parameters parameters(L"VertexCollapseMeshWindow", 0, 0, 1024, 1024);
    auto window = TheWindowSystem.Create<VertexCollapseMeshWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<VertexCollapseMeshWindow>(window);
    return 0;
}

VertexCollapseMeshWindow::VertexCollapseMeshWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 1.0f, 0.01f,
        { 0.0f, 0.0f, 6.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });

    VertexFormat format;
    format.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(format);
    std::shared_ptr<Visual> cylinder = mf.CreateCylinderOpen(8, 8, 1.0f, 2.0f);
    unsigned int numPositions = cylinder->GetVertexBuffer()->GetNumElements();
    mPositions.resize(numPositions);
    memcpy(mPositions.data(), cylinder->GetVertexBuffer()->GetData(), numPositions * sizeof(Vector3<float>));
    unsigned int numTriangles = cylinder->GetIndexBuffer()->GetNumPrimitives();
    mTriangles.resize(numTriangles);
    memcpy(mTriangles.data(), cylinder->GetIndexBuffer()->GetData(), numTriangles * 3 * sizeof(int));

    mVCMesh = std::make_shared<VertexCollapseMesh<float>>(numPositions, &mPositions[0],
        3 * numTriangles, (int const*)&mTriangles[0]);

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer =
        std::make_shared<VertexBuffer>(vformat, numPositions);
    Vertex* vertex = vbuffer->Get<Vertex>();
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    Vector3<float> average{ 0.0f, 0.0f, 0.0f };
    for (unsigned int i = 0; i < numPositions; ++i)
    {
        vertex[i].position = mPositions[i];
        vertex[i].color[0] = rnd(mte);
        vertex[i].color[1] = rnd(mte);
        vertex[i].color[2] = rnd(mte);
        vertex[i].color[3] = 1.0f;
        average += mPositions[i];
    }
    average /= (float)numPositions;

    std::shared_ptr<IndexBuffer> ibuffer =
        std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(int));
    int* index = ibuffer->Get<int>();
    for (auto const& element : mVCMesh->GetMesh().GetTriangles())
    {
        *index++ = element.first.V[0];
        *index++ = element.first.V[1];
        *index++ = element.first.V[2];
    }

    std::shared_ptr<VertexColorEffect> effect =
        std::make_shared<VertexColorEffect>(mProgramFactory);
    mSurface = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSurface->localTransform.SetTranslation(-average);
    mTrackball.Attach(mSurface);
    mTrackball.Update();
    mPVWMatrices.Subscribe(mSurface->worldTransform, effect->GetPVWMatrixConstant());
    mPVWMatrices.Update();
}

void VertexCollapseMeshWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mSurface);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool VertexCollapseMeshWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullWireState != mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;

    case 'c':
    case 'C':
    {
        VertexCollapseMesh<float>::Record record;
        if (mVCMesh->DoCollapse(record))
        {
            std::cout << "v = " << record.vertex << " rs = " << record.removed.size() << " is = " << record.inserted.size() << std::endl;
            auto const& mesh = mVCMesh->GetMesh();
            unsigned int const numTriangles =
                static_cast<unsigned int>(mesh.GetTriangles().size());
            std::shared_ptr<IndexBuffer> ibuffer =
                std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(unsigned int));
            unsigned int* index = ibuffer->Get<unsigned int>();
            for (auto const& element : mesh.GetTriangles())
            {
                *index++ = element.first.V[0];
                *index++ = element.first.V[1];
                *index++ = element.first.V[2];
            }

            mSurface->SetIndexBuffer(ibuffer);
        }
        return true;
    }
    }

    return Window::OnCharPress(key, x, y);
}
