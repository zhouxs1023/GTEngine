// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "IntersectTriangleBoxWindow.h"

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

    Window::Parameters parameters(L"IntersectTriangleBoxWindow", 0, 0, 512, 512);
    auto window = TheWindowSystem.Create<IntersectTriangleBoxWindow>(parameters);
    TheWindowSystem.MessagePump(window, TheWindowSystem.DEFAULT_ACTION);
    TheWindowSystem.Destroy<IntersectTriangleBoxWindow>(window);
    return 0;
}

IntersectTriangleBoxWindow::IntersectTriangleBoxWindow(Parameters& parameters)
    :
    Window3(parameters),
    mBoxAttached(true),
    mTriangleAttached(true),
    mDeltaTrn(0.1f)
{
    GenerateMessage();

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

    mNoWriteState = std::make_shared<DepthStencilState>();
    mNoWriteState->writeMask = DepthStencilState::MASK_ZERO;
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.1f, 0.01f,
        { 0.0f, 0.0f, -8.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    mPVWMatrices.Update();
    DoIntersectionQueries();
}

void IntersectTriangleBoxWindow::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
        DoIntersectionQueries();
    }

    mEngine->ClearBuffers();
    DrawSorted();

    std::array<float, 4> color{ 0.0f, 0.0f, 0.0f, 1.0f };
    mEngine->Draw(8, 24, color, mMessage);
    mEngine->Draw(8, mYSize - 8, color, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectTriangleBoxWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'i':
    {
        // If an unexpected result occurs, press the 'i' key to transform the
        // box to an axis-aligned box, which makes it slightly easier to
        // analyze the triangles in the triangle-box find-intersection query.
        Matrix4x4<float> wMatrix;
#if defined(GTE_USE_MAT_VEC)
        wMatrix.SetCol(0, HLift(mBox.axis[0], 0.0f));
        wMatrix.SetCol(1, HLift(mBox.axis[1], 0.0f));
        wMatrix.SetCol(2, HLift(mBox.axis[2], 0.0f));
        wMatrix.SetCol(3, HLift(mBox.center, 1.0f));
#else
        wMatrix.SetRow(0, HLift(mBox.axis[0], 0.0f));
        wMatrix.SetRow(1, HLift(mBox.axis[1], 0.0f));
        wMatrix.SetRow(2, HLift(mBox.axis[2], 0.0f));
        wMatrix.SetRow(3, HLift(mBox.center, 1.0f));
#endif
        Matrix4x4<float> invWMatrix = Inverse(wMatrix);
        UpdateVertices(invWMatrix);
        DoIntersectionQueries();
        return true;
    }
    case 'b':
    case 'B':
    {
        mBoxAttached = !mBoxAttached;
        GenerateMessage();
        return true;
    }

    case 't':
    case 'T':
    {
        mTriangleAttached = !mTriangleAttached;
        GenerateMessage();
        return true;
    }

    case 'x':
        if (mBoxAttached)
        {
            mBox.center -= mDeltaTrn * HProject(mCamera->GetRVector());
        }
        if (mTriangleAttached)
        {
            for (int i = 0; i < 3; ++i)
            {
                mTriangle.v[i] -= mDeltaTrn * HProject(mCamera->GetRVector());
            }
        }
        if (mBoxAttached || mTriangleAttached)
        {
            UpdateVertices(Matrix4x4<float>::Identity());
            DoIntersectionQueries();
        }
        return true;

    case 'X':
        if (mBoxAttached)
        {
            mBox.center += mDeltaTrn * HProject(mCamera->GetRVector());
        }
        if (mTriangleAttached)
        {
            for (int i = 0; i < 3; ++i)
            {
                mTriangle.v[i] += mDeltaTrn * HProject(mCamera->GetRVector());
            }
        }
        if (mBoxAttached || mTriangleAttached)
        {
            UpdateVertices(Matrix4x4<float>::Identity());
            DoIntersectionQueries();
        }
        return true;

    case 'y':
        if (mBoxAttached)
        {
            mBox.center -= mDeltaTrn * HProject(mCamera->GetUVector());
        }
        if (mTriangleAttached)
        {
            for (int i = 0; i < 3; ++i)
            {
                mTriangle.v[i] -= mDeltaTrn * HProject(mCamera->GetUVector());
            }
        }
        if (mBoxAttached || mTriangleAttached)
        {
            UpdateVertices(Matrix4x4<float>::Identity());
            DoIntersectionQueries();
        }
        return true;

    case 'Y':
        if (mBoxAttached)
        {
            mBox.center += mDeltaTrn * HProject(mCamera->GetUVector());
        }
        if (mTriangleAttached)
        {
            for (int i = 0; i < 3; ++i)
            {
                mTriangle.v[i] += mDeltaTrn * HProject(mCamera->GetUVector());
            }
        }
        if (mBoxAttached || mTriangleAttached)
        {
            UpdateVertices(Matrix4x4<float>::Identity());
            DoIntersectionQueries();
        }
        return true;

    case 'z':
        if (mBoxAttached)
        {
            mBox.center -= mDeltaTrn * HProject(mCamera->GetDVector());
        }
        if (mTriangleAttached)
        {
            for (int i = 0; i < 3; ++i)
            {
                mTriangle.v[i] -= mDeltaTrn * HProject(mCamera->GetDVector());
            }
        }
        if (mBoxAttached || mTriangleAttached)
        {
            UpdateVertices(Matrix4x4<float>::Identity());
            DoIntersectionQueries();
        }
        return true;

    case 'Z':
        if (mBoxAttached)
        {
            mBox.center += mDeltaTrn * HProject(mCamera->GetDVector());
        }
        if (mTriangleAttached)
        {
            for (int i = 0; i < 3; ++i)
            {
                mTriangle.v[i] += mDeltaTrn * HProject(mCamera->GetDVector());
            }
        }
        if (mBoxAttached || mTriangleAttached)
        {
            UpdateVertices(Matrix4x4<float>::Identity());
            DoIntersectionQueries();
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool IntersectTriangleBoxWindow::OnMouseMotion(MouseButton button, int x, int y,
    unsigned int modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        if (mBoxAttached || mTriangleAttached)
        {
            auto wMatrix = mTrackball.GetRoot()->worldTransform.GetHMatrix();
            UpdateVertices(wMatrix);
            DoIntersectionQueries();
        }
        mTrackball.GetRoot()->worldTransform.MakeIdentity();
        OnMouseClick(MOUSE_LEFT, MOUSE_DOWN, x, y, 0);
    }
    return true;
}

void IntersectTriangleBoxWindow::CreateScene()
{
    mBox.center = { 0.0f, 0.0f, 0.0f };
    mBox.axis[0] = { 1.0f, 0.0f, 0.0f };
    mBox.axis[1] = { 0.0f, 1.0f, 0.0f };
    mBox.axis[2] = { 0.0f, 0.0f, 1.0f };
    mBox.extent = { 1.0f, 2.0f, 3.0f };

    mTriangle.v[0] = { 2.0f, 0.0f, 0.0f };
    mTriangle.v[1] = { 2.0f, 1.0f, 0.0f };
    mTriangle.v[2] = { 2.0f, 0.0f, 1.0f };

    // The box face order is +x, -x, +y, -y, +z, -z.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(uint32_t));
    ibuffer->SetTriangle(0, 0, 1, 3);
    ibuffer->SetTriangle(1, 0, 3, 2);
    for (int i = 0; i < 6; ++i)
    {
        auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
        vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
        mFaceMesh[i] = std::make_shared<Visual>(vbuffer, ibuffer);
    }

    // The mesh to store the clipped triangle outside the box.
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 32);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 32, sizeof(uint32_t));
    ibuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    ibuffer->SetTriangle(0, 0, 1, 2);
    vbuffer->SetNumActiveElements(3);
    ibuffer->SetNumActivePrimitives(1);
    mOutsideTriangleMesh = std::make_shared<Visual>(vbuffer, ibuffer);
    mOutsideTriangleMesh->culling = CULL_NEVER;

    UpdateVertices(Matrix4x4<float>::Identity());

    // The mesh to store the clipped triangle inside the box.
    vbuffer = std::make_shared<VertexBuffer>(vformat, 32);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 32, sizeof(uint32_t));
    ibuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    ibuffer->SetTriangle(0, 0, 1, 2);
    auto position = vbuffer->Get<Vector3<float>>();
    position[0] = mTriangle.v[0];
    position[1] = mTriangle.v[1];
    position[2] = mTriangle.v[2];
    vbuffer->SetNumActiveElements(3);
    ibuffer->SetNumActivePrimitives(1);
    mInsideTriangleMesh = std::make_shared<Visual>(vbuffer, ibuffer);
    mInsideTriangleMesh->culling = CULL_ALWAYS;

    for (int i = 0; i < 6; ++i)
    {
        mRedEffect[i] = std::make_shared<ConstantColorEffect>(mProgramFactory,
            Vector4<float>{ 1.0f, 0.0f, 0.0f, 0.5f });

        mBlueEffect[i] = std::make_shared<ConstantColorEffect>(mProgramFactory,
            Vector4<float>{ 0.0f, 0.0f, 1.0f, 0.5f });

        mFaceMesh[i]->SetEffect(mBlueEffect[i]);
        mPVWMatrices.Subscribe(mFaceMesh[i]->worldTransform,
            mBlueEffect[i]->GetPVWMatrixConstant());
    }

    mGreenEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.0f, 1.0f, 0.0f, 0.5f });
    mOutsideTriangleMesh->SetEffect(mGreenEffect);
    mPVWMatrices.Subscribe(mOutsideTriangleMesh->worldTransform,
        mGreenEffect->GetPVWMatrixConstant());

    mGrayEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 0.5f, 0.5f, 0.5f, 0.5f });
    mInsideTriangleMesh->SetEffect(mGrayEffect);
    mPVWMatrices.Subscribe(mInsideTriangleMesh->worldTransform,
        mGrayEffect->GetPVWMatrixConstant());

    mBoxNode = std::make_shared<Node>();
    mTriangleNode = std::make_shared<Node>();
    for (int i = 0; i < 6; ++i)
    {
        mBoxNode->AttachChild(mFaceMesh[i]);
    }
    mTriangleNode->AttachChild(mOutsideTriangleMesh);
    mTriangleNode->AttachChild(mInsideTriangleMesh);
}

