// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.6.0 (2016/12/09)

#include "ContourEdgesWindow.h"

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

    Window::Parameters parameters(L"ContourEdgesWindow", 0, 0, 1024, 1024);
    auto window = TheWindowSystem.Create<ContourEdgesWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<ContourEdgesWindow>(window);
    return 0;
}

ContourEdgesWindow::ContourEdgesWindow(Parameters& parameters)
    :
    Window3(parameters),
#if !defined(DRAW_STANDARD_MESH)
    mLineThickness(3.0f),
#endif
    mUseWireState(false)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
    { 0.0f, -7.0f, 1.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    mTrackball.Update();
    mPVWMatrices.Update();
}

void ContourEdgesWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(DRAW_STANDARD_MESH)
    mBackground->SetEffect(mBackgroundFinderEffect);
    mTerrain->SetEffect(mTerrainFinderEffect);
    mEngine->Enable(mFinderTarget);
    mEngine->SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
    mEngine->ClearBuffers();
    mEngine->Draw(mBackground);
    mEngine->Draw(mTerrain);
    mEngine->Disable(mFinderTarget);
#endif

#if !defined(DRAW_STANDARD_MESH)
    mBackground->SetEffect(mBackgroundDrawerEffect);
    mTerrain->SetEffect(mTerrainDrawerEffect);
    mEngine->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
#endif
    mEngine->ClearBuffers();
    if (mUseWireState)
    {
        mEngine->SetRasterizerState(mWireState);
    }
    mEngine->Draw(mBackground);
    mEngine->Draw(mTerrain);
    if (mUseWireState)
    {
        mEngine->SetDefaultRasterizerState();
    }

    //mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ContourEdgesWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        mUseWireState = !mUseWireState;
        return true;

