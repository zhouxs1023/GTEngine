// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.3.1 (2017/06/18)

#include "HelixTubeSurfaceWindow.h"

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

    Window::Parameters parameters(L"HelixTubeSurfaceWindow", 0, 0, 640, 480);
    auto window = TheWindowSystem.Create<HelixTubeSurfaceWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<HelixTubeSurfaceWindow>(window);
    return 0;
}

HelixTubeSurfaceWindow::HelixTubeSurfaceWindow(Parameters& parameters)
    :
    Window3(parameters),
    mMinCurveTime(0.0f),
    mMaxCurveTime(0.0f),
    mCurvePeriod(0.0f),
    mCurveTime(0.0f),
    mDeltaTime(0.0f)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    CreateScene();

    // Disable the default camera rig and work directly with the camera.
    // The coordinate frame will be set by the MoveCamera() member function.
    mCameraRig.ClearMotions();
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.01f, 10.0f);

    MoveCamera(mMinCurveTime);
    mPVWMatrices.Update();
}

void HelixTubeSurfaceWindow::OnIdle()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    mEngine->Draw(mHelixTube);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool HelixTubeSurfaceWindow::OnCharPress(unsigned char key, int x, int y)
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
    case '+':
    case '=':
        mDeltaTime *= 2.0f;
        return true;
    case '-':
    case '_':
        mDeltaTime *= 0.5f;
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool HelixTubeSurfaceWindow::OnKeyDown(int key, int x, int y)
{
    if (key == KEY_UP)
    {
        mCurveTime += mDeltaTime;
        if (mCurveTime > mMaxCurveTime)
        {
            mCurveTime -= mCurvePeriod;
        }
        MoveCamera(mCurveTime);
        return true;
    }

    if (key == KEY_DOWN)
    {
        mCurveTime -= mDeltaTime;
        if (mCurveTime < mMinCurveTime)
        {
            mCurveTime += mCurvePeriod;
        }
        MoveCamera(mCurveTime);
        return true;
    }

    return Window3::OnKeyDown(key, x, y);
}

bool HelixTubeSurfaceWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Grating.png") == "")
    {
        LogError("Cannot find file Grating.png");
        return false;
    }

    return true;
}

void HelixTubeSurfaceWindow::CreateScene()
{
    MeshDescription desc(MeshTopology::CYLINDER, 256, 32);
    desc.wantCCW = false;

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, desc.numVertices);
    Vertex* vertices = vbuffer->Get<Vertex>();
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(unsigned int));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());

    CreateCurve();
    TubeMesh<float> surface(desc, mMedial, [](float) { return 0.0625f; },
        false, false, { 0.0f, 0.0f, 1.0f });

    // The texture coordinates are in [0,1]^2.  Allow the texture to repeat
    // in the direction along the medial curve.
    for (unsigned int i = 0; i < vbuffer->GetNumElements(); ++i)
    {
        vertices[i].tcoord[1] *= 32.0f;
    }

    auto texture = WICFileIO::Load(mEnvironment.GetPath("Grating.png"), false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::WRAP, SamplerState::WRAP);

    mHelixTube = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mHelixTube->Update();
    mPVWMatrices.Subscribe(mHelixTube->worldTransform, effect->GetPVWMatrixConstant());
}

void HelixTubeSurfaceWindow::CreateCurve()
{
    // Sample points on a looped helix (first and last point must match).
    float const fourPi = static_cast<float>(2.0 * GTE_C_TWO_PI);
    int const numSegments = 32;
    int const numSegmentsP1 = numSegments + 1;
    float const invNumSegments = 1.0f / static_cast<float>(numSegments);
    float const invNumSegmentsP1 = 1.0f / static_cast<float>(numSegmentsP1);
    std::vector<float> times(numSegmentsP1);
    std::vector<Vector3<float>> points(numSegmentsP1);
    float t;
    int i;

    for (i = 0; i <= numSegmentsP1 / 2; ++i)
    {
        t = i * fourPi * invNumSegmentsP1;
        times[i] = t;
        points[i] = { (float)cos(t), (float)sin(t), t };
    }
    for (/**/; i < numSegments; ++i)
    {
        t = i * fourPi * invNumSegments;
        times[i] = t;
        points[i] = { 2.0f - (float)cos(t), (float)sin(t), fourPi - t };
    }

    times[numSegments] = fourPi;
    points[numSegments] = points[0];

    // Save min and max times.
    mMinCurveTime = 0.0f;
    mMaxCurveTime = fourPi;
    mCurvePeriod = mMaxCurveTime - mMinCurveTime;
    mCurveTime = mMinCurveTime;
    mDeltaTime = 0.01f;

    // Create a closed cubic curve containing the sample points.
    mMedial = std::make_shared<NaturalSplineCurve<3, float>>(false,
        numSegmentsP1, points.data(), times.data());
}

void HelixTubeSurfaceWindow::MoveCamera(float time)
{
    Vector3<float> values[4];
    mMedial->Evaluate(time, 1, values);
    Vector4<float> position = HLift(values[0], 1.0f);
    Vector4<float> tangent = HLift(values[1], 0.0f);
    Vector4<float> binormal = UnitCross(tangent, Vector4<float>({ 0.0f, 0.0f, 1.0f, 0.0f }));
    Vector4<float> normal = UnitCross(binormal, tangent);
    binormal -= Dot(binormal, normal) * normal;
    Normalize(binormal);
    tangent = Cross(normal, binormal);
    mCamera->SetFrame(position, tangent, normal, binormal);
    mPVWMatrices.Update();
}
