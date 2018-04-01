// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.3.0 (2016/07/14)

#include <GTEngine.h>
using namespace gte;

int const gSize = 512;
std::shared_ptr<Texture2> gImage;
std::vector<float> gOutput(gSize);
float gPendulumConstant = 1.0f;

std::function<void(int, int)> DrawPixel =
[](int x, int y)
{
    if (0 <= x && x < gSize && 0 <= y && y < gSize)
    {
        unsigned int* pixels = gImage->Get<unsigned int>();
        pixels[x + gSize * y] = 0;
    }
};

typedef void (*SolverFunction)(float, float, float);

void ExplicitEuler(float x0, float y0, float h)
{
    for (int i = 0; i < gSize; ++i)
    {
        float x1 = x0 + h * y0;
        float y1 = y0 - h * gPendulumConstant * sin(x0);

        gOutput[i] = x1;
        x0 = x1;
        y0 = y1;
    }
}

void ImplicitEuler(float x0, float y0, float h)
{
    float const k0 = gPendulumConstant * h * h;
    for (int i = 0; i < gSize; ++i)
    {
        float k1 = x0 + h*y0;
        float x1 = x0;
        int const maxIteration = 32;
        for (int j = 0; j < maxIteration; ++j)
        {
            float g = x1 + k0 * sin(x1) - k1;
            float gDer = 1.0f + k0 * cos(x1);
            x1 -= g / gDer;
        }
        float y1 = y0 - h * gPendulumConstant* sin(x1);

        gOutput[i] = x1;
        x0 = x1;
        y0 = y1;
    }
}

void RungeKutta(float x0, float y0, float h)
{
    for (int i = 0; i < gSize; ++i)
    {
        float k1X = h * y0;
        float k1Y = -h * gPendulumConstant * sin(x0);
        float x1 = x0 + 0.5f * k1X;
        float y1 = y0 + 0.5f * k1Y;
        float k2X = h * y1;
        float k2Y = -h * gPendulumConstant * sin(x1);
        x1 = x0 + 0.5f * k2X;
        y1 = y0 + 0.5f * k2Y;
        float k3X = h * y1;
        float k3Y = -h * gPendulumConstant * sin(x1);
        x1 = x0 + k3X;
        y1 = y0 + k3Y;
        float k4X = h * y1;
        float k4Y = -h * gPendulumConstant * sin(x1);
        x1 = x0 + (k1X + 2.0f * k2X + 2.0f * k3X + k4X) / 6.0f;
        y1 = y0 + (k1Y + 2.0f * k2Y + 2.0f * k3Y + k4Y) / 6.0f;

        gOutput[i] = x1;
        x0 = x1;
        y0 = y1;
    }
}

void LeapFrog(float x0, float y0, float h)
{
    // Generate first iterate with Euler's to start up the process.
    float x1 = x0 + h * y0;
    float y1 = y0 - h * gPendulumConstant * sin(x0);
    gOutput[0] = x1;

    for (int i = 1; i < gSize; ++i)
    {
        float x2 = x0 + 2.0f * h * y1;
        float y2 = y0 - 2.0f * h * gPendulumConstant * sin(x1);

        gOutput[i] = x2;
        x0 = x1;
        y0 = y1;
        x1 = x2;
        y1 = y2;
    }
}

void SolveODE(SolverFunction solver, std::string const& outImage, std::string const& outText)
{
    float x0 = 0.1f, y0 = 1.0f;
    float h = 0.1f;
    solver(x0, y0, h);

    // Write the approximation solution as text.
    std::ofstream outFile(outText);
    for (int i = 0; i < gSize; ++i)
    {
        outFile << "i = " << i << ", " << gOutput[i] << std::endl;
    }
    outFile.close();

    // Draw the approximate solution as an image.
    memset(gImage->GetData(), 0xFF, gImage->GetNumBytes());
    float y = 256.0f * (gOutput[0] + 3.0f) / 6.0f;
    int iY0 = gSize - 1 - static_cast<int>(y);
    for (int i = 1; i < gSize; ++i)
    {
        y = 256.0f * (gOutput[i] + 3.0f) / 6.0f;
        int iY1 = gSize - 1 - static_cast<int>(y);
        ImageUtility2::DrawLine(i - 1, iY0, i, iY1, DrawPixel);
        iY0 = iY1;
    }
    WICFileIO::SaveToPNG(outImage, gImage);
}

