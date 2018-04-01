// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
#include "VideoStreamManager.h"
#include "FileVideoStream.h"
using namespace gte;

// NOTE: Expose only one of these.
#if defined(GTE_DEV_OPENGL)
// TODO:  The only flag that works currently with GL4 is DO_MANUAL_SERIAL,
// because the OpenGL engine is not thread-safe regarding resource
// creation (the DX11 engine is).  Add thread-safe resource creation
// to GL4 by supporting sharing via contexts.
#define DO_MANUAL_SERIAL
#else
#define DO_MANUAL_SERIAL
//#define DO_MANUAL_PARALLEL
//#define DO_TRIGGERED_SERIAL
//#define DO_TRIGGERED_PARALLEL
#endif

class VideoStreamsWindow : public Window
{
public:
    virtual ~VideoStreamsWindow();
    VideoStreamsWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool CreateOverlays(int textureWidth, int textureHeight);
    void DrawStatistics();

    enum { NUM_VIDEO_STREAMS = 4 };
    std::vector<std::shared_ptr<VideoStream>> mVideoStreams;
    std::vector<std::shared_ptr<OverlayEffect>> mOverlay;
    std::unique_ptr<VideoStreamManager> mVideoStreamManager;
    VideoStreamManager::Frame mCurrent;
};
