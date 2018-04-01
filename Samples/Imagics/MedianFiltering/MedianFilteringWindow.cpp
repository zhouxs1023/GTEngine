// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "MedianFilteringWindow.h"

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

    Window::Parameters parameters(L"MedianFilteringWindow", 0, 0, 1024, 512);
    auto window = TheWindowSystem.Create<MedianFilteringWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<MedianFilteringWindow>(window);
    return 0;
}

MedianFilteringWindow::MedianFilteringWindow(Parameters& parameters)
    :
    Window(parameters),
    mSelection(0)
{
    unsigned int const txWidth = 1024, txHeight = 1024;
    if (!SetEnvironment() || !CreatePrograms(txWidth, txHeight))
    {
        parameters.created = false;
        return;
    }

    mOriginal = std::make_shared<Texture2>(DF_R32_FLOAT, txWidth, txHeight);
    for (int i = 0; i < 2; ++i)
    {
        mImage[i] = std::make_shared<Texture2>(DF_R32_FLOAT, txWidth, txHeight);
        mImage[i]->SetUsage(Resource::SHADER_OUTPUT);
        mImage[i]->SetCopyType(Resource::COPY_BIDIRECTIONAL);
    }

    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0625f, 1.0f);
    float* data = mOriginal->Get<float>();
    for (unsigned int i = 0; i < txWidth*txHeight; ++i)
    {
        data[i] = rnd(mte);
    }
    Memcpy(mImage[0]->GetData(), data, mImage[0]->GetNumBytes());
    Memcpy(mImage[1]->GetData(), data, mImage[1]->GetNumBytes());

    // Create two overlays, one for the original image and one for the
    // median-filtered image.
    std::array<int, 4> rect[2] =
    {
        { 0, 0, mXSize / 2, mYSize },
        { mXSize / 2, 0, mXSize / 2, mYSize }
    };
    for (int i = 0; i < 2; ++i)
    {
        mOverlay[i] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize, mYSize, txWidth, txHeight,
            SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP, false);
        mOverlay[i]->SetOverlayRectangle(rect[i]);
    }
    mOverlay[0]->SetTexture(mOriginal);
    mOverlay[1]->SetTexture(mImage[1]);

    for (int i = 0; i < 4; ++i)
    {
        mMedianProgram[i]->GetCShader()->Set("inImage", mImage[0]);
        mMedianProgram[i]->GetCShader()->Set("outImage", mImage[1]);
    }
    mCProgram = mMedianProgram[0];
}

void MedianFilteringWindow::OnIdle()
{
    mTimer.Measure();

    mEngine->Execute(mCProgram, mNumXGroups, mNumYGroups, 1);
    mEngine->Draw(mOverlay[0]);
    mEngine->Draw(mOverlay[1]);
    std::swap(mImage[0], mImage[1]);
    mCProgram->GetCShader()->Set("inImage", mImage[0]);
    mCProgram->GetCShader()->Set("outImage", mImage[1]);
    mOverlay[1]->SetTexture(mImage[1]);
    std::array<float, 4> textColor{ 1.0f, 1.0f, 0.0f, 1.0f };
    mEngine->Draw(8, mYSize - 24, textColor, msName[mSelection]);
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MedianFilteringWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '0':
        mSelection = 0;
        Memcpy(mImage[0]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[0]);
        Memcpy(mImage[1]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[1]);
        mCProgram = mMedianProgram[0];
        return true;

    case '1':
        mSelection = 1;
        Memcpy(mImage[0]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[0]);
        Memcpy(mImage[1]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[1]);
        mCProgram = mMedianProgram[1];
        return true;

    case '2':
        mSelection = 2;
        Memcpy(mImage[0]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[0]);
        Memcpy(mImage[1]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[1]);
        mCProgram = mMedianProgram[2];
        return true;

    case '3':
        mSelection = 3;
        Memcpy(mImage[0]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[0]);
        Memcpy(mImage[1]->GetData(), mOriginal->GetData(), mOriginal->GetNumBytes());
        mEngine->CopyCpuToGpu(mImage[1]);
        mCProgram = mMedianProgram[3];
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool MedianFilteringWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Imagics/MedianFiltering/Shaders/");
#if defined(GTE_DEV_OPENGL)
    std::vector<std::string> inputs =
    {
        "Median3x3.glsl",
        "Median5x5.glsl",
        "MedianBySort.glsl"
    };
#else
    std::vector<std::string> inputs =
    {
        "Median3x3.hlsl",
        "Median5x5.hlsl",
        "MedianBySort.hlsl",
        "MedianShared.hlsli"
    };
#endif

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

bool MedianFilteringWindow::CreatePrograms(unsigned int txWidth, unsigned int txHeight)
{
    // Create the shaders.
    int const numThreads = 8;
    mNumXGroups = txWidth / numThreads;
    mNumYGroups = txHeight / numThreads;

    mProgramFactory->defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", numThreads);

#if defined(GTE_DEV_OPENGL)
    std::string ext = ".glsl";
#else
    std::string ext = ".hlsl";
#endif

    mProgramFactory->defines.Set("RADIUS", 1);
    mMedianProgram[0] = mProgramFactory->CreateFromFile(mEnvironment.GetPath("MedianBySort" + ext));
    if (!mMedianProgram[0])
    {
        return false;
    }

    mMedianProgram[1] = mProgramFactory->CreateFromFile(mEnvironment.GetPath("Median3x3" + ext));
    if (!mMedianProgram[1])
    {
        return false;
    }

    mProgramFactory->defines.Set("RADIUS", 2);
    mMedianProgram[2] = mProgramFactory->CreateFromFile(mEnvironment.GetPath("MedianBySort" + ext));
    if (!mMedianProgram[2])
    {
        return false;
    }

    mMedianProgram[3] = mProgramFactory->CreateFromFile(mEnvironment.GetPath("Median5x5" + ext));
    if (!mMedianProgram[3])
    {
        return false;
    }

    mProgramFactory->defines.Clear();
    return true;
}

std::string MedianFilteringWindow::msName[4] =
{
    "median 3x3 by insertion sort",
    "median 3x3 by min-max",
    "median 5x5 by insertion sort",
    "median 5x5 by min-max"
};
