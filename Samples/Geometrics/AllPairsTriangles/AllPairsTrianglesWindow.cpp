// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "AllPairsTrianglesWindow.h"

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

    Window::Parameters parameters(L"AllPairsTrianglesWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<AllPairsTrianglesWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<AllPairsTrianglesWindow>(window);
    return 0;
}

AllPairsTrianglesWindow::AllPairsTrianglesWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    int const count = 48;
    if (!SetEnvironment()
        || !CreateCylinder(count, count, 1.0f, 8.0f)    // 4416 triangles
        || !CreateTorus(count, count, 2.0f, 0.5f))      // 4608 triangles
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

#if !defined(USE_CPU_FIND_INTERSECTIONS)
    if (!CreateShaders())
    {
        parameters.created = false;
        return;
    }
#endif

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.1f, 0.01f,
        { 8.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mPVWMatrices.Update();
}

void AllPairsTrianglesWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    UpdateTransforms();
    FindIntersections();

    mEngine->ClearBuffers();
#if defined(USE_CPU_FIND_INTERSECTIONS)
    mEngine->Draw(mCylinder);
    mEngine->Draw(mTorus);
#else
    mEngine->Draw(mCylinderID);
    mEngine->Draw(mTorusID);
#endif
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool AllPairsTrianglesWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() != mWireState)
        {
            mEngine->SetRasterizerState(mWireState);
        }
        else
        {
            mEngine->SetDefaultRasterizerState();
        }
        return true;
    }
    return Window::OnCharPress(key, x, y);
}

bool AllPairsTrianglesWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Geometrics/AllPairsTriangles/Shaders/");
#if defined(GTE_DEV_OPENGL)
    std::vector<std::string> inputs =
    {
        "DrawUsingVertexIDVS.glsl",
        "DrawUsingVertexIDPS.glsl",
        "InitializeColors.glsl",
        "TriangleIntersection.glsl",
        "VertexColorIndexedVS.glsl",
        "VertexColorIndexedPS.glsl"
    };
#else
    std::vector<std::string> inputs =
    {
        "DrawUsingVertexID.hlsl",
        "InitializeColors.hlsl",
        "TriangleIntersection.hlsl",
        "VertexColorIndexed.hlsl"
    };
#endif

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

bool AllPairsTrianglesWindow::CreateCylinder(unsigned int numAxisSamples,
    unsigned int numRadialSamples, float radius, float height)
{
#if defined(GTE_DEV_OPENGL)
    std::string pathVS = mEnvironment.GetPath("VertexColorIndexedVS.glsl");
    std::string pathPS = mEnvironment.GetPath("VertexColorIndexedPS.glsl");
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(pathVS, pathPS, "");
#else
    std::string path = mEnvironment.GetPath("VertexColorIndexed.hlsl");
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(path, path, "");
#endif
    if (!program)
    {
        return false;
    }

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetIndexFormat(true);

    std::shared_ptr<Visual> cylinder = mf.CreateCylinderClosed(
        numAxisSamples, numRadialSamples, radius, height);
    std::shared_ptr<VertexBuffer> vbuffer = cylinder->GetVertexBuffer();
    Vector3<float>* vertices = vbuffer->Get<Vector3<float>>();
    std::shared_ptr<IndexBuffer> ibuffer = cylinder->GetIndexBuffer();
    unsigned int numIndices = ibuffer->GetNumElements();
    unsigned int* indices = ibuffer->Get<unsigned int>();

    VertexFormat meshVFormat;
    meshVFormat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    meshVFormat.Bind(VA_COLOR, DF_R32_UINT, 0);
    std::shared_ptr<VertexBuffer> meshVBuffer =
        std::make_shared<VertexBuffer>(meshVFormat, numIndices);
    meshVBuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    Vertex* meshVertices = meshVBuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numIndices; ++i)
    {
        meshVertices[i].position = vertices[indices[i]];
        meshVertices[i].colorIndex = 0;
    }

    mNumCylinderTriangles = numIndices / 3;
    std::shared_ptr<IndexBuffer> meshIBuffer =
        std::make_shared<IndexBuffer>(IP_TRIMESH, mNumCylinderTriangles);

    mCylinderPVWMatrix = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVShader()->Set("PVWMatrix", mCylinderPVWMatrix);
    mCylinderEffect = std::make_shared<VisualEffect>(program);

    mCylinder = std::make_shared<Visual>(meshVBuffer, meshIBuffer, mCylinderEffect);
    return true;
}

