// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.3 (2016/08/29)

#include "ClothWindow.h"

//#define SINGLE_STEP

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

    Window::Parameters parameters(L"ClothWindow", 0, 0, 800, 600);
    auto window = TheWindowSystem.Create<ClothWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<ClothWindow>(window);
    return 0;
}

ClothWindow::ClothWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;
    mEngine->SetRasterizerState(mNoCullState);

    mWireNoCullState = std::make_shared<RasterizerState>();
    mWireNoCullState->fillMode = RasterizerState::FILL_WIREFRAME;
    mWireNoCullState->cullMode = RasterizerState::CULL_NONE;

    mEngine->SetClearColor({ 0.85f, 0.85f, 1.0f, 1.0f });

    CreateSprings();
    CreateCloth();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.01f,
        { 0.0f, -1.75f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();

    mTime0 = std::chrono::high_resolution_clock::now();
}

void ClothWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(SINGLE_STEP)
    PhysicsTick();
#endif

    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool ClothWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
    case 'W':
        if (mNoCullState != mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        else
        {
            mEngine->SetRasterizerState(mWireNoCullState);
        }
        return true;

#if defined(SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window::OnCharPress(key, x, y);
}

bool ClothWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Cloth.png") == "")
    {
        LogError("Cannot find file Cloth.png.");
        return false;
    }

    return true;
}

void ClothWindow::CreateSprings()
{
    // Set up the mass-spring system.
    int numRows = 8;
    int numCols = 16;
    float step = 0.01f;
    Vector3<float> gravity{ 0.0f, 0.0f, -1.0f };
    Vector3<float> wind{ 0.5f, 0.0f, 0.0f };
    float viscosity = 10.0f;
    float maxAmplitude = 2.0f;
    mModule = std::make_unique<PhysicsModule>(numRows, numCols, step, gravity,
        wind, viscosity, maxAmplitude);

    // The top r of the mesh is immovable (infinite mass).  All other masses
    // are constant.
    int r, c;
    for (c = 0; c < numCols; ++c)
    {
        mModule->SetMass(numRows - 1, c, std::numeric_limits<float>::max());
    }
    for (r = 0; r < numRows - 1; ++r)
    {
        for (c = 0; c < numCols; ++c)
        {
            mModule->SetMass(r, c, 1.0f);
        }
    }

    // Initial position on a vertical axis-aligned rectangle, zero velocity.
    float rowFactor = 1.0f / (float)(numRows - 1);
    float colFactor = 1.0f / (float)(numCols - 1);
    for (r = 0; r < numRows; ++r)
    {
        for (c = 0; c < numCols; ++c)
        {
            float x = c*colFactor;
            float z = r*rowFactor;
            mModule->SetPosition(r, c, { x, 0.0f, z });
            mModule->SetVelocity(r, c, { 0.0f, 0.0f, 0.0f });
        }
    }

    // Springs are at rest in the initial configuration.
    float rowConstant = 1000.0f;
    float bottomConstant = 100.0f;
    Vector3<float> diff;
    for (r = 0; r < numRows; ++r)
    {
        for (c = 0; c < numCols - 1; ++c)
        {
            mModule->SetConstantC(r, c, rowConstant);
            diff = mModule->GetPosition(r, c + 1) -
                mModule->GetPosition(r, c);
            mModule->SetLengthC(r, c, Length(diff));
        }
    }

    for (r = 0; r < numRows - 1; ++r)
    {
        for (c = 0; c < numCols; ++c)
        {
            mModule->SetConstantR(r, c, bottomConstant);
            diff = mModule->GetPosition(r, c) -
                mModule->GetPosition(r + 1, c);
            mModule->SetLengthR(r, c, Length(diff));
        }
    }
}

void ClothWindow::CreateCloth()
{
    MeshDescription desc(MeshTopology::RECTANGLE, 16, 32);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, desc.numVertices);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    Vertex* vertices = vbuffer->Get<Vertex>();
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(unsigned int));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());

    BasisFunctionInput<float> input[2] =
    {
        BasisFunctionInput<float>(mModule->GetNumRows(), 2),
        BasisFunctionInput<float>(mModule->GetNumCols(), 2)
    };
    mSpline = std::make_shared<BSplineSurface<3, float>>(input, &mModule->GetPosition(0, 0));
    mSurface = std::make_unique<RectanglePatchMesh<float>>(desc, mSpline);

    std::string path = mEnvironment.GetPath("Cloth.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);

    mCloth = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mCloth->UpdateModelBound();
    mCloth->localTransform.SetTranslation(-mCloth->modelBound.GetCenter());
    mPVWMatrices.Subscribe(mCloth->worldTransform, effect->GetPVWMatrixConstant());

    mTrackball.Attach(mCloth);
    mTrackball.Update();
}

void ClothWindow::PhysicsTick()
{
    mTime1 = std::chrono::high_resolution_clock::now();
    int64_t delta = std::chrono::duration_cast<std::chrono::milliseconds>(
        mTime1 - mTime0).count();

    mModule->Update(static_cast<float>(delta) / 1000.0f);

    // Update spline surface.  Remember that the spline maintains its own
    // copy of the control points, so this update is necessary.
    for (int r = 0; r < mModule->GetNumRows(); ++r)
    {
        for (int c = 0; c < mModule->GetNumCols(); ++c)
        {
            mSpline->SetControl(r, c, mModule->GetPosition(r, c));
        }
    }

    // Update the GPU copy of the vertices.
    mSurface->Update();
    mEngine->Update(mCloth->GetVertexBuffer());
    mPVWMatrices.Update();
}

void ClothWindow::GraphicsTick()
{
    mEngine->ClearBuffers();
    mEngine->Draw(mCloth);
    mEngine->DisplayColorBuffer(0);
}
