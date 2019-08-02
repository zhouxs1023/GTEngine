// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.2 (2019/05/03)

#include "ExtractLevelCurvesWindow.h"
#include <LowLevel/GteLogReporter.h>

// Determine how large the window should be to draw an enlarged version of
// the original image.  The pixel type is int16_t but only 10 bits are used
// per pixel.  Feel free to modify gsMagnify to 1, 2, 3 or 4.
static int const gsImageSize = 256;
static int const gsMagnify = 3;
static int const gsEnlargedSize = gsMagnify * gsImageSize;
static int const gsMaxPixel = 1024;  // 10 bits per pixel

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

    Window::Parameters parameters(L"ExtractLevelCurvesWindow", 0, 0, gsEnlargedSize, gsEnlargedSize);
    auto window = TheWindowSystem.Create<ExtractLevelCurvesWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<ExtractLevelCurvesWindow>(window);
    return 0;
}

ExtractLevelCurvesWindow::ExtractLevelCurvesWindow(Parameters& parameters)
    :
    Window2(parameters),
    mOriginal(gsImageSize * gsImageSize),
    mEnlarged(gsEnlargedSize * gsEnlargedSize),
    mUseSquares(true),
    mMouseDown(false)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Read the 256x256 input image.  Each pixel is stored as a 16-bit
    // unsigned integer but using only 10 bits; that is, the pixel values
    // are in [0,1024).
    std::string path = mEnvironment.GetPath("Head_U16_X256_Y256.binary");
    std::ifstream input(path, std::ios::binary);
    input.read((char*)mOriginal.data(), mOriginal.size() * sizeof(int16_t));
    input.close();

    // Enlarge the image.
    int const numer = gsImageSize - 1, denom = gsMaxPixel - 1;
    for (int y = 0, index = 0; y < gsImageSize; ++y)
    {
        for (int x = 0; x < gsImageSize; ++x, ++index)
        {
            // Scale the pixel value to [0,256).
            uint8_t value = static_cast<uint8_t>(mOriginal[index] * numer / denom);
            uint32_t gray = value | (value << 8) | (value << 16) | 0xFF000000;
            for (int dy = 0; dy < gsMagnify; ++dy)
            {
                int my = gsMagnify * y + dy;
                for (int dx = 0; dx < gsMagnify; ++dx)
                {
                    int mx = gsMagnify * x + dx;
                    mEnlarged[mx + gsEnlargedSize * my] = gray;
                }
            }
        }
    }

    mExtractorSquares = std::make_unique<CurveExtractorSquares<int16_t, double>>(
        gsImageSize, gsImageSize, mOriginal.data());

    mExtractorTriangles = std::make_unique<CurveExtractorTriangles<int16_t, double>>(
        gsImageSize, gsImageSize, mOriginal.data());
}

void ExtractLevelCurvesWindow::OnDisplay()
{
    // Copy the image as background.  Level curves are drawn on top of
    // this in color.
    auto texels = mScreenTexture->Get<uint32_t>();
    std::memcpy(texels, mEnlarged.data(), mEnlarged.size() * sizeof(uint32_t));

    uint32_t const color = (mUseSquares ? 0xFF00FF00 : 0xFF0000FF);
    for (auto const& edge : mEdges)
    {
        int x0 = gsMagnify * static_cast<int>(mVertices[edge.v[0]][0]);
        int y0 = gsMagnify * static_cast<int>(mVertices[edge.v[0]][1]);
        int x1 = gsMagnify * static_cast<int>(mVertices[edge.v[1]][0]);
        int y1 = gsMagnify * static_cast<int>(mVertices[edge.v[1]][1]);
        DrawLine(x0, y0, x1, y1, color);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool ExtractLevelCurvesWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'e':
    case 'E':
        mUseSquares = !mUseSquares;
        ExtractLevelCurves(x, y);
        return true;
    }
    return Window2::OnCharPress(key, x, y);
}

bool ExtractLevelCurvesWindow::OnMouseClick(int button, int state, int x, int y, unsigned int)
{
    if (button == MOUSE_LEFT)
    {
        if (state == MOUSE_DOWN)
        {
            mMouseDown = true;
            ExtractLevelCurves(x, y);
        }
        else
        {
            mMouseDown = false;
        }
        return true;
    }
    return false;
}

bool ExtractLevelCurvesWindow::OnMouseMotion(int button, int x, int y, unsigned int)
{
    if (button == MOUSE_LEFT && mMouseDown)
    {
        ExtractLevelCurves(x, y);
        return true;
    }
    return false;
}

bool ExtractLevelCurvesWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    if (mEnvironment.GetPath("Head_U16_X256_Y256.binary") == "")
    {
        LogError("Cannot find file Head_U16_X256_Y256.binary");
        return false;
    }
    return true;
}

void ExtractLevelCurvesWindow::ExtractLevelCurves(int x, int y)
{
    // If you uncomment the MakeUnique calls, the performance takes a minor
    // hit for mExtractorSquares but a major hit for mExtractorTriangles.
    if (0 <= x && x < mXSize && 0 <= y && y < mYSize)
    {
        x = x / gsMagnify;
        y = y / gsMagnify;
        int16_t level = mOriginal[x + gsImageSize * y];
        std::vector<CurveExtractor<int16_t, double>::Vertex> rationalVertices;
        if (mUseSquares)
        {
            mExtractorSquares->Extract(level, rationalVertices, mEdges);
            // mExtractorSquares->MakeUnique(rationalVertices, mEdges);
            mExtractorSquares->Convert(rationalVertices, mVertices);
        }
        else
        {
            mExtractorTriangles->Extract(level, rationalVertices, mEdges);
            // mExtractorTriangles->MakeUnique(rationalVertices, mEdges);
            mExtractorTriangles->Convert(rationalVertices, mVertices);
        }
        OnDisplay();
    }
}
