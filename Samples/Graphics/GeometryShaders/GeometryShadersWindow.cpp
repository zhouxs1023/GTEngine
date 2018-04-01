// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "GeometryShadersWindow.h"

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

    Window::Parameters parameters(L"GeometryShadersWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<GeometryShadersWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<GeometryShadersWindow>(window);
    return 0;
}

GeometryShadersWindow::GeometryShadersWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 2.8f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize, mYSize);
    mTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);
#endif
}

void GeometryShadersWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

#if defined(SAVE_RENDERING_TO_DISK)
    mEngine->Enable(mTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Disable(mTarget);
    mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
    WICFileIO::SaveToPNG("GeometryShaders.png", mTarget->GetRTTexture(0));
#endif

    mTimer.UpdateFrameCount();
}

bool GeometryShadersWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/GeometryShaders/Shaders/");
    std::vector<std::string> inputs =
    {
#if defined(GTE_DEV_OPENGL)
        "RandomSquaresDirectVS.glsl",
        "RandomSquaresIndirectVS.glsl",
        "RandomSquaresDirectGS.glsl",
        "RandomSquaresIndirectGS.glsl",
        "RandomSquaresPS.glsl"
#else
        "RandomSquares.hlsl",
        "RandomSquaresIndirect.hlsl"
#endif
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

bool GeometryShadersWindow::CreateScene()
{
#if defined(GTE_DEV_OPENGL)
#if defined(USE_DRAW_DIRECT)
    std::shared_ptr<VisualProgram> program =
        mProgramFactory->CreateFromFiles(
            mEnvironment.GetPath("RandomSquaresDirectVS.glsl"),
            mEnvironment.GetPath("RandomSquaresPS.glsl"),
            mEnvironment.GetPath("RandomSquaresDirectGS.glsl"));
#else
    std::shared_ptr<VisualProgram> program =
        mProgramFactory->CreateFromFiles(
            mEnvironment.GetPath("RandomSquaresIndirectVS.glsl"),
            mEnvironment.GetPath("RandomSquaresPS.glsl"),
            mEnvironment.GetPath("RandomSquaresIndirectGS.glsl"));
#endif
#else
    std::string filename;
#if defined(USE_DRAW_DIRECT)
    filename = mEnvironment.GetPath("RandomSquares.hlsl");
#else
    filename = mEnvironment.GetPath("RandomSquaresIndirect.hlsl");
#endif
    std::shared_ptr<VisualProgram> program =
        mProgramFactory->CreateFromFiles(filename, filename, filename);
#endif
    if (!program)
    {
        return false;
    }

    // Create particles used by direct and indirect drawing.
#if defined(GTE_DEV_OPENGL) && !defined(USE_DRAW_DIRECT)
    struct Vertex
    {
        Vector3<float> position;

        // std430 does tight packing on arrays but for three-component
        // vectors, according to the "red book": "both the size and alignment
        // are four times the size of the underlying scalar".  So it cannot
        // place the "vec3 color" immediately following the "vec3 position"
        // because of the alignment constraint.
        float dummy;

        Vector3<float> color;
        float size;
    };
#else
    struct Vertex
    {
        Vector3<float> position;
        Vector3<float> color;
        float size;
    };
#endif

    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> symr(-1.0f, 1.0f);
    std::uniform_real_distribution<float> unir(0.0f, 1.0f);
    std::uniform_real_distribution<float> posr(0.01f, 0.1f);

    int const numParticles = 128;
    std::vector<Vertex> particles(numParticles);
    for (auto& particle : particles)
    {
        particle.position = { symr(mte), symr(mte), symr(mte) };
        particle.color = { unir(mte), unir(mte), unir(mte) };
        particle.size = posr(mte);
    }

    // Create the constant buffer used by direct and indirect drawing.
    mMatrices = std::make_shared<ConstantBuffer>(2 * sizeof(Matrix4x4<float>), true);
    program->GetGShader()->Set("Matrices", mMatrices);

#if defined(USE_DRAW_DIRECT)
    // Create a mesh for direct drawing.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, numParticles);
    Memcpy(vbuffer->GetData(), &particles[0], numParticles*sizeof(Vertex));
#else
#if defined(GTE_DEV_OPENGL)
    BufferLayout layout;
    program->GetGShader()->GetStructuredBufferLayout("particles", layout);
    LogAssert(layout[0].offset == offsetof(Vertex, position),
        "Vertex::position in GLSL is at offset = " + std::to_string(layout[0].offset));
    LogAssert(layout[1].offset == offsetof(Vertex, color),
        "Vertex::color in GLSL is at offset = " + std::to_string(layout[1].offset));
    LogAssert(layout[2].offset == offsetof(Vertex, size),
        "Vertex::size in GLSL is at offset = " + std::to_string(layout[2].offset));
#endif
    auto const layoutSize = program->GetGShader()->GetStructuredBufferSize("particles");
    LogAssert(layoutSize == sizeof(Vertex),
        "Vertex in GLSL has size = " + std::to_string(layoutSize));

    // Create a mesh for indirect drawing.
    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(numParticles);
    mParticles = std::make_shared<StructuredBuffer>(numParticles, sizeof(Vertex));
    Memcpy(mParticles->GetData(), &particles[0], numParticles*sizeof(Vertex));
    program->GetGShader()->Set("particles", mParticles);
#endif

    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(IP_POLYPOINT, numParticles);

    std::shared_ptr<VisualEffect> effect = std::make_shared<VisualEffect>(program);

    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    return true;
}

void GeometryShadersWindow::UpdateConstants()
{
    Matrix4x4<float> vwMatrix, pMatrix;
#if defined(GTE_USE_MAT_VEC)
    vwMatrix = mCamera->GetViewMatrix() * mTrackball.GetOrientation();
#else
    vwMatrix = mTrackball.GetOrientation() * mCamera->GetViewMatrix();
#endif

    mMatrices->SetMember("vwMatrix", vwMatrix);
    mMatrices->SetMember("pMatrix", mCamera->GetProjectionMatrix());
    mEngine->Update(mMatrices);
}