bool AllPairsTrianglesWindow::CreateTorus(unsigned int numCircleSamples,
    unsigned int numRadialSamples, float outerRadius, float innerRadius)
{
#if defined(GTE_DEV_OPENGL)
    std::string pathVS = mEnvironment.GetPath("VertexColorIndexedVS.glsl");
    std::string pathPS = mEnvironment.GetPath("VertexColorIndexedPS.glsl");
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(pathVS, pathPS, "");
#else
    std::string path = mEnvironment.GetPath("VertexColorIndexed.hlsl");
    std::shared_ptr<VisualProgram> program = mProgramFactory->CreateFromFiles(path, path, "");
#endif
    if (!program)
    {
        return false;
    }

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetIndexFormat(true);

    std::shared_ptr<Visual> cylinder = mf.CreateTorus(
        numCircleSamples, numRadialSamples, outerRadius, innerRadius);
    std::shared_ptr<VertexBuffer> vbuffer = cylinder->GetVertexBuffer();
    Vector3<float>* vertices = vbuffer->Get<Vector3<float>>();
    std::shared_ptr<IndexBuffer> ibuffer = cylinder->GetIndexBuffer();
    unsigned int numIndices = ibuffer->GetNumElements();
    unsigned int* indices = ibuffer->Get<unsigned int>();

    VertexFormat meshVFormat;
    meshVFormat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    meshVFormat.Bind(VA_COLOR, DF_R32_UINT, 0);
    std::shared_ptr<VertexBuffer> meshVBuffer =
        std::make_shared<VertexBuffer>(meshVFormat, numIndices);
    meshVBuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    Vertex* meshVertices = meshVBuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numIndices; ++i)
    {
        meshVertices[i].position = vertices[indices[i]];
        meshVertices[i].colorIndex = 1;
    }

    mNumTorusTriangles = numIndices / 3;
    std::shared_ptr<IndexBuffer> meshIBuffer =
        std::make_shared<IndexBuffer>(IP_TRIMESH, mNumTorusTriangles);

    mTorusPVWMatrix = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVShader()->Set("PVWMatrix", mTorusPVWMatrix);
    mTorusEffect = std::make_shared<VisualEffect>(program);

    mTorus = std::make_shared<Visual>(meshVBuffer, meshIBuffer, mTorusEffect);
    return true;
}

#if !defined(USE_CPU_FIND_INTERSECTIONS)