void Stiff1()
{
    int const maxIterations = 1024 + 256;
    float const cSqr = 2.0f, c = sqrt(2.0f);

    float h = 0.01f;
    float x0 = 1.0f, x0Save = x0;
    float y0 = -c * x0;

    std::vector<float> approx(maxIterations);
    int i;
    for (i = 0; i < maxIterations; ++i)
    {
        float k1X = h * y0;
        float k1Y = h * cSqr * x0;
        float x1 = x0 + 0.5f * k1X;
        float y1 = y0 + 0.5f * k1Y;
        float k2X = h * y1;
        float k2Y = h * cSqr * x1;
        x1 = x0 + 0.5f * k2X;
        y1 = y0 + 0.5f * k2Y;
        float k3X = h * y1;
        float k3Y = h * cSqr * x1;
        x1 = x0 + k3X;
        y1 = y0 + k3Y;
        float k4X = h * y1;
        float k4Y = h * cSqr * x1;
        x1 = x0 + (k1X + 2.0f * k2X + 2.0f * k3X + k4X) / 6.0f;
        y1 = y0 + (k1Y + 2.0f * k2Y + 2.0f * k3Y + k4Y) / 6.0f;

        approx[i] = x1;
        x0 = x1;
        y0 = y1;
    }

    // Write the approximation solution as text.
    std::ofstream outFile("stiff1.txt");
    for (i = 0; i < maxIterations; ++i)
    {
        outFile << "i = " << i << ", " << approx[i] << std::endl;
    }
    outFile.close();

    // Draw the true solution.
    memset(gImage->GetData(), 0xFF, gImage->GetNumBytes());
    float y = 256.0f * (x0Save + 3.0f) / 6.0f;
    int iY0 = gSize - 1 - static_cast<int>(y);
    for (i = 1; i < gSize; ++i)
    {
        int j = (maxIterations - 1) * i / (gSize - 1);
        y = 256.0f * (x0Save * exp(-c * j * h) + 3.0f) / 6.0f;
        int iY1 = gSize - 1 - static_cast<int>(y);
        ImageUtility2::DrawLine(i - 1, iY0, i, iY1, DrawPixel);
        iY0 = iY1;
    }
    WICFileIO::SaveToPNG("stiff1_true.png", gImage);

    // Draw the approximate solution as an image.
    memset(gImage->GetData(), 0xFF, gImage->GetNumBytes());
    y = 256.0f * (approx[0] + 3.0f) / 6.0f;
    iY0 = gSize - 1 - static_cast<int>(y);
    for (i = 1; i < gSize; ++i)
    {
        int j = (maxIterations - 1) * i / (gSize - 1);
        y = 256.0f * (approx[j] + 3.0f) / 6.0f;
        int iY1 = gSize - 1 - static_cast<int>(y);
        ImageUtility2::DrawLine(i - 1, iY0, i, iY1, DrawPixel);
        iY0 = iY1;
    }
    WICFileIO::SaveToPNG("stiff1_appr.png", gImage);
}

float F0(float t, float x, float y)
{
    return 9.0f * x + 24.0f * y + 5.0f * cos(t) - sin(t) / 3.0f;
}

float F1(float t, float x, float y)
{
    return -24.0f * x - 51.0f * y - 9.0f * cos(t) + sin(t) / 3.0f;
}

void Stiff2TrueSolution()
{
    float h = 0.05f;
    float x0 = 4.0f / 3.0f;
    float y0 = 2.0f / 3.0f;
    float t0 = 0.0f;

    std::ofstream outFile("stiff2_true.txt");
    int const maxIterations = 20;
    for (int i = 0; i <= maxIterations; ++i, t0 += h)
    {
        float e0 = exp(-3.0f*t0);
        float e1 = exp(-39.0f*t0);
        float cDiv3 = cos(t0) / 3.0f;
        x0 = 2.0f * e0 - e1 + cDiv3;
        y0 = -e0 + 2.0f * e1 - cDiv3;
        if (i >= 2 && ((i % 2) == 0))
        {
            outFile << "i = " << i << ", " << x0 << ", " << y0 << std::endl;
        }
    }
    outFile.close();
}

