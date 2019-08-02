// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.4 (2019/05/03)

#include <Applications/GteEnvironment.h>
#include <LowLevel/GteLogReporter.h>
#include <GTGraphics.h>
#include <random>
#include <iomanip>
using namespace gte;

void TestPartialSums()
{
    Environment env;
    std::string path = env.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return;
    }
    env.Insert(path + "/Samples/Mathematics/PartialSums/Shaders/");

    path = env.GetPath(DefaultShaderName("PartialSums.cs"));
    if (path == "")
    {
        LogError("Cannot find file " + DefaultShaderName("PartialSums.cs"));
        return;
    }

    DefaultEngine engine;
    DefaultProgramFactory factory;

    // Compute partial sums of 8 numbers.
    int const LOGN = 3;
    int const n = (1 << LOGN);

    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> urd(0.0f, 1.0f);

    // Select random numbers and store as the diagonal of an n-by-n texture.
    auto sum = std::make_shared<Texture2>(DF_R32_FLOAT, n, n);
    sum->SetUsage(Resource::SHADER_OUTPUT);
    sum->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    auto data = sum->Get<float>();
    std::memset(data, 0, sum->GetNumBytes());
    for (int i = 0; i < n; ++i)
    {
        data[i + n*i] = urd(mte);
    }

    // Create the shader for each p with 1 <= p <= log(n).
    std::shared_ptr<ComputeProgram> partialSumProgram[LOGN];
    for (int i = 0, p = 1; i < LOGN; ++i, ++p)
    {
        factory.defines.Set("NUM_X_THREADS", (1 << (LOGN - p)));
        factory.defines.Set("NUM_Y_THREADS", (1 << i));
        factory.defines.Set("TWO_P", (1 << p));
        factory.defines.Set("TWO_PM1", (1 << i));
        partialSumProgram[i] = factory.CreateFromFile(path);
        if (!partialSumProgram[i])
        {
            return;
        }
        partialSumProgram[i]->GetCShader()->Set("sum", sum);
    }

    // Execute the shaders.
    for (int i = 0; i < LOGN; ++i)
    {
        engine.Execute(partialSumProgram[i], 1, 1, 1);
    }

    // Read back the results from GPU memory.
    engine.CopyGpuToCpu(sum);

    // Write the entire 2D sum texture to a file.  The first column contains
    // the partial sums.  The other nonzero entries in the texture are the
    // intermediate values computed by the shaders.
    std::ofstream output("PartialSumsResult.txt");
    output << std::setprecision(6) << std::left << std::setfill('0');
    for (int row = 0; row < n; ++row)
    {
        for (int col = 0; col < n; ++col)
        {
            output << std::setw(8) << data[col + n*row] << ' ';
        }
        output << std::endl;
    }
    output << std::endl;
    output.close();

    for (int i = 0; i < LOGN; ++i)
    {
        partialSumProgram[i] = nullptr;
    }
    sum = nullptr;
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

    TestPartialSums();
    return 0;
}
