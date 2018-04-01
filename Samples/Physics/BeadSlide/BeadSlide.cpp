// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include <LowLevel/GteLogReporter.h>
#include "PhysicsModule.h"
#include <fstream>
#include <sstream>
#include <iomanip>

void Simulation()
{
    // Set up the physics module.
    PhysicsModule module;
    module.gravity = 1.0f;
    module.mass = 0.1f;

    float time = 0.0f;
    float deltaTime = 0.001f;
    float q = 1.0f;
    float qDot = 0.0f;
    module.Initialize(time, deltaTime, q, qDot);

    // Run the simulation.
    std::ofstream outFile("simulation.txt");
    outFile << "time   q            qder         position" << std::endl;
    int const imax = 2500;
    for (int i = 0; i < imax; ++i)
    {
        float x = q, y = q * q, z = q * y;

        std::ostringstream message;
        message << std::fixed << std::showpoint;
        message << std::setw(5) << std::setprecision(3) << time << ' ';
        message << std::showpos;
        message << std::setw(12) << std::setprecision(8) << q << ' ';
        message << std::setw(12) << std::setprecision(8) << qDot << ' ';
        message << std::setw(8) << std::setprecision(4) << x << ' ';
        message << std::setw(8) << std::setprecision(4) << y << ' ';
        message << std::setw(8) << std::setprecision(4) << z;
        outFile << message.str() << std::endl;

        module.Update();
        time = module.GetTime();
        q = module.GetQ();
        qDot = module.GetQDot();
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
    Simulation();
    return 0;
}