bool AllPairsTrianglesWindow::CreateShaders()
{
    // Create the compute programs.
    unsigned int const numThreads = 8;
    mNumXGroups = mNumCylinderTriangles / numThreads;
    mNumYGroups = mNumTorusTriangles / numThreads;
    mProgramFactory->defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", numThreads);
#if defined(GTE_DEV_OPENGL)
    std::string path = mEnvironment.GetPath("InitializeColors.glsl");
#else
    std::string path = mEnvironment.GetPath("InitializeColors.hlsl");
#endif
    mInitializeColor = mProgramFactory->CreateFromFile(path);
    if (!mInitializeColor)
    {
        return false;
    }
#if defined(GTE_DEV_OPENGL)
    path = mEnvironment.GetPath("TriangleIntersection.glsl");
#else
    path = mEnvironment.GetPath("TriangleIntersection.hlsl");
#endif
    mTriangleIntersection = mProgramFactory->CreateFromFile(path);
    if (!mTriangleIntersection)
    {
        return false;
    }
    mProgramFactory->defines.Clear();

    // Create the visual programs.
#if defined(GTE_DEV_OPENGL)
    std::string pathVS = mEnvironment.GetPath("DrawUsingVertexIDVS.glsl");
    std::string pathPS = mEnvironment.GetPath("DrawUsingVertexIDPS.glsl");
    std::shared_ptr<VisualProgram> cylinderProgram = mProgramFactory->CreateFromFiles(pathVS, pathPS, "");
#else
    path = mEnvironment.GetPath("DrawUsingVertexID.hlsl");
    std::shared_ptr<VisualProgram> cylinderProgram = mProgramFactory->CreateFromFiles(path, path, "");
#endif
    if (!cylinderProgram)
    {
        return false;
    }
    mCylinderIDEffect = std::make_shared<VisualEffect>(cylinderProgram);

#if defined(GTE_DEV_OPENGL)
    pathVS = mEnvironment.GetPath("DrawUsingVertexIDVS.glsl");
    pathPS = mEnvironment.GetPath("DrawUsingVertexIDPS.glsl");
    std::shared_ptr<VisualProgram> torusProgram = mProgramFactory->CreateFromFiles(pathVS, pathPS, "");
#else
    path = mEnvironment.GetPath("DrawUsingVertexID.hlsl");
    std::shared_ptr<VisualProgram> torusProgram = mProgramFactory->CreateFromFiles(path, path, "");
#endif
    if (!torusProgram)
    {
        return false;
    }
    mTorusIDEffect = std::make_shared<VisualEffect>(torusProgram);

    unsigned int const numIndices0 = 3 * mNumCylinderTriangles;
    unsigned int const numIndices1 = 3 * mNumTorusTriangles;

    // Create resources for the compute programs and attach them to the
    // shaders.
    mColor0Buffer = std::make_shared<StructuredBuffer>(numIndices0, sizeof(unsigned int));
    mColor0Buffer->SetUsage(Resource::SHADER_OUTPUT);
    mColor0Buffer->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    mColor1Buffer = std::make_shared<StructuredBuffer>(numIndices1, sizeof(unsigned int));
    mColor1Buffer->SetUsage(Resource::SHADER_OUTPUT);
    mColor1Buffer->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    mTIParameters = std::make_shared<ConstantBuffer>(sizeof(TIParameters), true);
    TIParameters& data = *mTIParameters->Get<TIParameters>();
    data.wMatrix0 = Matrix4x4<float>::Identity();
    data.wMatrix1 = Matrix4x4<float>::Identity();
    data.numTriangles0 = mNumCylinderTriangles;
    data.numTriangles1 = mNumTorusTriangles;

    mVertices0 = std::make_shared<StructuredBuffer>(numIndices0, sizeof(Vector4<float>));
    Vector4<float>* data0 = mVertices0->Get<Vector4<float>>();
    Vertex* meshVertices0 = mCylinder->GetVertexBuffer()->Get<Vertex>();
    for (unsigned int i = 0; i < numIndices0; ++i)
    {
        data0[i] = HLift(meshVertices0[i].position, 1.0f);
    }

    mVertices1 = std::make_shared<StructuredBuffer>(numIndices1, sizeof(Vector4<float>));
    Vector4<float>* data1 = mVertices1->Get<Vector4<float>>();
    Vertex* meshVertices1 = mTorus->GetVertexBuffer()->Get<Vertex>();
    for (unsigned int i = 0; i < numIndices1; ++i)
    {
        data1[i] = HLift(meshVertices1[i].position, 1.0f);
    }

    mInitializeColor->GetCShader()->Set("color0", mColor0Buffer);
    mInitializeColor->GetCShader()->Set("color1", mColor1Buffer);

    mTriangleIntersection->GetCShader()->Set("Parameters", mTIParameters);
    mTriangleIntersection->GetCShader()->Set("vertices0", mVertices0);
    mTriangleIntersection->GetCShader()->Set("vertices1", mVertices1);
    mTriangleIntersection->GetCShader()->Set("color0", mColor0Buffer);
    mTriangleIntersection->GetCShader()->Set("color1", mColor1Buffer);

    // Create resources for the cylinder visual program, attach them to the
    // shaders, and create the geometric primitive.
    std::shared_ptr<ConstantBuffer> cbuffer =
        std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    std::shared_ptr<VertexShader> vshader = cylinderProgram->GetVShader();
    vshader->Set("PVWMatrix", cbuffer);
    vshader->Set("positions", mVertices0);
    vshader->Set("colorIndices", mColor0Buffer);
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32A32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, mVertices0);
    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numIndices0 / 3);
    mCylinderID = std::make_shared<Visual>(vbuffer, ibuffer, mCylinderIDEffect);

    // Create resources for the torus visual program, attach them to the
    // shaders, and create the geometric primitive.
    cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    vshader = torusProgram->GetVShader();
    vshader->Set("PVWMatrix", cbuffer);
    vshader->Set("positions", mVertices1);
    vshader->Set("colorIndices", mColor1Buffer);
    vbuffer = std::make_shared<VertexBuffer>(vformat, mVertices1);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numIndices1 / 3);
    mTorusID = std::make_shared<Visual>(vbuffer, ibuffer, mTorusIDEffect);
    return true;
}