void IntersectTriangleBoxWindow::UpdateVertices(Matrix4x4<float> const& wMatrix)
{
    Vector3<float>* position;

    if (mBoxAttached)
    {
#if defined(GTE_USE_MAT_VEC)
        mBox.center = HProject(wMatrix * HLift(mBox.center, 1.0f));
        for (int i = 0; i < 3; ++i)
        {
            mBox.axis[i] = HProject(wMatrix * HLift(mBox.axis[i], 0.0f));
        }
#else
        mBox.center = HProject(HLift(mBox.center, 1.0f) * wMatrix);
        for (int i = 0; i < 3; ++i)
        {
            mBox.axis[i] = HProject(HLift(mBox.axis[i], 0.0f) * wMatrix);
        }
#endif

        Vector3<float> center, offset[3];
        for (int i = 0; i < 3; ++i)
        {
            offset[i] = mBox.extent[i] * mBox.axis[i];
        }

        // +x face
        position = mFaceMesh[0]->GetVertexBuffer()->Get<Vector3<float>>();
        center = mBox.center + offset[0];
        position[0] = center - offset[1] - offset[2];
        position[1] = center + offset[1] - offset[2];
        position[2] = center - offset[1] + offset[2];
        position[3] = center + offset[1] + offset[2];

        // -x face
        position = mFaceMesh[1]->GetVertexBuffer()->Get<Vector3<float>>();
        center = mBox.center - offset[0];
        position[0] = center + offset[1] - offset[2];
        position[1] = center - offset[1] - offset[2];
        position[2] = center + offset[1] + offset[2];
        position[3] = center - offset[1] + offset[2];

        // +y face
        position = mFaceMesh[2]->GetVertexBuffer()->Get<Vector3<float>>();
        center = mBox.center + offset[1];
        position[0] = center + offset[0] - offset[2];
        position[1] = center - offset[0] - offset[2];
        position[2] = center + offset[0] + offset[2];
        position[3] = center - offset[0] + offset[2];

        // -y face
        position = mFaceMesh[3]->GetVertexBuffer()->Get<Vector3<float>>();
        center = mBox.center - offset[1];
        position[0] = center - offset[0] - offset[2];
        position[1] = center + offset[0] - offset[2];
        position[2] = center - offset[0] + offset[2];
        position[3] = center + offset[0] + offset[2];

        // +z face
        position = mFaceMesh[4]->GetVertexBuffer()->Get<Vector3<float>>();
        center = mBox.center + offset[2];
        position[0] = center - offset[0] - offset[1];
        position[1] = center + offset[0] - offset[1];
        position[2] = center - offset[0] + offset[1];
        position[3] = center + offset[0] + offset[1];

        // -z face
        position = mFaceMesh[5]->GetVertexBuffer()->Get<Vector3<float>>();
        center = mBox.center - offset[2];
        position[0] = center + offset[0] - offset[1];
        position[1] = center - offset[0] - offset[1];
        position[2] = center + offset[0] + offset[1];
        position[3] = center - offset[0] + offset[1];

        for (int i = 0; i < 6; ++i)
        {
            mEngine->Update(mFaceMesh[i]->GetVertexBuffer());
        }
    }

    if (mTriangleAttached)
    {
#if defined(GTE_USE_MAT_VEC)
        for (int i = 0; i < 3; ++i)
        {
            mTriangle.v[i] = HProject(wMatrix * HLift(mTriangle.v[i], 1.0f));
        }
#else
        for (int i = 0; i < 3; ++i)
        {
            mTriangle.v[i] = HProject(HLift(mTriangle.v[i], 1.0f) * wMatrix);
        }
#endif

        position = mOutsideTriangleMesh->GetVertexBuffer()->Get<Vector3<float>>();
        for (int i = 0; i < 3; ++i)
        {
            position[i] = mTriangle.v[i];
        }
        mEngine->Update(mOutsideTriangleMesh->GetVertexBuffer());
    }
}

