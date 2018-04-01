// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "StructuredBuffersWindow.h"

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

    Window::Parameters parameters(L"StructuredBuffersWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<StructuredBuffersWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<StructuredBuffersWindow>(window);
    return 0;
}

StructuredBuffersWindow::StructuredBuffersWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 0.0f, 0.0f, 1.25f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void StructuredBuffersWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    memset(mDrawnPixels->GetData(), 0, mDrawnPixels->GetNumBytes());
    mEngine->CopyCpuToGpu(mDrawnPixels);

    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);

    mEngine->CopyGpuToCpu(mDrawnPixels);
    Vector4<float>* src = mDrawnPixels->Get<Vector4<float>>();
    unsigned int* trg = mDrawnPixelsTexture->Get<unsigned int>();
    for (int i = 0; i < mXSize*mYSize; ++i)
    {
        unsigned int r = static_cast<unsigned char>(255.0f*src[i][0]);
        unsigned int g = static_cast<unsigned char>(255.0f*src[i][1]);
        unsigned int b = static_cast<unsigned char>(255.0f*src[i][2]);
        trg[i] = r | (g << 8) | (b << 16) | (0xFF << 24);
    }
    WICFileIO::SaveToPNG("DrawnPixels.png", mDrawnPixelsTexture);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool StructuredBuffersWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Graphics/StructuredBuffers/Shaders/");
    std::vector<std::string> inputs =
    {
#if defined(GTE_DEV_OPENGL)
        "StructuredBuffersVS.glsl",
        "StructuredBuffersPS.glsl",
#else
        "StructuredBuffers.hlsl",
#endif
        "StoneWall.png"
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

bool StructuredBuffersWindow::CreateScene()
{
    // Create the shaders and associated resources.
#if defined(GTE_DEV_OPENGL)
    mProgramFactory->defines.Set("WINDOW_WIDTH", mXSize);
    std::shared_ptr<VisualProgram> program =
        mProgramFactory->CreateFromFiles(
            mEnvironment.GetPath("StructuredBuffersVS.glsl"),
            mEnvironment.GetPath("StructuredBuffersPS.glsl"),
            "");
#else
    std::string filename = mEnvironment.GetPath("StructuredBuffers.hlsl");
    mProgramFactory->defines.Set("WINDOW_WIDTH", mXSize);
    std::shared_ptr<VisualProgram> program =
        mProgramFactory->CreateFromFiles(filename, filename, "");
#endif
    if (!program)
    {
        return false;
    }
    mProgramFactory->defines.Clear();

    std::shared_ptr<ConstantBuffer> cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVShader()->Set("PVWMatrix", cbuffer);

    // Create the pixel shader and associated resources.
    std::shared_ptr<PixelShader> pshader = program->GetPShader();
    std::string path = mEnvironment.GetPath("StoneWall.png");
    std::shared_ptr<Texture2> baseTexture = WICFileIO::Load(path, false);
#if defined(GTE_DEV_OPENGL)
    pshader->Set("baseSampler", baseTexture);
#else
    pshader->Set("baseTexture", baseTexture);
#endif

    std::shared_ptr<SamplerState> baseSampler = std::make_shared<SamplerState>();
    baseSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    baseSampler->mode[0] = SamplerState::CLAMP;
    baseSampler->mode[1] = SamplerState::CLAMP;
    pshader->Set("baseSampler", baseSampler);

    mDrawnPixels = std::make_shared<StructuredBuffer>(mXSize*mYSize, sizeof(Vector4<float>));
    mDrawnPixels->SetUsage(Resource::SHADER_OUTPUT);
    mDrawnPixels->SetCopyType(Resource::COPY_BIDIRECTIONAL);
    memset(mDrawnPixels->GetData(), 0, mDrawnPixels->GetNumBytes());
    pshader->Set("drawnPixels", mDrawnPixels);

    // Create the visual effect for the square.
    std::shared_ptr<VisualEffect> effect = std::make_shared<VisualEffect>(program);

    // Create a vertex buffer for a single triangle.  The PNG is stored in
    // left-handed coordinates.  The texture coordinates are chosen to reflect
    // the texture in the y-direction.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    Vertex* vertex = vbuffer->Get<Vertex>();
    vertex[0].position = { 0.0f, 0.0f, 0.0f };
    vertex[0].tcoord = { 0.0f, 1.0f };
    vertex[1].position = { 1.0f, 0.0f, 0.0f };
    vertex[1].tcoord = { 1.0f, 1.0f };
    vertex[2].position = { 0.0f, 1.0f, 0.0f };
    vertex[2].tcoord = { 0.0f, 0.0f };
    vertex[3].position = { 1.0f, 1.0f, 0.0f };
    vertex[3].tcoord = { 1.0f, 0.0f };

    // Create an indexless buffer for a triangle mesh with two triangles.
    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(IP_TRISTRIP, 2);

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSquare->localTransform.SetTranslation(-0.5f, -0.5f, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mPVWMatrices.Subscribe(mSquare->worldTransform, cbuffer);

    // The structured buffer is written in the pixel shader.  This texture
    // will receive a copy of it so that we can write the results to disk
    // as a PNG file.
    mDrawnPixelsTexture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, mXSize, mYSize);

    mTrackball.Attach(mSquare);
    mTrackball.Update();
    return true;
}
