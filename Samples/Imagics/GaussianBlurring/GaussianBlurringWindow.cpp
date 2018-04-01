// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "GaussianBlurringWindow.h"

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

    Window::Parameters parameters(L"GaussianBlurringWindow", 0, 0, 1024, 768);
    auto window = TheWindowSystem.Create<GaussianBlurringWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<GaussianBlurringWindow>(window);
    return 0;
}

GaussianBlurringWindow::GaussianBlurringWindow(Parameters& parameters)
    :
    Window(parameters),
    mNumXThreads(8),
    mNumYThreads(8),
    mNumXGroups(mXSize / mNumXThreads),  // 1024/8 = 128
    mNumYGroups(mYSize / mNumYThreads),  // 768/8 = 96
    mPass(0)
{
    if (!SetEnvironment() || !CreateImages() || !CreateShader())
    {
        parameters.created = false;
        return;
    }

    // Create an overlay that covers the entire window.  The blurred image
    // is drawn by the overlay effect.
    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::MIN_P_MAG_P_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay->SetTexture(mImage[1]);

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize, mYSize);
    mTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);
#endif
}

void GaussianBlurringWindow::OnIdle()
{
    mTimer.Measure();

#if defined(SAVE_RENDERING_TO_DISK)
    if (mPass == 0 || mPass == 100 || mPass == 1000 || mPass == 10000)
    {
        mOverlay->SetTexture(mImage[0]);
        mEngine->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        mEngine->ClearBuffers();
        mEngine->Enable(mTarget);
        mEngine->Draw(mOverlay);
        mEngine->Disable(mTarget);
        mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
        WICFileIO::SaveToPNG("Gauss" + std::to_string(mPass) + ".png", mTarget->GetRTTexture(0));
        mOverlay->SetTexture(mImage[1]);
    }
#endif

    std::shared_ptr<ComputeShader> cshader = mGaussianBlurProgram->GetCShader();
    mEngine->Execute(mGaussianBlurProgram, mNumXGroups, mNumYGroups, 1);
    mEngine->Draw(mOverlay);
    std::swap(mImage[0], mImage[1]);
    cshader->Set("inImage", mImage[0]);
    cshader->Set("outImage", mImage[1]);
    mOverlay->SetTexture(mImage[1]);
    ++mPass;

    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool GaussianBlurringWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Imagics/GaussianBlurring/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "MedicineBag.png",
#if defined(GTE_DEV_OPENGL)
        "GaussianBlur3x3.glsl"
#else
        "GaussianBlur3x3.hlsl"
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

bool GaussianBlurringWindow::CreateImages()
{
    for (int i = 0; i < 2; ++i)
    {
        mImage[i] = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, mXSize, mYSize);
        mImage[i]->SetUsage(Resource::SHADER_OUTPUT);
    }

    std::string path = mEnvironment.GetPath("MedicineBag.png");
    auto original = WICFileIO::Load(path, false);
    unsigned int const* src = original->Get<unsigned int>();
    float* trg = mImage[0]->Get<float>();
    for (int j = 0; j < mXSize*mYSize; ++j)
    {
        unsigned int rgba = *src++;
        *trg++ = (rgba & 0x000000FF) / 255.0f;
        *trg++ = ((rgba & 0x0000FF00) >> 8) / 255.0f;
        *trg++ = ((rgba & 0x00FF0000) >> 16) / 255.0f;
        *trg++ = 1.0f;
    }

    return true;
}

bool GaussianBlurringWindow::CreateShader()
{
#if !defined(GTE_DEV_OPENGL)
    // The flags are chosen to allow you to debug the shaders through MSVS.
    // The menu path is "Debug | Graphics | Start Diagnostics" (ALT+F5).
    mProgramFactory->PushFlags();
    mProgramFactory->flags =
        D3DCOMPILE_ENABLE_STRICTNESS |
        D3DCOMPILE_IEEE_STRICTNESS |
        D3DCOMPILE_DEBUG |
        D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    mProgramFactory->defines.Set("NUM_X_THREADS", mNumXThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", mNumYThreads);
#if defined(GTE_DEV_OPENGL)
    mGaussianBlurProgram = mProgramFactory->CreateFromFile(mEnvironment.GetPath("GaussianBlur3x3.glsl"));
#else
    mGaussianBlurProgram = mProgramFactory->CreateFromFile(mEnvironment.GetPath("GaussianBlur3x3.hlsl"));
#endif
    mProgramFactory->defines.Clear();

#if !defined(GTE_DEV_OPENGL)
    mProgramFactory->PopFlags();
#endif

    if (mGaussianBlurProgram)
    {
        mGaussianBlurProgram->GetCShader()->Set("inImage", mImage[0]);
        mGaussianBlurProgram->GetCShader()->Set("outImage", mImage[1]);
        return true;
    }
    return false;
}
