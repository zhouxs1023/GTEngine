// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.1.0 (yyyy/mm/dd)

#include "WaterDropFormationWindow.h"

namespace gte
{
    template class RevolutionMesh<float>;
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

    Window::Parameters parameters(L"WaterDropFormationWindow", 0, 0, 640, 480);
    auto window = TheWindowSystem.Create<WaterDropFormationWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<WaterDropFormationWindow>(window);
    return 0;
}

WaterDropFormationWindow::WaterDropFormationWindow(Parameters& parameters)
    :
    Window3(parameters),
    mSimTime(0.0f),
    mSimDelta(0.05f),
    mLastUpdateTime(mMotionTimer.GetSeconds())
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.4f, 0.5f, 0.6f });

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

    CreateScene();

    float angle = static_cast<float>(0.01 * GTE_C_PI);
    float cs = cos(angle), sn = sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 1000.0f, 0.01f, 0.001f,
        { 21.1804028f, 0.0f, 0.665620983f }, { -cs, 0.0f, -sn }, { sn, 0.0f, -cs });

    mPVWMatrices.Update();
    mTrackball.Update();
}

void WaterDropFormationWindow::OnIdle()
{
    mTimer.Measure();

#if !defined(WATER_DROP_FORMATION_SINGLE_STEP)
    double time = mMotionTimer.GetSeconds();
    if (30.0 * (time - mLastUpdateTime) >= 1.0)
    {
        PhysicsTick();
        mLastUpdateTime = time;
    }
#endif
    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool WaterDropFormationWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mWireState == mEngine->GetRasterizerState())
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;

#if defined(WATER_DROP_FORMATION_SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool WaterDropFormationWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "StoneWall.png",
        "Water.png"
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

void WaterDropFormationWindow::CreateScene()
{
    // Vertex format shared by the ceiling, wall, and water surfaces.
    mVFormat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    mVFormat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    mScene = std::make_shared<Node>();

    CreateCeilingAndWall();
    CreateWaterRoot();
    CreateConfiguration0();

    mScene->localTransform.SetTranslation(4.0f, 0.0f, -4.0f);
    mTrackball.Attach(mScene);
    mTrackball.Update();
}

void WaterDropFormationWindow::CreateCeilingAndWall()
{
    MeshFactory mf;
    mf.SetVertexFormat(mVFormat);

    auto texture = WICFileIO::Load(mEnvironment.GetPath("StoneWall.png"), true);
    texture->AutogenerateMipmaps();

    mCeiling = mf.CreateRectangle(2, 2, 8.0f, 16.0f);
    auto vbuffer = mCeiling->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vertex* vertices = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertices[i].tcoord[1] *= 2.0f;
    }

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);
    mCeiling->SetEffect(effect);
    mPVWMatrices.Subscribe(mCeiling->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mCeiling);

    mWall = mf.CreateRectangle(2, 2, 16.0f, 8.0f);
    vbuffer = mWall->GetVertexBuffer();
    numVertices = vbuffer->GetNumElements();
    vertices = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        Vector3<float> pos = vertices[i].position;
        vertices[i].position = { pos[2] - 8.0f, pos[0], pos[1] + 8.0f };
        vertices[i].tcoord[0] *= 2.0f;
    }

    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);
    mWall->SetEffect(effect);
    mPVWMatrices.Subscribe(mWall->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mWall);
}

void WaterDropFormationWindow::CreateWaterRoot()
{
    // The texture shared by the water drop surfaces.  Modify the alpha
    // channel for transparency.
    mWaterTexture = WICFileIO::Load(mEnvironment.GetPath("Water.png"), true);
    mWaterTexture->AutogenerateMipmaps();
    unsigned int numTexels = mWaterTexture->GetNumElements();
    unsigned int* texels = mWaterTexture->Get<unsigned int>();
    for (unsigned int i = 0; i < numTexels; ++i)
    {
        texels[i] = (texels[i] & 0x00FFFFFF) | 0x80000000;
    }

    mWaterRoot = std::make_shared<Node>();
    mWaterRoot->localTransform.SetTranslation(0.0f, 0.0f, 0.1f);
    mWaterRoot->localTransform.SetUniformScale(8.0f);
    mScene->AttachChild(mWaterRoot);
}

