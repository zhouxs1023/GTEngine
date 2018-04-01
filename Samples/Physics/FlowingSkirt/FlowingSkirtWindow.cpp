// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/08/25)

#include "FlowingSkirtWindow.h"

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

    Window::Parameters parameters(L"FlowingSkirtWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<FlowingSkirtWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<FlowingSkirtWindow>(window);
    return 0;
}

FlowingSkirtWindow::FlowingSkirtWindow(Parameters& parameters)
    :
    Window3(parameters),
    mNumCtrl(32),
    mDegree(3),
    mATop(1.0f),
    mBTop(1.5f),
    mABottom(2.0f),
    mBBottom(3.0f),
    mFrequencies(mNumCtrl)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;
    mWireNoCullState = std::make_shared<RasterizerState>();
    mWireNoCullState->cullMode = RasterizerState::CULL_NONE;
    mWireNoCullState->fillMode = RasterizerState::FILL_WIREFRAME;
    mEngine->SetRasterizerState(mNoCullState);

    CreateScene();

    // Center-and-fit for camera viewing.
    mScene->Update();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.005f, 0.01f,
        { 0.0f, 0.0f, -2.5f * mScene->worldBound.GetRadius() },
        { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mSkirt->localTransform.SetTranslation(-mScene->worldBound.GetCenter());
    mSkirt->Update();
    mPVWMatrices.Update();
}

void FlowingSkirtWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(SINGLE_STEP)
    ModifyCurves();
#endif

    mEngine->ClearBuffers();
    mEngine->Draw(mSkirt);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool FlowingSkirtWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
    case 'W':
        if (mWireNoCullState == mEngine->GetRasterizerState())
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
        ModifyCurves();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool FlowingSkirtWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Flower.png") == "")
    {
        LogError("Cannot find file Flower.png.");
        return false;
    }

    return true;
}

void FlowingSkirtWindow::CreateScene()
{
    mScene = std::make_shared<Node>();

    // The skirt top and bottom boundary curves are chosen to be periodic,
    // looped B-spline curves.  The top control points are generated on an
    // ellipse (x/a0)^2 + (z/b0)^2 = 1 with y = 4.  The bottom control points
    // are generated on an ellipse (x/a1)^2 + (z/b1)^2 = 1 with y = 0.

    // The vertex storage is used for the B-spline control points.  The
    // curve objects make a copy of the input points.  The vertex storage is
    // then used for the skirt mesh vertices themselves.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    // Use random numbers for the frequencies.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.5f, 1.0f);

    unsigned int numVertices = 2 * mNumCtrl;
    std::vector<Vector3<float>> positions(numVertices);
    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    Vertex* vertices = vbuffer->Get<Vertex>();
    int i, j;
    for (i = 0, j = mNumCtrl; i < mNumCtrl; ++i, ++j)
    {
        float ratio = static_cast<float>(i) / static_cast<float>(mNumCtrl);
        float angle = ratio * static_cast<float>(GTE_C_TWO_PI);
        float sn = sin(angle);
        float cs = cos(angle);
        float v = 1.0f - fabs(2.0f * ratio - 1.0f);

        // Set a vertex for the skirt top.
        positions[i] = { mATop * cs, 4.0f, mBTop * sn };
        vertices[i].position = positions[i];
        vertices[i].tcoord = { 1.0f, v };

        // Set a vertex for the skirt bottom.
        positions[j] = { mABottom * cs, 0.0f, mBBottom * sn };
        vertices[j].position = positions[j];
        vertices[j].tcoord = { 0.0f, v };

        // Frequency of sinusoidal motion for skirt bottom.
        mFrequencies[i] = rnd(mte);
    }

    // The control points are copied by the curve objects.
    BasisFunctionInput<float> bfInput;
    bfInput.numControls = mNumCtrl;
    bfInput.degree = mDegree;
    bfInput.uniform = false;
    bfInput.periodic = true;
    bfInput.numUniqueKnots = mNumCtrl + mDegree + 1;
    bfInput.uniqueKnots.resize(bfInput.numUniqueKnots);
    float invNmD = 1.0f / static_cast<float>(mNumCtrl - mDegree);
    for (i = 0; i < bfInput.numUniqueKnots; ++i)
    {
        bfInput.uniqueKnots[i].t = static_cast<float>(i - mDegree) * invNmD;
        bfInput.uniqueKnots[i].multiplicity = 1;
    }
    mSkirtTop = std::make_unique<BSplineCurve<3, float>>(bfInput, positions.data());
    mSkirtBottom = std::make_unique<BSplineCurve<3, float>>(bfInput, positions.data() + mNumCtrl);

    // Generate the triangle connectivity (cylinder connectivity).
    unsigned int numTriangles = numVertices;
    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(
        IP_TRIMESH, numTriangles, sizeof(unsigned int));
    unsigned int* indices = ibuffer->Get<unsigned int>();
    int i0 = 0, i1 = 1, i2 = mNumCtrl, i3 = mNumCtrl + 1;
    for (i = 0; i1 < mNumCtrl; i0 = i1++, i2 = i3++)
    {
        indices[i++] = i0;
        indices[i++] = i1;
        indices[i++] = i3;
        indices[i++] = i0;
        indices[i++] = i3;
        indices[i++] = i2;
    }
    indices[i++] = mNumCtrl - 1;
    indices[i++] = 0;
    indices[i++] = mNumCtrl;
    indices[i++] = mNumCtrl - 1;
    indices[i++] = mNumCtrl;
    indices[i++] = 2 * mNumCtrl - 1;

    std::string path = mEnvironment.GetPath("Flower.png");
    std::shared_ptr<Texture2> texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(
        mProgramFactory, texture, SamplerState::MIN_L_MAG_L_MIP_L,
        SamplerState::CLAMP, SamplerState::CLAMP);

    mSkirt = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSkirt->UpdateModelBound();
    mPVWMatrices.Subscribe(mSkirt->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mSkirt);
    mTrackball.Attach(mScene);

    // Compute the vertex values for the current B-spline curves.
    UpdateSkirt();
}

void FlowingSkirtWindow::UpdateSkirt()
{
    std::shared_ptr<VertexBuffer> vbuffer = mSkirt->GetVertexBuffer();
    Vertex* vertices = vbuffer->Get<Vertex>();
    for (int i = 0, j = mNumCtrl; i < mNumCtrl; ++i, ++j)
    {
        float t = static_cast<float>(i) / static_cast<float>(mNumCtrl);
        Vector3<float> values[4];
        mSkirtTop->Evaluate(t, 0, values);
        vertices[i].position = values[0];
        mSkirtBottom->Evaluate(t, 0, values);
        vertices[j].position = values[0];
    }

    mSkirt->Update();
    mPVWMatrices.Update();
    mEngine->Update(vbuffer);
}

void FlowingSkirtWindow::ModifyCurves()
{
    // Perturb the skirt bottom.
    float time = static_cast<float>(mAnimTimer.GetSeconds());
    for (int i = 0; i < mNumCtrl; ++i)
    {
        float ratio = static_cast<float>(i) / static_cast<float>(mNumCtrl);
        float angle = ratio * static_cast<float>(GTE_C_TWO_PI);
        float sn = sin(angle);
        float cs = cos(angle);

        float amplitude = 1.0f + 0.25f * cos(mFrequencies[i] * time);
        Vector3<float> ctrl{ amplitude * mABottom * cs, 0.0f, amplitude * mBBottom * sn };
        mSkirtBottom->SetControl(i, ctrl);
    }

    UpdateSkirt();
}
