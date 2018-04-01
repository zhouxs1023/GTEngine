// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "BufferUpdatingWindow.h"

//#define TEST_UPDATE
//#define TEST_COPY_CPU_TO_GPU
#define TEST_COPY_GPU_TO_CPU

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

    Window::Parameters parameters(L"BufferUpdatingWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<BufferUpdatingWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<BufferUpdatingWindow>(window);
    return 0;
}

BufferUpdatingWindow::BufferUpdatingWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    // Create a flat surface with a gridded texture.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mSurface = mf.CreateRectangle(NUM_SAMPLES, NUM_SAMPLES, 1.0f, 1.0f);
    auto vbuffer = mSurface->GetVertexBuffer();

#if defined(TEST_UPDATE)
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
#endif
#if defined(TEST_COPY_CPU_TO_GPU)
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    vbuffer->SetCopyType(Resource::COPY_CPU_TO_STAGING);
#endif
#if defined(TEST_COPY_GPU_TO_CPU)
    // Start with flat height field, offset the middle row on CPU and
    // copy to GPU, copy from GPU to CPU, modify the middle row, copy
    // from CPU to GPU.  Thus, we need the COPY_BIDIRECTIONAL flag.  If
    // all you do is copy from GPU to CPU, then use COPY_STAGING_TO_CPU.
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    vbuffer->SetCopyType(Resource::COPY_BIDIRECTIONAL);
#endif
    mEngine->Bind(vbuffer);

    mEnvironment.Insert(GetGTEPath() + "/Samples/Data/");
    std::string path = mEnvironment.GetPath("BlueGrid.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory,
        texture, SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP,
        SamplerState::CLAMP);
    mSurface->SetEffect(effect);
    mPVWMatrices.Subscribe(mSurface->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mSurface);

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0, 0.001f, 0.001f,
        { 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void BufferUpdatingWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    // Offset the middle row of vertices of the flat surface.
    auto vbuffer = mSurface->GetVertexBuffer();
    unsigned int saveOffset = vbuffer->GetOffset();
    unsigned int saveNumActiveElements = vbuffer->GetNumActiveElements();
    vbuffer->SetNumActiveElements(NUM_SAMPLES);
    vbuffer->SetOffset(NUM_SAMPLES * NUM_SAMPLES / 2);
    Vertex* vertices = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < vbuffer->GetNumActiveElements(); ++i)
    {
        vertices[i + vbuffer->GetOffset()].position[2] = 1.0f;
    }
#if defined(TEST_UPDATE)
    // 273 fps, NVIDIA Quadro K2200
    mEngine->Update(vbuffer);
#endif
#if defined(TEST_COPY_CPU_TO_GPU)
    // 260 fps, NVIDIA Quadro K2200
    mEngine->CopyCpuToGpu(vbuffer);
#endif
#if defined(TEST_COPY_GPU_TO_CPU)
    mEngine->CopyCpuToGpu(vbuffer);
    mEngine->CopyGpuToCpu(vbuffer);
    float invNumElements = 1.0f / static_cast<float>(vbuffer->GetNumActiveElements());
    for (unsigned int i = 0; i < vbuffer->GetNumActiveElements(); ++i)
    {
        vertices[i + vbuffer->GetOffset()].position[2] -= i * invNumElements;
    }
    mEngine->CopyCpuToGpu(vbuffer);
#endif
    vbuffer->SetOffset(saveOffset);
    vbuffer->SetNumActiveElements(saveNumActiveElements);

    mEngine->ClearBuffers();
    mEngine->Draw(mSurface);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BufferUpdatingWindow::OnCharPress(unsigned char key, int x, int y)
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
    }
    return Window3::OnCharPress(key, x, y);
}