void WaterDropFormationWindow::CreateConfiguration0()
{
    mWaterRoot->DetachChildAt(0);
    mWaterRoot->DetachChildAt(1);
    mCircle = nullptr;
    mSimTime = 0.0f;

    // Create the water surface curve of revolution.
    int const numControls = 13;
    BasisFunctionInput<float> input(numControls, 2);
    mSpline = std::make_shared<NURBSCurve<2, float>>(input, nullptr, nullptr);

    for (int i = 0; i < mSpline->GetNumControls(); ++i)
    {
        mSpline->SetControl(i, { 0.125f + 0.0625f * i, 0.0625f });
        mSpline->SetWeight(i, 1.0f);
    }

    float const modWeight = 0.3f;
    mSpline->SetWeight(3, modWeight);
    mSpline->SetWeight(5, modWeight);
    mSpline->SetWeight(7, modWeight);
    mSpline->SetWeight(9, modWeight);

    float const h = 0.5f;
    float const d = 0.0625f;
    float const extra = 0.1f;
    mTargets.resize(numControls);
    mTargets[0] = mSpline->GetControl(0);
    mTargets[1] = mSpline->GetControl(6);
    mTargets[2] = { mSpline->GetControl(6)[0], h - d - extra };
    mTargets[3] = { mSpline->GetControl(5)[0], h - d - extra };
    mTargets[4] = { mSpline->GetControl(5)[0], h };
    mTargets[5] = { mSpline->GetControl(5)[0], h + d };
    mTargets[6] = { mSpline->GetControl(6)[0], h + d };
    mTargets[7] = { mSpline->GetControl(7)[0], h + d };
    mTargets[8] = { mSpline->GetControl(7)[0], h };
    mTargets[9] = { mSpline->GetControl(7)[0], h - d - extra };
    mTargets[10] = { mSpline->GetControl(6)[0], h - d - extra };
    mTargets[11] = mSpline->GetControl(6);
    mTargets[12] = mSpline->GetControl(12);

    // Restrict evaluation to a subinterval of the domain.
    mSpline->SetTimeInterval(0.5f, 1.0f);

    // Create the water surface.
    MeshDescription desc(MeshTopology::DISK, 32, 16);
    auto vbuffer = std::make_shared<VertexBuffer>(mVFormat, desc.numVertices);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    Vertex* vertices = vbuffer->Get<Vertex>();
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(unsigned int));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());
    desc.wantCCW = false;

    mWaterSurfaceMesh = std::make_unique<RevolutionMesh<float>>(desc, mSpline, false);

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, mWaterTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);
    mWaterSurface = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mWaterSurface->localTransform.SetTranslation(mSpline->GetControl(6)[0], 0.0f, 0.0f);
    mPVWMatrices.Subscribe(mWaterSurface->worldTransform, effect->GetPVWMatrixConstant());

    mWaterRoot->AttachChild(mWaterSurface);
    mWaterRoot->Update();
}

