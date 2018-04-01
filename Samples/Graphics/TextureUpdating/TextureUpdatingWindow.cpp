// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "TextureUpdatingWindow.h"

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

    Window::Parameters parameters(L"TextureUpdatingWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<TextureUpdatingWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<TextureUpdatingWindow>(window);
    return 0;
}

TextureUpdatingWindow::TextureUpdatingWindow(Parameters& parameters)
    :
    Window3(parameters),
    mForward(true)
{
    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0, 0.001f, 0.001f,
        { 0.0f, 0.0f, 1.25 }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void TextureUpdatingWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();

    // Access the current texture for the effect.
    auto data = reinterpret_cast<uint8_t*>(mTexture->GetData());
    unsigned const rowBytes = mTexture->GetWidth() * mTexture->GetElementSize();

    // First clear all the values in the CPU copy of the texture.
    memset(data, 0, rowBytes*mTexture->GetHeight());

    // Read the values back from the GPU.
    mEngine->CopyGpuToCpu(mTexture);

    // Move the rows "down" in a circular fashion.
    // Use the Update call on the texture.
    if (mForward)
    {
        data += rowBytes * (mTexture->GetHeight() - 1);
        std::vector<uint8_t*> saveRow(rowBytes);
        memcpy(saveRow.data(), data, rowBytes);
        for (unsigned y=mTexture->GetHeight()-1; y > 0; --y)
        {
            memmove(data, data-rowBytes, rowBytes);
            data -= rowBytes;
        }
        memcpy(data, saveRow.data(), rowBytes);
#if defined(GTE_DEV_OPENGL)
        mEngine->Update(mTexture);
#else
        // TODO: DX11 automipmapped textures fail the Update(...) call,
        // probably due to being tagged as render targets.  Can this be
        // fixed?  Verify that in fact the automipmapped textures must
        // be render targets.
        mEngine->CopyCpuToGpu(mTexture);
#endif
    }

    // Move the rows "up" in a circular fashion.
    // Use the CopyCpuToGpu call on the texture.
    else
    {
        std::vector<uint8_t*> saveRow(rowBytes);
        memcpy(saveRow.data(), data, rowBytes);
        for (unsigned y=1; y < mTexture->GetHeight(); ++y)
        {
            memmove(data, data+rowBytes, rowBytes);
            data += rowBytes;
        }
        memcpy(data, saveRow.data(), rowBytes);
        mEngine->CopyCpuToGpu(mTexture);
    }
}

bool TextureUpdatingWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'f':
    case 'F':
        mForward = true;
        return true;

    case 'b':
    case 'B':
        mForward = false;
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

void TextureUpdatingWindow::CreateScene()
{
    // Create a vertex buffer for a two-triangles square.  The PNG is stored
    // in left-handed coordinates.  The texture coordinates are chosen to
    // reflect the texture in the y-direction.
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
    mTexture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, 256, 256, true, true);
    mTexture->AutogenerateMipmaps();
    mTexture->SetCopyType(Texture2::COPY_BIDIRECTIONAL);
    mTexture->SetUsage(Texture2::DYNAMIC_UPDATE);
    uint8_t* data = mTexture->Get<uint8_t>();
    for (unsigned y = 0; y < mTexture->GetHeight(); ++y)
    {
        unsigned const rowBytes = mTexture->GetWidth() * mTexture->GetElementSize();
        memset(data, y, rowBytes);
        data += rowBytes;
    }
    std::shared_ptr<Texture2Effect> effect =
        std::make_shared<Texture2Effect>(mProgramFactory, mTexture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP,
        SamplerState::CLAMP);

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSquare->localTransform.SetTranslation(-0.5f, -0.5f, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mPVWMatrices.Subscribe(mSquare->worldTransform, effect->GetPVWMatrixConstant());

    mTrackball.Attach(mSquare);
    mTrackball.Update();
}