void Stiff2ApproximateSolution()
{
    // Approximation with step size 0.05.
    float h = 0.05f;
    float x0 = 4.0f / 3.0f;
    float y0 = 2.0f / 3.0f;
    float t0 = 0.0f;

    int const maxIterations = 20;
    std::vector<float> approx0(maxIterations + 1), approx1(maxIterations + 1);
    approx0[0] = x0;
    approx1[0] = y0;
    int i;
    for (i = 1; i <= maxIterations; ++i)
    {
        float k1X = h * F0(t0, x0, y0);
        float k1Y = h * F1(t0, x0, y0);
        float x1 = x0 + 0.5f * k1X;
        float y1 = y0 + 0.5f * k1Y;
        float k2X = h * F0(t0 + 0.5f * h, x1, y1);
        float k2Y = h * F1(t0 + 0.5f * h, x1, y1);
        x1 = x0 + 0.5f * k2X;
        y1 = y0 + 0.5f * k2Y;
        float k3X = h * F0(t0 + 0.5f * h, x1, y1);
        float k3Y = h * F1(t0 + 0.5f * h, x1, y1);
        x1 = x0 + k3X;
        y1 = y0 + k3Y;
        float k4X = h * F0(t0 + h, x1, y1);
        float k4Y = h * F1(t0 + h, x1, y1);
        x1 = x0 + (k1X + 2.0f * k2X + 2.0f * k3X + k4X) / 6.0f;
        y1 = y0 + (k1Y + 2.0f * k2Y + 2.0f * k3Y + k4Y) / 6.0f;

        approx0[i] = x1;
        approx1[i] = y1;
        x0 = x1;
        y0 = y1;
        t0 += h;
    }

    std::ofstream outFile("stiff2_appr_h0.05.txt");
    for (i = 0; i <= maxIterations; ++i)
    {
        if ((i % 2) == 0)
        {
            outFile << "i = " << i << ", " << approx0[i] << ", " << approx1[i] << std::endl;
        }
    }
    outFile.close();

    // Approximation with step size 0.1.
    h = 0.1f;
    x0 = 4.0f / 3.0f;
    y0 = 2.0f / 3.0f;
    t0 = 0.0f;

    approx0[0] = x0;
    approx1[0] = y0;
    for (i = 1; i <= maxIterations / 2; ++i)
    {
        float k1X = h * F0(t0, x0, y0);
        float k1Y = h * F1(t0, x0, y0);
        float x1 = x0 + 0.5f * k1X;
        float y1 = y0 + 0.5f * k1Y;
        float k2X = h * F0(t0 + 0.5f * h, x1, y1);
        float k2Y = h * F1(t0 + 0.5f * h, x1, y1);
        x1 = x0 + 0.5f * k2X;
        y1 = y0 + 0.5f * k2Y;
        float k3X = h * F0(t0 + 0.5f * h, x1, y1);
        float k3Y = h * F1(t0 + 0.5f * h, x1, y1);
        x1 = x0 + k3X;
        y1 = y0 + k3Y;
        float k4X = h * F0(t0 + h, x1, y1);
        float k4Y = h * F1(t0 + h, x1, y1);
        x1 = x0 + (k1X + 2.0f * k2X + 2.0f * k3X + k4X) / 6.0f;
        y1 = y0 + (k1Y + 2.0f * k2Y + 2.0f * k3Y + k4Y) / 6.0f;

        approx0[i] = x1;
        approx1[i] = y1;
        x0 = x1;
        y0 = y1;
        t0 += h;
    }

    outFile.open("stiff2_appr_h0.10.txt");
    for (i = 0; i <= maxIterations / 2; ++i)
    {
        outFile << "i = " << i << ", " << approx0[i] << ", " << approx1[i] << std::endl;
    }
    outFile.close();
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

    // The shared GTEngine resource objects cannot be created pre-main.
    gImage = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, gSize, gSize);

    SolveODE(ExplicitEuler, "explicit.png", "explicit.txt");
    SolveODE(ImplicitEuler, "implicit.png", "implicit.txt");
    SolveODE(RungeKutta, "runge.png", "runge.txt");
    SolveODE(LeapFrog, "leapfrog.png", "leapfrog.txt");
    Stiff1();
    Stiff2TrueSolution();
    Stiff2ApproximateSolution();

    gImage = nullptr;
    return 0;
}