void WaterDropFormationWindow::CreateConfiguration1()
{
    // Create the water drop curve of revolution.
    int const numControls = 16;
    int const degree = 2;
    std::vector<Vector2<float>> controls(numControls);
    std::vector<float> weights(numControls);

    controls[0] = mSpline->GetControl(0);
    controls[1] = mSpline->GetControl(1);
    controls[2] = 0.5f * (mSpline->GetControl(1) + mSpline->GetControl(2));
    controls[3] = mSpline->GetControl(11);
    controls[4] = mSpline->GetControl(12);
    for (int i = 2, j = 5; i <= 10; ++i, ++j)
    {
        controls[j] = mSpline->GetControl(i);
    }
    controls[5] = 0.5f * (controls[2] + controls[5]);
    controls[13] = controls[5];

    for (int i = 0; i < numControls; ++i)
    {
        weights[i] = 1.0f;
    }
    weights[6] = mSpline->GetWeight(3);
    weights[8] = mSpline->GetWeight(5);
    weights[10] = mSpline->GetWeight(7);
    weights[12] = mSpline->GetWeight(9);

    // Replicate the first two control-weights to obtain C1 continuity for
    // the periodic mCircle curve.
    controls[14] = controls[5];
    controls[15] = controls[6];
    weights[14] = weights[5];
    weights[15] = weights[6];

    // Create the spline and restrict evaluation to a subinterval of the
    // domain.
    BasisFunctionInput<float> input0(5, 2);
    mSpline = std::make_shared<NURBSCurve<2, float>>(input0, controls.data(), weights.data());
    mSpline->SetTimeInterval(0.5f, 1.0f);

    // Create the circle.  Restrict evaluation to a subinterval of the
    // domain.  Why 0.375?  The circle NURBS is a loop and not open.  The
    // curve is constructed with degree (2) replicated control points.
    // Although the curve is geometrically symmetric about the vertical
    // axis, it is not symmetric in t about the half way point (0.5) of
    // the domain [0,1].
    BasisFunctionInput<float> input1;
    input1.numControls = 11;
    input1.degree = 2;
    input1.uniform = true;
    input1.periodic = true;
    input1.numUniqueKnots = input1.numControls + input1.degree + 1;
    input1.uniqueKnots.resize(input1.numUniqueKnots);
    float invNmD = 1.0f / static_cast<float>(input1.numControls - input1.degree);
    for (int i = 0; i < input1.numUniqueKnots; ++i)
    {
        input1.uniqueKnots[i].t = static_cast<float>(i - input1.degree) * invNmD;
        input1.uniqueKnots[i].multiplicity = 1;
    }
    mCircle = std::make_shared<NURBSCurve<2, float>>(input1, controls.data() + 5, weights.data() + 5);
    mCircle->SetTimeInterval(0.375f, 1.0f);

    // Create the new water surface.
    MeshDescription desc(MeshTopology::DISK, 32, 16);
    auto vbuffer = std::make_shared<VertexBuffer>(mVFormat, desc.numVertices);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    Vertex* vertices = vbuffer->Get<Vertex>();
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(unsigned int));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());

    mWaterSurfaceMesh = std::make_unique<RevolutionMesh<float>>(desc, mSpline, false);

    auto effect = std::static_pointer_cast<Texture2Effect>(mWaterSurface->GetEffect());
    mPVWMatrices.Unsubscribe(mWaterSurface->worldTransform);
    mWaterSurface = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mWaterSurface->localTransform.SetTranslation(mSpline->GetControl(6)[0], 0.0f, 0.0f);
    mPVWMatrices.Subscribe(mWaterSurface->worldTransform, effect->GetPVWMatrixConstant());

    mWaterRoot->AttachChild(mWaterSurface);

    // Create water drop.  The outside view value is set to 'false' because
    // the curve (x(t),z(t)) has the property dz/dt < 0.  If the curve
    // instead had the property dz/dt > 0, then 'true' is the correct value
    // for the outside view.
    vbuffer = std::make_shared<VertexBuffer>(mVFormat, desc.numVertices);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    vertices = vbuffer->Get<Vertex>();
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(unsigned int));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());

    mWaterDropMesh = std::make_unique<RevolutionMesh<float>>(desc, mSpline, false);

    effect = std::make_shared<Texture2Effect>(mProgramFactory, mWaterTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);
    mWaterDrop = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mWaterDrop->localTransform.SetTranslation(controls[9][0], 0.0f, 0.0f);
    mPVWMatrices.Subscribe(mWaterDrop->worldTransform, effect->GetPVWMatrixConstant());

    mWaterRoot->AttachChild(mWaterDrop);
    mWaterRoot->Update();
}