#if !defined(DRAW_STANDARD_MESH)
    case 'c':
    case 'C':
    {
        auto texture = mFinderTarget->GetRTTexture(0);
        mEngine->CopyGpuToCpu(texture);
        std::array<uint32_t, 2>* texels;
        texels = texture->Get<std::array<uint32_t, 2>>();
        auto saveTexture = std::make_shared<Texture2>(DF_R8_UNORM, mXSize, mYSize);
        uint8_t* target = saveTexture->Get<uint8_t>();
        memset(target, 0, saveTexture->GetNumBytes());
        for (uint32_t i = 0; i < texture->GetNumElements(); ++i)
        {
            if (texels[i][0] != 0)
            {
                target[i] = 255;
            }
        }
        WICFileIO::SaveToPNG("Mask.png", saveTexture);
        return true;
    }

    case 's':
    case 'S':
    {
        // Save the camera state and world-view-projection matrix for
        // use in a CPU-based implementation diagnose numerical issues
        // with the contour edge finder.
        std::ofstream output("pvwmatrix.binary", std::ofstream::binary);
        auto wMatrix = mBackground->worldTransform.GetHMatrix();
        auto vMatrix = mCamera->GetViewMatrix();
        auto pMatrix = mCamera->GetProjectionMatrix();
        auto camPosition = mCamera->GetPosition();
        auto camDVector = mCamera->GetDVector();
        auto camUVector = mCamera->GetUVector();
        auto camRVector = mCamera->GetRVector();
        auto pvwMatrix = *mBackgroundPVWMatrix->Get<Matrix4x4<float>>();
        output.write((char const*)&pvwMatrix, sizeof(pvwMatrix));
        output.write((char const*)&wMatrix, sizeof(wMatrix));
        output.write((char const*)&vMatrix, sizeof(vMatrix));
        output.write((char const*)&pMatrix, sizeof(pMatrix));
        output.write((char const*)&camPosition, sizeof(camPosition));
        output.write((char const*)&camDVector, sizeof(camDVector));
        output.write((char const*)&camUVector, sizeof(camUVector));
        output.write((char const*)&camRVector, sizeof(camRVector));
        output.close();
        return true;
    }
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool ContourEdgesWindow::SetEnvironment()
{
    std::string gtePath = GetGTEPath();
    if (gtePath == "")
    {
        return false;
    }

#if _MSC_VER < 1900
    std::string csoPath = "_Output\\v120\\";
#else
    std::string csoPath = "_Output\\v140\\";
#endif
#if defined(ARCH_WIN32)
    csoPath += "Win32/";
#else
    csoPath += "x64/";
#endif

#if defined(_DEBUG)
    csoPath += "Debug/";
#else
    csoPath += "Release/";
#endif

    mEnvironment.Insert(gtePath + "/Samples/Graphics/ContourEdges/" + csoPath);
    mEnvironment.Insert(gtePath + "/Samples/Data/");
    mEnvironment.Insert(gtePath + "/Samples/Graphics/ContourEdges/Shaders/");
    std::vector<std::string> inputs =
    {
        "BTHeightField.png",
        "BTStone.png",
        "ContourDrawer.hlsl",
        "ContourFinder.hlsl",
        "ContourDrawer_vs.cso",
        "ContourDrawer_ps.cso",
        "ContourDrawer_gs.cso",
        "ContourFinder_vs.cso",
        "ContourFinder_ps.cso",
        "ContourFinder_gs.cso"
    };

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

bool ContourEdgesWindow::CreateScene()
{
#if !defined(DRAW_STANDARD_MESH)
    mFinderTarget = std::make_shared<DrawTarget>(1, DF_R32G32B32A32_UINT, mXSize, mYSize,
        false, true, DF_D24_UNORM_S8_UINT, false);
    mFinderTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    memset(mFinderTarget->GetRTTexture(0)->GetData(), 0,
        mFinderTarget->GetRTTexture(0)->GetNumBytes());
#endif
    return CreateBackground() && CreateTerrain();
}

bool ContourEdgesWindow::CreateTerrain()
{
    // Create the height field for terrain using heights from a gray-scale
    // bitmap image.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    std::string path = mEnvironment.GetPath("BTHeightField.png");
    auto heightTexture = WICFileIO::Load(path, false);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mTerrain = mf.CreateRectangle(heightTexture->GetWidth(), heightTexture->GetHeight(), 8.0f, 8.0f);
    mTrackball.Attach(mTerrain);

    // The mesh factory creates a flat height field.  Use the height-field
    // image to generate the heights and use a random number generator to
    // perturb them, just to add some noise.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    std::shared_ptr<VertexBuffer> vbuffer = mTerrain->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    unsigned char* heights = heightTexture->Get<unsigned char>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        float height = static_cast<float>(heights[4 * i]) / 255.0f;
        float perturb = 0.05f * rnd(mte);
        vertex[i].position[2] = 3.0f * height + perturb;
    }

    //std::ofstream output("TerrainVBuffer_P3T2_64x64.binary", std::ofstream::binary);
    //output.write((char const*)vbuffer->GetData(), vbuffer->GetNumElements() * sizeof(Vertex));
    //output.close();

    path = mEnvironment.GetPath("BTStone.png");
    auto baseTexture = WICFileIO::Load(path, true);
    baseTexture->AutogenerateMipmaps();

#if defined(DRAW_STANDARD_MESH)
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, baseTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);
    mTerrain->SetEffect(effect);
    mPVWMatrices.Subscribe(mTerrain->worldTransform, effect->GetPVWMatrixConstant());