void IntersectTriangleBoxWindow::DrawSorted()
{
    Matrix4x4<float> wMatrix = mTrackball.GetOrientation();
    Matrix4x4<float> invWMatrix = Inverse(wMatrix);
#if defined(GTE_USE_MAT_VEC)
    Vector3<float> cameraModelPosition = HProject(invWMatrix * mCamera->GetPosition());
#else
    Vector3<float> cameraModelPosition = HProject(mCamera->GetPosition() * invWMatrix);
#endif
    int backFace = 0, frontFace = 5;
    std::array<int, 6> order;
    float dot;

    // +x face visibility test
    dot = cameraModelPosition[0] - mBox.extent[0];
    if (dot <= 0)
    {
        order[backFace++] = 0;
    }
    else
    {
        order[frontFace--] = 0;
    }

    // -x face visibility test
    dot = cameraModelPosition[0] + mBox.extent[0];
    if (dot >= 0)
    {
        order[backFace++] = 1;
    }
    else
    {
        order[frontFace--] = 1;
    }

    // +y face visibility test
    dot = cameraModelPosition[1] - mBox.extent[1];
    if (dot <= 0)
    {
        order[backFace++] = 2;
    }
    else
    {
        order[frontFace--] = 2;
    }

    // -y face visibility test
    dot = cameraModelPosition[1] + mBox.extent[1];
    if (dot >= 0)
    {
        order[backFace++] = 3;
    }
    else
    {
        order[frontFace--] = 3;
    }

    // +z face visibility test
    dot = cameraModelPosition[2] - mBox.extent[2];
    if (dot <= 0)
    {
        order[backFace++] = 4;
    }
    else
    {
        order[frontFace--] = 4;
    }

    // -z face visibility test
    dot = cameraModelPosition[2] + mBox.extent[2];
    if (dot >= 0)
    {
        order[backFace++] = 5;
    }
    else
    {
        order[frontFace--] = 5;
    }

    // Draw any portion of the triangle outside the box (opaque).
    mEngine->SetRasterizerState(mNoCullState);
    if (mOutsideTriangleMesh->culling == CULL_NEVER)
    {
        mEngine->Draw(mOutsideTriangleMesh);
    }
    mEngine->SetDefaultRasterizerState();

    mEngine->SetDepthStencilState(mNoWriteState);
    mEngine->SetBlendState(mBlendState);
    {
        // Draw back-facing box faces.
        int i;
        for (i = 0; i < backFace; ++i)
        {
            mEngine->Draw(mFaceMesh[order[i]]);
        }

        // Draw any portion of the triangle inside the box.
        mEngine->SetRasterizerState(mNoCullState);
        {
            if (mInsideTriangleMesh->culling == CULL_NEVER)
            {
                mEngine->Draw(mInsideTriangleMesh);
            }
        }
        mEngine->SetDefaultRasterizerState();

        // Draw front-facing box faces.
        for (/**/; i < 6; ++i)
        {
            mEngine->Draw(mFaceMesh[order[i]]);
        }
    }
    mEngine->SetDefaultBlendState();
    mEngine->SetDefaultDepthStencilState();
}

