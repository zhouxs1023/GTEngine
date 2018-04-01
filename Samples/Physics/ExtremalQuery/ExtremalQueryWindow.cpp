// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "ExtremalQueryWindow.h"

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

    Window::Parameters parameters(L"ExtremalQueryWindow", 0, 0, 640, 480);
    auto window = TheWindowSystem.Create<ExtremalQueryWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<ExtremalQueryWindow>(window);
    return 0;
}

ExtremalQueryWindow::ExtremalQueryWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    // Set up an orthogonal camera.  This projection type is used to make it
    // clear that the displayed extreme points really are extreme; the
    // perspective projection is deceptive.
#if defined(GTE_DEV_OPENGL)
    mCamera = std::make_shared<Camera>(false, false);
#else
    mCamera = std::make_shared<Camera>(false, true);
#endif
    mCamera->SetFrustum(1.0f, 1000.0f, -1.5f, 1.5f, -2.0, 2.0f);
    Vector4<float> camPosition{ 4.0f, 0.0f, 0.0f, 1.0f };
    Vector4<float> camDVector{ -1.0f, 0.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mPVWMatrices.Set(mCamera, mUpdater);
    mTrackball.Set(mXSize, mYSize, mCamera);

    CreateScene();
}

void ExtremalQueryWindow::OnIdle()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    mEngine->Draw(mConvexMesh);
    mEngine->Draw(mMaxSphere);
    mEngine->Draw(mMinSphere);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ExtremalQueryWindow::OnCharPress(unsigned char key, int x, int y)
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

bool ExtremalQueryWindow::OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        UpdateExtremePoints();
    }
    return true;
}

void ExtremalQueryWindow::CreateScene()
{
    mScene = std::make_shared<Node>();

    // Create a convex polyhedron that is the hull of numVertices randomly generated points.
    int const numVertices = 32;
    CreateConvexPolyhedron(numVertices);
    CreateVisualConvexPolyhedron();

    // Use small spheres to show the extreme points in the camera's right direction.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mMaxSphere = mf.CreateSphere(8, 8, 0.05f);
    mMinSphere = mf.CreateSphere(8, 8, 0.05f);

    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    std::shared_ptr<ConstantColorEffect> effect;
    
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);
    mMaxSphere->SetEffect(effect);
    mPVWMatrices.Subscribe(mMaxSphere->worldTransform, effect->GetPVWMatrixConstant());

    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);
    mMinSphere->SetEffect(effect);
    mPVWMatrices.Subscribe(mMinSphere->worldTransform, effect->GetPVWMatrixConstant());

    mScene->AttachChild(mMaxSphere);
    mScene->AttachChild(mMinSphere);

    mTrackball.Attach(mScene);
    mTrackball.Update();
    UpdateExtremePoints();
}

void ExtremalQueryWindow::CreateConvexPolyhedron(int numVertices)
{
    // Create the convex hull of a randomly generated set of points on the unit sphere.
    std::shared_ptr<std::vector<Vector3<float>>> vertexPool =
        std::make_shared<std::vector<Vector3<float>>>(numVertices);
    auto& vertices = *vertexPool.get();
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    for (auto& vertex : vertices)
    {
        for (int j = 0; j < 3; ++j)
        {
            vertex[j] = rnd(mte);
        }
        Normalize(vertex);
    }

    typedef BSNumber<UIntegerAP32> Rational;
    ConvexHull3<float, Rational> hull;
    bool success = hull(numVertices, vertices.data(), 0.0f);
    LogAssert(success, "Invalid polyhedron.");
    (void)success;

    auto const& triangles = hull.GetHullUnordered();
    int numIndices = 3 * static_cast<int>(triangles.size());
    int const* indices = reinterpret_cast<int const*>(triangles.data());
    mConvexPolyhedron = std::make_unique<Polyhedron3<float>>(vertexPool, numIndices, indices, true);

#ifdef USE_BSP_QUERY
    mExtremalQuery = std::make_unique<ExtremalQuery3BSP<float>>(*mConvexPolyhedron.get());
#else
    mExtremalQuery = std::make_unique<ExtremalQuery3PRJ<float>>(*mConvexPolyhedron.get());
#endif

#ifdef MEASURE_TIMING_OF_QUERY
    // For timing purposes and determination of asymptotic order.
    int const imax = 10000000;
    std::vector<Vector3<float>> directions(imax);
    for (auto& direction : directions)
    {
        for (int j = 0; j < 3; ++j)
        {
            direction[j] = rnd(mte);
        }
        Normalize(direction);
    }

    Timer timer;
    for (int i = 0; i < imax; ++i)
    {
        int pos, neg;
        mExtremalQuery->GetExtremeVertices(directions[i], pos, neg);
    }
    double duration = timer.GetSeconds();
    std::ofstream outFile("timing.txt");
    outFile << "duration = " << duration << " seconds" << std::endl;
    outFile.close();
#endif
}

void ExtremalQueryWindow::CreateVisualConvexPolyhedron()
{
    auto vertexPool = mConvexPolyhedron->GetVertices();
    auto const& polyIndices = mConvexPolyhedron->GetIndices();
    int const numIndices = static_cast<int>(polyIndices.size());
    int const numTriangles = numIndices / 3;

    // Visualize the convex polyhedron as a collection of face-colored triangles.
    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer =
        std::make_shared<VertexBuffer>(vformat, numIndices);
    Vertex* vertices = vbuffer->Get<Vertex>();

    std::shared_ptr<IndexBuffer> ibuffer =
        std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(unsigned int));
    int* indices = ibuffer->Get<int>();
    for (int i = 0; i < numIndices; ++i)
    {
        vertices[i].position = vertexPool[polyIndices[i]];
        indices[i] = i;
    }

    // Use randomly generated vertex colors.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    for (int t = 0; t < numTriangles; ++t)
    {
        Vector4<float> color{ rnd(mte), rnd(mte), rnd(mte), 1.0f };
        for (int j = 0; j < 3; ++j)
        {
            vertices[3 * t + j].color = color;
        }
    }

    std::shared_ptr<VertexColorEffect> effect =
        std::make_shared<VertexColorEffect>(mProgramFactory);

    mConvexMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mConvexMesh->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mConvexMesh);
}

void ExtremalQueryWindow::UpdateExtremePoints()
{
#if defined(GTE_USE_MAT_VEC)
    Vector4<float> rVector = mScene->worldTransform.GetHInverse() * mCamera->GetRVector();
#else
    Vector4<float> rVector = mCamera->GetRVector() * mScene->worldTransform.GetHInverse();
#endif
    Vector3<float> direction = HProject<4, float>(rVector);

    int posDir, negDir;
    mExtremalQuery->GetExtremeVertices(direction, posDir, negDir);

    auto vertexPool = mConvexPolyhedron->GetVertices();
    mMaxSphere->localTransform.SetTranslation(vertexPool[posDir]);
    mMinSphere->localTransform.SetTranslation(vertexPool[negDir]);

    mTrackball.Update();
    mPVWMatrices.Update();
}