#else
    // Replace the vertex buffer with a structured buffer.
    auto sbuffer = std::make_shared<StructuredBuffer>(numVertices, sizeof(Vertex));
    memcpy(sbuffer->GetData(), vbuffer->GetData(), sbuffer->GetNumBytes());
    vbuffer = std::make_shared<VertexBuffer>(vformat, sbuffer);
    mTerrain->SetVertexBuffer(vbuffer);

    // Compute the triangle-adjacent index buffer and then use it instead of
    // the original index buffer.  Although the height field has a regular
    // pattern that allows you to create the triangle-adjacent buffer directly,
    // the code here is general and will handle any manifold mesh.
    auto ibuffer = mTerrain->GetIndexBuffer();
    unsigned int numTriangles = ibuffer->GetNumPrimitives();
    int* indices = ibuffer->Get<int>();
    ETManifoldMesh mesh;
    for (unsigned int t = 0; t < numTriangles; ++t, indices += 3)
    {
        mesh.Insert(indices[0], indices[1], indices[2]);
    }

    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH_ADJ, numTriangles, sizeof(int));
    indices = ibuffer->Get<int>();  // 6-tuple per primitive
    std::array<int, 3> k0 = { 0, 1, 2 }, k1 = { 1, 2, 0 }, k2 = { 2, 0, 1 };
    for (auto const& element : mesh.GetTriangles())
    {
        // Get the center triangle of the triangle-adjacent primitive.
        auto tri = element.second;
        indices[0] = tri->V[0];
        indices[2] = tri->V[1];
        indices[4] = tri->V[2];

        // Get the adjacent triangles.
        for (int i = 0; i < 3; ++i)
        {
            auto adj = tri->T[i].lock();
            if (adj)
            {
                int j;
                for (j = 0; j < 3; ++j)
                {
                    if (adj->V[k0[j]] == tri->V[k1[i]] && adj->V[k1[j]] == tri->V[k0[i]])
                    {
                        indices[2 * i + 1] = adj->V[k2[j]];
                        break;
                    }
                }
            }
            else
            {
                indices[2 * i + 1] = indices[2 * i];
            }
        }

        indices += 6;
    }
    mTerrain->SetIndexBuffer(ibuffer);

    //std::ofstream output("TerrainIndices_64x64_6tuple.binary", std::ofstream::binary);
    //output.write((char const*)ibuffer->GetData(), ibuffer->GetNumElements() * 4);
    //output.close();

    mTerrainID = std::make_shared<ConstantBuffer>(sizeof(uint32_t), false);
    *mTerrainID->Get<uint32_t>() = 2;

    mTerrainPVWMatrix = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    *mTerrainPVWMatrix->Get<Matrix4x4<float>>() = Matrix4x4<float>::Identity();
    std::vector<MemberLayout> layout(1);
    layout[0].name = "pvwMatrix";
    layout[0].numElements = 0;
    layout[0].offset = 0;
    mTerrainPVWMatrix->SetLayout(layout);
    mPVWMatrices.Subscribe(mTerrain->worldTransform, mTerrainPVWMatrix);

    std::string vsPath = mEnvironment.GetPath("ContourFinder_vs.cso");
    std::string psPath = mEnvironment.GetPath("ContourFinder_ps.cso");
    std::string gsPath = mEnvironment.GetPath("ContourFinder_gs.cso");
    mTerrainFinderEffect = std::make_shared<ContourFinderEffect>(mProgramFactory,
        vsPath, psPath, gsPath, mTerrainID, mXSize, mYSize, mLineThickness,
        mTerrainPVWMatrix, sbuffer);
    if (!mTerrainFinderEffect->GetProgram())
    {
        return false;
    }

    auto baseSampler = std::make_shared<SamplerState>();
    baseSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    baseSampler->mode[0] = SamplerState::CLAMP;
    baseSampler->mode[1] = SamplerState::CLAMP;

    vsPath = mEnvironment.GetPath("ContourDrawer_vs.cso");
    psPath = mEnvironment.GetPath("ContourDrawer_ps.cso");
    gsPath = mEnvironment.GetPath("ContourDrawer_gs.cso");
    mTerrainDrawerEffect = std::make_shared<ContourDrawerEffect>(mProgramFactory,
        vsPath, psPath, gsPath, mTerrainID, mTerrainPVWMatrix, sbuffer,
        mFinderTarget->GetRTTexture(0), baseTexture, baseSampler);
    if (!mTerrainDrawerEffect->GetProgram())
    {
        return false;
    }
#endif
    return true;
}

