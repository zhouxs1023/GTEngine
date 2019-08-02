// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/05/02)

#include "FitSqrt.h"
#include "FitInvSqrt.h"
#include "FitSin.h"
#include "FitCos.h"
#include "FitTan.h"
#include "FitASin.h"
#include "FitATan.h"
#include "FitExp2.h"
#include "FitLog2.h"
#include "FitReciprocal.h"
#include <LowLevel/GteLogReporter.h>

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

    FitSqrt fitterSqrt;
    FitInvSqrt fitterInvSqrt;
    FitSin fitterSin;  // template parameter is 'order', degree = 2*order + 1
    FitCos fitterCos;  // template parameter is 'order', degree = 2*order
    FitTan fitterTan;  // template parameter is 'order', degree = 2*order + 1
    FitASin fitterASin;  // template parameter is 'order', degree = 2*order + 1
    FitATan fitterATan;  // template parameter is 'order', degree = 2*order + 1
    FitExp2 fitterExp2;
    FitLog2 fitterLog2;
    FitReciprocal fitterReciprocal;
    std::vector<double> poly;
    double error;
    fitterSin.Generate<4>(poly, error);
    return 0;
}

