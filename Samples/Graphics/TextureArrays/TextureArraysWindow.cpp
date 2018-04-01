// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "TextureArraysWindow.h"

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

    Window::Parameters parameters(L"TextureArraysWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<TextureArraysWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<TextureArraysWindow>(window);
    return 0;
}

TextureArraysWindow::TextureArraysWindow(Parameters& parameters)
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

void TextureArraysWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool TextureArraysWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Graphics/TextureArrays/Shaders/");
    std::vector<std::string> inputs =
    {
        "TextureArrays.hlsl",
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

bool TextureArraysWindow::CreateScene()
{
    std::shared_ptr<VisualProgram> program;

    int api = mProgramFactory->GetAPI();

    // Load and compile the shaders.
    if (ProgramFactory::PF_GLSL == api)
    {
        auto pathVertexShader = mEnvironment.GetPath("TextureArraysVertex.glsl");
        auto pathPixelShader = mEnvironment.GetPath("TextureArraysPixel.glsl");
        program = mProgramFactory->CreateFromFiles(pathVertexShader, pathPixelShader, "");
    }
    else if (ProgramFactory::PF_HLSL == api)
    {
        auto path = mEnvironment.GetPath("TextureArrays.hlsl");
        program = mProgramFactory->CreateFromFiles(path, path, "");
    }
    else
    {
        LogError("No shader support for API=" + api);
    }

    if (!program)
    {
        return false;
    }

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

    // Create an effect for the vertex and pixel shaders.  The texture is
    // bilinearly filtered and the texture coordinates are clamped to [0,1]^2.
    std::shared_ptr<ConstantBuffer> cbuffer = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    program->GetVShader()->Set("PVWMatrix", cbuffer);

    std::shared_ptr<PixelShader> pshader = program->GetPShader();
    std::shared_ptr<Texture1Array> t1array = std::make_shared<Texture1Array>(2, DF_R8G8B8A8_UNORM, 2);
    unsigned int* t1data = t1array->Get<unsigned int>();
    t1data[0] = 0xFF000000;
    t1data[1] = 0xFFFFFFFF;

    auto stoneTexture = WICFileIO::Load(mEnvironment.GetPath("StoneWall.png"), false);
    std::shared_ptr<Texture2Array> t2array = std::make_shared<Texture2Array>(2, DF_R8G8B8A8_UNORM, 256, 256);
    unsigned char* t2data = t2array->Get<unsigned char>();
    size_t const numBytes = stoneTexture->GetNumBytes();
    Memcpy(t2data, stoneTexture->GetData(), numBytes);
    t2data += numBytes;
    for (size_t i = 0; i < numBytes; ++i)
    {
        *t2data++ = static_cast<unsigned char>(rand() % 256);
    }

    std::shared_ptr<SamplerState> samplerState = std::make_shared<SamplerState>();
    samplerState->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    samplerState->mode[0] = SamplerState::CLAMP;
    samplerState->mode[1] = SamplerState::CLAMP;

    if (ProgramFactory::PF_GLSL == api)
    {
        pshader->Set("mySampler1", t1array);
        pshader->Set("mySampler1", samplerState);

        pshader->Set("mySampler2", t2array);
        pshader->Set("mySampler2", samplerState);
    }
    else if (ProgramFactory::PF_HLSL == api)
    {
        pshader->Set("myTexture1", t1array);
        pshader->Set("myTexture2", t2array);
        pshader->Set("mySampler", samplerState);
    }

    std::shared_ptr<VisualEffect> effect = std::make_shared<VisualEffect>(program);

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSquare->localTransform.SetTranslation(-0.5f, -0.5f, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mPVWMatrices.Subscribe(mSquare->worldTransform, cbuffer);

    mTrackball.Attach(mSquare);
    mTrackball.Update();
    return true;
}