void WaterDropFormationWindow::DoPhysical1()
{
    // Modify control points.
    float t = mSimTime, oneMinusT = 1.0f - t;
    float t2 = t * t, oneMinusT2 = 1.0f - t2;
    int numControls = mSpline->GetNumControls();
    for (int i = 0; i < numControls; ++i)
    {
        if (i != 4)
        {
            mSpline->SetControl(i, oneMinusT * mSpline->GetControl(i) + t * mTargets[i]);
        }
        else
        {
            mSpline->SetControl(i, oneMinusT2 * mSpline->GetControl(i) + t2 * mTargets[i]);
        }
    }

    // Modify mesh vertices.
    mWaterSurfaceMesh->Update();
    mEngine->Update(mWaterSurface->GetVertexBuffer());
    mScene->Update();
}

void WaterDropFormationWindow::DoPhysical2()
{
    if (!mCircle)
    {
        CreateConfiguration1();
    }

    mSimTime += mSimDelta;

    // Surface evolves to a disk.
    float t = mSimTime - 1.0f, oneMinusT = 1.0f - t;
    Vector2<float> control = oneMinusT * mSpline->GetControl(2) + t * mSpline->GetControl(1);
    mSpline->SetControl(2, control);

    // Sphere floats down a little bit.
    int const numControls = mCircle->GetNumControls();
    for (int i = 0; i < numControls; ++i)
    {
        control = mCircle->GetControl(i) + Vector2<float>{ 0.0f, 1.0f / 32.0f};
        mCircle->SetControl(i, control);
    }

    mWaterSurfaceMesh->Update();
    mEngine->Update(mWaterSurface->GetVertexBuffer());
    mWaterDropMesh->Update();
    mEngine->Update(mWaterDrop->GetVertexBuffer());
    mScene->Update();
}

void WaterDropFormationWindow::DoPhysical3()
{
    mSimTime += mSimDelta;

    // Sphere floats down a little bit.
    int const numControls = mCircle->GetNumControls();
    for (int i = 0; i < numControls; ++i)
    {
        Vector2<float> control = mCircle->GetControl(i);
        if (i == 0 || i == numControls - 1)
        {
            control[1] += 1.3f / 32.0f;
        }
        else
        {
            control[1] += 1.0f / 32.0f;
        }
        mCircle->SetControl(i, control);
    }

    mWaterDropMesh->Update();
    mEngine->Update(mWaterDrop->GetVertexBuffer());
    mScene->Update();
}

void WaterDropFormationWindow::PhysicsTick()
{
    mSimTime += mSimDelta;
    if (mSimTime <= 1.0f)
    {
        // Water surface extruded to form a water drop.
        DoPhysical1();
    }
    else if (mSimTime <= 2.0f)
    {
        // Water drop splits from water surface.
        DoPhysical2();
    }
    else if (mSimTime <= 4.0f)
    {
        // Water drop continues downward motion, surface no longer changes.
        DoPhysical3();
    }
    else
    {
        // Restart the animation.
        CreateConfiguration0();
    }
}

void WaterDropFormationWindow::GraphicsTick()
{
    mEngine->ClearBuffers();

    mEngine->Draw(mCeiling);
    mEngine->Draw(mWall);

    mEngine->SetBlendState(mBlendState);
    mEngine->Draw(mWaterSurface);
    if (mCircle)
    {
        mEngine->Draw(mWaterDrop);
    }
    mEngine->SetDefaultBlendState();

    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::string message = "time = " + std::to_string(mSimTime);
    mEngine->Draw(96, mYSize - 8, textColor, message);
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());

    mEngine->DisplayColorBuffer(0);
}
