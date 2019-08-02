// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.4 (2019/05/02)

#include <Applications/GteEnvironment.h>
#include <LowLevel/GteLogReporter.h>
#include <LowLevel/GteTimer.h>
#include <GTGraphics.h>
#include <iostream>
#include <thread>
using namespace gte;

union IEEEFloatType
{
    float number;
    unsigned int encoding;
};

float MyFunction(float z)
{
    return (z - 1.1f) * (z + 2.2f);
}

void FindRootsCPU(std::set<float>& roots)
{
    unsigned int const supTrailing = (1 << 23);
    for (unsigned int trailing = 0; trailing < supTrailing; ++trailing)
    {
        for (unsigned int biased = 0; biased < 255; ++biased)
        {
            IEEEFloatType z0, z1;
            z0.encoding = (biased << 23) | trailing;
            z1.encoding = z0.encoding + 1;

            float f0 = MyFunction(z0.number);
            float f1 = MyFunction(z1.number);
            if (f0 * f1 <= 0.0f)
            {
                roots.insert(std::fabs(f0) <= std::abs(f1) ? z0.number : z1.number);
            }

            z0.number = -z0.number;
            z1.number = -z1.number;
            f0 = MyFunction(z0.number);
            f1 = MyFunction(z1.number);
            if (f0 * f1 <= 0.0f)
            {
                roots.insert(std::fabs(f0) <= std::fabs(f1) ? z0.number : z1.number);
            }
        }
    }
}

void FindRootsGPU(std::set<float>& roots)
{
    Environment env;
    std::string path = env.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return;
    }
    env.Insert(path + "/Samples/Mathematics/RootFinding/Shaders/");

    path = env.GetPath(DefaultShaderName("RootFinder.cs"));
    if (path == "")
    {
        LogError("Cannot find file. ");
        return;
    }

    DefaultEngine engine;
    DefaultProgramFactory factory;
    factory.defines.Set("FUNCTION_BODY", "(z - 1.1f)*(z + 2.2f)");
    auto cprogram = factory.CreateFromFile(path);
    if (!cprogram)
    {
        return;
    }

    auto acBuffer = std::make_shared<StructuredBuffer>(1024, sizeof(Vector4<float>));
    acBuffer->MakeAppendConsume();
    acBuffer->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    acBuffer->SetNumActiveElements(0);

    cprogram->GetCShader()->Set("rootBounds", acBuffer);

    engine.Execute(cprogram, 512, 256, 1);

    engine.CopyGpuToCpu(acBuffer);
    unsigned int numActive = acBuffer->GetNumActiveElements();
    auto rootBounds = acBuffer->Get<Vector4<float>>();
    for (unsigned int i = 0; i < numActive; ++i)
    {
        auto const& rb = rootBounds[i];
        if (std::fabs(rb[1]) <= std::fabs(rb[3]))
        {
            roots.insert(rb[0]);
        }
        else
        {
            roots.insert(rb[2]);
        }
    }

    acBuffer = nullptr;
    cprogram = nullptr;
}

void FindSubRootsCPU(unsigned int tmin, unsigned int tsup, std::set<float>& roots)
{
    for (unsigned int trailing = tmin; trailing < tsup; ++trailing)
    {
        for (unsigned int biased = 0; biased < 255; ++biased)
        {
            IEEEFloatType z0, z1;
            z0.encoding = (biased << 23) | trailing;
            z1.encoding = z0.encoding + 1;

            float f0 = MyFunction(z0.number);
            float f1 = MyFunction(z1.number);
            if (f0 * f1 <= 0.0f)
            {
                roots.insert(std::fabs(f0) <= std::fabs(f1) ? z0.number : z1.number);
            }

            z0.number = -z0.number;
            z1.number = -z1.number;
            f0 = MyFunction(z0.number);
            f1 = MyFunction(z1.number);
            if (f0 * f1 <= 0.0f)
            {
                roots.insert(std::fabs(f0) <= std::fabs(f1) ? z0.number : z1.number);
            }
        }
    }
}

void FindRootsCPUMultithreaded(std::set<float>& roots)
{
    int const numThreads = 16;
    unsigned int const supTrailing = (1 << 23);
    std::set<float> subRoots[numThreads];

    std::thread process[numThreads];
    for (int t = 0; t < numThreads; ++t)
    {
        unsigned int tmin = t * supTrailing / numThreads;
        unsigned int tsup = (t + 1) * supTrailing / numThreads;
        auto rootFinder = std::bind(FindSubRootsCPU, tmin, tsup, std::ref(subRoots[t]));
        process[t] = std::thread([&rootFinder](){ rootFinder(); });
    }

    for (int t = 0; t < numThreads; ++t)
    {
        process[t].join();
    }

    for (int t = 0; t < numThreads; ++t)
    {
        for (auto const& z : subRoots[t])
        {
            roots.insert(z);
        }
    }
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

    std::set<float> rootsCPU, rootsCPUMultithreaded, rootsGPU;

    Timer timer;
    double start, final, delta;
    std::string message;

    start = timer.GetSeconds();
    FindRootsCPU(rootsCPU);
    final = timer.GetSeconds();
    delta = final - start;
    message = "CPU: " + std::to_string(delta) + "\n";
    std::cout << message;

    start = timer.GetSeconds();
    FindRootsCPUMultithreaded(rootsCPUMultithreaded);
    final = timer.GetSeconds();
    delta = final - start;
    message = "CPU multithreaded: " + std::to_string(delta) + "\n";
    std::cout << message;

    start = timer.GetSeconds();
    FindRootsGPU(rootsGPU);
    final = timer.GetSeconds();
    delta = final - start;
    message = "GPU: " + std::to_string(delta) + "\n";
    std::cout << message;
    return 0;
}