bool ContourEdgesWindow::CreateBackground()
{
    // Create the background as a rectangle with a single-colored texture.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    uint32_t numVertices = 4;
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    Vertex* vertices = vbuffer->Get<Vertex>();
    float const extent = 100.0f;
    float const center = 50.0f;
    vertices[0].position = { -extent, center, -extent };
    vertices[0].tcoord = { 0.0f, 0.0f };
    vertices[1].position = { +extent, center, -extent };
    vertices[1].tcoord = { 1.0f, 0.0f };
    vertices[2].position = { -extent, center, +extent };
    vertices[2].tcoord = { 0.0f, 1.0f };
    vertices[3].position = { +extent, center, +extent };
    vertices[3].tcoord = { 1.0f, 1.0f };

    auto baseTexture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, 1, 1);
    uint8_t* texel = baseTexture->Get<uint8_t>();
    texel[0] = 134;
    texel[1] = 189;
    texel[2] = 212;
    texel[3] = 255;

#if defined(DRAW_STANDARD_MESH)
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRISTRIP, 2);

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, baseTexture,
        SamplerState::MIN_P_MAG_P_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP);

    mBackground = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mBackground->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mBackground);
#else
    auto sbuffer = std::make_shared<StructuredBuffer>(numVertices, sizeof(Vertex));
    memcpy(sbuffer->GetData(), vbuffer->GetData(), sbuffer->GetNumBytes());
    vbuffer = std::make_shared<VertexBuffer>(vformat, sbuffer);

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH_ADJ, 2, sizeof(uint32_t));
    uint32_t* I = ibuffer->Get<uint32_t>();
    I[0] = 0;  I[1] = 1;  I[2] = 1;  I[3] = 1;  I[4] = 3;  I[5] = 2;
    I[6] = 3;  I[7] = 2;  I[8] = 2;  I[9] = 2;  I[10] = 0;  I[11] = 1;

    mBackground = std::make_shared<Visual>(vbuffer, ibuffer);
    mTrackball.Attach(mBackground);

    mBackgroundID = std::make_shared<ConstantBuffer>(sizeof(uint32_t), false);
    *mBackgroundID->Get<uint32_t>() = 1;

    mBackgroundPVWMatrix = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    *mBackgroundPVWMatrix->Get<Matrix4x4<float>>() = Matrix4x4<float>::Identity();
    std::vector<MemberLayout> layout(1);
    layout[0].name = "pvwMatrix";
    layout[0].numElements = 0;
    layout[0].offset = 0;
    mBackgroundPVWMatrix->SetLayout(layout);
    mPVWMatrices.Subscribe(mBackground->worldTransform, mBackgroundPVWMatrix);

    std::string vsPath = mEnvironment.GetPath("ContourFinder_vs.cso");
    std::string psPath = mEnvironment.GetPath("ContourFinder_ps.cso");
    std::string gsPath = mEnvironment.GetPath("ContourFinder_gs.cso");
    mBackgroundFinderEffect = std::make_shared<ContourFinderEffect>(mProgramFactory,
        vsPath, psPath, gsPath, mBackgroundID, mXSize, mYSize, mLineThickness,
        mBackgroundPVWMatrix, sbuffer);
    if (!mBackgroundFinderEffect->GetProgram())
    {
        return false;
    }

    auto baseSampler = std::make_shared<SamplerState>();
    baseSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    baseSampler->mode[0] = SamplerState::CLAMP;
    baseSampler->mode[1] = SamplerState::CLAMP;

    vsPath = mEnvironment.GetPath("ContourDrawer_vs.cso");
    psPath = mEnvironment.GetPath("ContourDrawer_ps.cso");
    gsPath = mEnvironment.GetPath("ContourDrawer_gs.cso");
    mBackgroundDrawerEffect = std::make_shared<ContourDrawerEffect>(mProgramFactory,
        vsPath, psPath, gsPath, mBackgroundID, mBackgroundPVWMatrix, sbuffer,
        mFinderTarget->GetRTTexture(0), baseTexture, baseSampler);
    if (!mBackgroundDrawerEffect->GetProgram())
    {
        return false;
    }
#endif
    return true;
}