#endif

void AllPairsTrianglesWindow::UpdateTransforms()
{
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float> pvwMatrix;
#if defined(GTE_USE_MAT_VEC)
    pvwMatrix = pvMatrix * mTrackball.GetOrientation();
#else
    pvwMatrix = mTrackball.GetOrientation() * pvMatrix;
#endif
    mCylinderPVWMatrix->SetMember("pvwMatrix", pvMatrix);
    mTorusPVWMatrix->SetMember("pvwMatrix", pvwMatrix);
    mEngine->Update(mCylinderPVWMatrix);
    mEngine->Update(mTorusPVWMatrix);

#if !defined(USE_CPU_FIND_INTERSECTIONS)
    TIParameters& data = *mTIParameters->Get<TIParameters>();
    data.wMatrix0 = Matrix4x4<float>::Identity();
    data.wMatrix1 = mTrackball.GetOrientation();
    mEngine->Update(mTIParameters);

    std::shared_ptr<ConstantBuffer> cbuffer;
    cbuffer = mCylinderIDEffect->GetVertexShader()->Get<ConstantBuffer>("PVWMatrix");
    *cbuffer->Get<Matrix4x4<float>>() = pvMatrix;
    mEngine->Update(cbuffer);
    cbuffer = mTorusIDEffect->GetVertexShader()->Get<ConstantBuffer>("PVWMatrix");
    *cbuffer->Get<Matrix4x4<float>>() = pvwMatrix;
    mEngine->Update(cbuffer);
#endif
}

void AllPairsTrianglesWindow::FindIntersections()
{
#if defined(USE_CPU_FIND_INTERSECTIONS)
    std::shared_ptr<VertexBuffer> buffer0 = mCylinder->GetVertexBuffer();
    unsigned int numVertices0 = buffer0->GetNumElements();
    unsigned int numTriangles0 = numVertices0 / 3;
    Vertex* vertices0 = buffer0->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices0; ++i)
    {
        vertices0[i].colorIndex = 0;
    }

    std::shared_ptr<VertexBuffer> buffer1 = mTorus->GetVertexBuffer();
    unsigned int numVertices1 = buffer1->GetNumElements();
    unsigned int numTriangles1 = numVertices1 / 3;
    Vertex* vertices1 = buffer1->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices1; ++i)
    {
        vertices1[i].colorIndex = 1;
    }

    TriangleIntersection intersects;
    Vector3<float> cylinder[3], torus[3];
    for (unsigned int t0 = 0; t0 < numTriangles0; ++t0)
    {
        for (int j = 0; j < 3; ++j)
        {
            cylinder[j] = vertices0[3 * t0 + j].position;
        }

        for (unsigned int t1 = 0; t1 < numTriangles1; ++t1)
        {
            for (int j = 0; j < 3; ++j)
            {
                Vector3<float> pos3 = vertices1[3 * t1 + j].position;
                Vector4<float> pos4{ pos3[0], pos3[1], pos3[2], 1.0f };
#if defined(GTE_USE_MAT_VEC)
                pos4 = mTrackball.GetOrientation() * pos4;
#else
                pos4 = pos4 * mTrackball.GetOrientation();
#endif
                pos3 = Vector3<float>{ pos4[0], pos4[1], pos4[2] };
                torus[j] = pos3;
            }

            if (intersects(cylinder, torus))
            {
                for (int j = 0; j < 3; ++j)
                {
                    vertices0[3 * t0 + j].colorIndex = 2;
                    vertices1[3 * t1 + j].colorIndex = 3;
                }
            }
        }
    }

    mEngine->Update(buffer0);
    mEngine->Update(buffer1);
#else
    mEngine->Execute(mInitializeColor, mNumXGroups, mNumYGroups, 1);
    mEngine->Execute(mTriangleIntersection, mNumXGroups, mNumYGroups, 1);
#endif
}