void IntersectTriangleBoxWindow::DoIntersectionQueries()
{
    FIQuery<float, Triangle3<float>, OrientedBox3<float>> query;
    auto result = query(mTriangle, mBox);
    bool intersects = (result.insidePolygon.size() >= 3);
    if (intersects)
    {
        uint32_t numVertices = static_cast<uint32_t>(result.insidePolygon.size());
        uint32_t numTriangles = numVertices - 2;
        auto vbuffer = mInsideTriangleMesh->GetVertexBuffer();
        auto ibuffer = mInsideTriangleMesh->GetIndexBuffer();
        Vector3<float>* position = vbuffer->Get<Vector3<float>>();
        for (uint32_t i = 0; i < numVertices; ++i)
        {
            position[i] = result.insidePolygon[i];
        }
        for (uint32_t t = 0; t < numTriangles; ++t)
        {
            ibuffer->SetTriangle(t, 0, t + 1, t + 2);
        }
        vbuffer->SetNumActiveElements(numVertices);
        ibuffer->SetNumActivePrimitives(numTriangles);
        mInsideTriangleMesh->culling = CULL_NEVER;
        mEngine->Update(vbuffer);
        mEngine->Update(ibuffer);
    }
    else
    {
        mInsideTriangleMesh->culling = CULL_ALWAYS;
    }

    if (result.outsidePolygons.size() > 0)
    {
        auto vbuffer = mOutsideTriangleMesh->GetVertexBuffer();
        auto ibuffer = mOutsideTriangleMesh->GetIndexBuffer();
        Vector3<float>* position = vbuffer->Get<Vector3<float>>();
        uint32_t* indices = ibuffer->Get<uint32_t>();
        uint32_t totalVertices = 0;
        uint32_t totalTriangles = 0;
        for (auto const& outsidePolygon : result.outsidePolygons)
        {
            uint32_t numVertices = static_cast<uint32_t>(outsidePolygon.size());
            uint32_t numTriangles = numVertices - 2;
            for (uint32_t i = 0; i < numVertices; ++i)
            {
                *position++ = outsidePolygon[i];
            }
            for (uint32_t t = 0; t < numTriangles; ++t)
            {
                *indices++ = totalVertices;
                *indices++ = totalVertices + t + 1;
                *indices++ = totalVertices + t + 2;
            }
            totalVertices += numVertices;
            totalTriangles += numTriangles;
        }
        vbuffer->SetNumActiveElements(totalVertices);
        ibuffer->SetNumActivePrimitives(totalTriangles);
        mOutsideTriangleMesh->culling = CULL_NEVER;
        mEngine->Update(vbuffer);
        mEngine->Update(ibuffer);
    }
    else
    {
        mOutsideTriangleMesh->culling = CULL_ALWAYS;
    }

#if 1
    // Test the TIQuery code.  If this block of code is disabled, then
    // the box color updates use the FIQuery 'intersects', which in theory
    // should match those of TIQuery.
    TIQuery<float, Triangle3<float>, OrientedBox3<float>> tiquery;
    auto tiresult = tiquery(mTriangle, mBox);
    intersects = tiresult.intersect;
#endif
    if (intersects)
    {
        // The triangle intersects the solid box.  If previously there was
        // no intersection, the box is blue and we now change it to red.
        if (mFaceMesh[0]->GetEffect() == mBlueEffect[0])
        {
            for (int i = 0; i < 6; ++i)
            {
                mPVWMatrices.Unsubscribe(mFaceMesh[i]->worldTransform);
                mFaceMesh[i]->SetEffect(mRedEffect[i]);
                mPVWMatrices.Subscribe(mFaceMesh[i]->worldTransform,
                    mRedEffect[i]->GetPVWMatrixConstant());
            }
        }
    }
    else
    {
        // The triangle does not intersect the solid box.  If previously
        // there was an intersection, the box is red and we now change it
        // to blue.
        if (mFaceMesh[0]->GetEffect() == mRedEffect[0])
        {
            for (int i = 0; i < 6; ++i)
            {
                mPVWMatrices.Unsubscribe(mFaceMesh[i]->worldTransform);
                mFaceMesh[i]->SetEffect(mBlueEffect[i]);
                mPVWMatrices.Subscribe(mFaceMesh[i]->worldTransform,
                    mBlueEffect[i]->GetPVWMatrixConstant());
            }
        }
    }

    mPVWMatrices.Update();
}

void IntersectTriangleBoxWindow::GenerateMessage()
{
    if (mBoxAttached)
    {
        if (mTriangleAttached)
        {
            mMessage = "trackball updates box and triangle";
        }
        else
        {
            mMessage = "trackball updates box";
        }
    }
    else
    {
        if (mTriangleAttached)
        {
            mMessage = "trackball updates triangle";
        }
        else
        {
            mMessage = "trackball updates nothing";
        }
    }
}
