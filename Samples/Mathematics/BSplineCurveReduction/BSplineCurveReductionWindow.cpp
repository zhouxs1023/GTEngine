// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.23.1 (2019/04/23)

#include "BSplineCurveReductionWindow.h"
#include <LowLevel/GteLogReporter.h>
#include <Graphics/GteConstantColorEffect.h>
#include <Mathematics/GteBSplineCurve.h>
#include <Mathematics/GteBSplineReduction.h>

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

    Window::Parameters parameters(L"BSplineCurveReductionWindow", 0, 0, 768, 768);
    auto window = TheWindowSystem.Create<BSplineCurveReductionWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<BSplineCurveReductionWindow>(window);
    return 0;
}

BSplineCurveReductionWindow::BSplineCurveReductionWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 10000.0f, 1.0f, 0.01f,
        { 0.0f, 0.0f, -600.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    
    CreateScene();
    mTrackball.Update();
    mPVWMatrices.Update();
}

void BSplineCurveReductionWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mOriginal);
    mEngine->Draw(mReduced);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BSplineCurveReductionWindow::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Mathematics/BSplineCurveReduction/Data");

    if (mEnvironment.GetPath("ControlPoints.txt") == "")
    {
        LogError("Cannot find file ControlPoints.txt");
        return false;
    }
    return true;
}

void BSplineCurveReductionWindow::CreateScene()
{
    // Keep track of the average to translate the two curves to the
    // origin for trackball rotation.
    Vector3<double> daverage = { 0.0, 0.0, 0.0 };

    // Load the control points for the B-spline curve.
    std::string path = mEnvironment.GetPath("ControlPoints.txt");
    std::ifstream inFile(path);
    int numInControls;
    inFile >> numInControls;
    std::vector<Vector3<double>> inControls(numInControls);
    for (int i = 0; i < numInControls; ++i)
    {
        inFile >> inControls[i][0];
        inFile >> inControls[i][1];
        inFile >> inControls[i][2];
        daverage += inControls[i];
    }
    inFile.close();

    // Create the B-spline curve.
    int const degree = 3;
    BasisFunctionInput<double> inBasis(numInControls, degree);
    BSplineCurve<3, double> original(inBasis, inControls.data());

    // Create the control points for the reduced B-spline curve.
    std::vector<Vector3<double>> outControls;
    double fraction = 0.1;
    BSplineReduction<3, double> reducer;
    reducer(inControls, degree, fraction, outControls);
    int numOutControls = static_cast<int>(outControls.size());
    BasisFunctionInput<double> outBasis(numOutControls, degree);
    BSplineCurve<3, double> reduced(outBasis, outControls.data());
    for (int i = 0; i < numOutControls; ++i)
    {
        daverage += outControls[i];
    }
    daverage /= static_cast<double>(numInControls + numOutControls);
    Vector3<float> average
    {
        static_cast<float>(daverage[0]),
        static_cast<float>(daverage[1]),
        static_cast<float>(daverage[2])
    };

    // The vertex format is shared by the Visual objects for both curves.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);

    // Create the polyline approximation to visualize the original curve.
    unsigned int numVertices = static_cast<unsigned int>(numInControls);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto vertices = vbuffer->Get<Vector3<float>>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        double t = i / static_cast<double>(numVertices);
        Vector3<double> pos;
        original.Evaluate(t, 0, &pos);
        for (int j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(pos[j]);
        }
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, numVertices - 1);

    Vector4<float> red = { 1.0f, 0.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, red);

    mOriginal = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mOriginal->localTransform.SetTranslation(-average);
    mPVWMatrices.Subscribe(mOriginal->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mOriginal);

    // Create the polyline approximation to visualize the reduced curve.
    numVertices = static_cast<unsigned int>(outControls.size());
    vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vertices = vbuffer->Get<Vector3<float>>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        double t = i / static_cast<double>(numVertices);
        Vector3<double> pos;
        reduced.Evaluate(t, 0, &pos);
        for (int j = 0; j < 3; ++j)
        {
            vertices[i][j] = static_cast<float>(pos[j]);
        }
    }

    ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, numVertices - 1);

    Vector4<float> blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory, blue);

    mReduced = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mReduced->localTransform.SetTranslation(-average);
    mPVWMatrices.Subscribe(mReduced->worldTransform, effect->GetPVWMatrixConstant());
    mTrackball.Attach(mReduced);
}
