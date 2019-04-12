// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.0 (2019/04/09)

#include "CLODPolylineWindow.h"

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

    Window::Parameters parameters(L"CLODPolylineWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<CLODPolylineWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<CLODPolylineWindow>(window);
    return 0;
}

CLODPolylineWindow::CLODPolylineWindow(Parameters& parameters)
    :
    Window2(parameters)
{
    std::default_random_engine dre;
    std::uniform_real_distribution<float> urd(0.75f, 1.25f);
    int numVertices = 16;
    std::vector<Vector3<float>> vertices(numVertices);
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = static_cast<float>(GTE_C_TWO_PI * i / numVertices);
        vertices[i] = urd(dre)* Vector3<float>{ std::cos(angle), std::sin(angle), 0.0f };
    }

    mPolyline = std::make_unique<CLODPolyline<3, float>>(vertices, true);
}

void CLODPolylineWindow::OnDisplay()
{
    unsigned int white = 0xFFFFFFFF;
    unsigned int black = 0xFF000000;

    ClearScreen(white);

    int numVertices = mPolyline->GetNumVertices();
    auto const& vertices = mPolyline->GetVertices();
    int numEdges = mPolyline->GetNumEdges();
    auto const& edges = mPolyline->GetEdges();

    for (int i = 0; i < numVertices; ++i)
    {
        int x, y;
        Get(vertices[i], x, y);
        DrawThickPixel(x, y, 1, black);
    }

    for (int i = 0; i < numEdges; ++i)
    {
        int x0, y0, x1, y1;
        Get(vertices[edges[2 * i]], x0, y0);
        Get(vertices[edges[2 * i + 1]], x1, y1);
        DrawLine(x0, y0, x1, y1, black);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool CLODPolylineWindow::OnCharPress(unsigned char key, int x, int y)
{
    int levelOfDetail;

    switch (key)
    {
    case '+':  // increase level of detail
    case '=':
        levelOfDetail = mPolyline->GetLevelOfDetail();
        if (levelOfDetail < mPolyline->GetMaxLevelOfDetail())
        {
            mPolyline->SetLevelOfDetail(levelOfDetail + 1);
            OnDisplay();
        }
        return true;
    case '-':  // decrease level of detail
    case '_':
        levelOfDetail = mPolyline->GetLevelOfDetail();
        if (levelOfDetail > mPolyline->GetMinLevelOfDetail())
        {
            mPolyline->SetLevelOfDetail(levelOfDetail - 1);
            OnDisplay();
        }
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}
