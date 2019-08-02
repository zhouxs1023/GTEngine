// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.1 (2019/05/02)

#include <LowLevel/GteLogReporter.h>
#include <Mathematics/GteIntpThinPlateSpline2.h>
#include <Mathematics/GteIntpThinPlateSpline3.h>
#include <array>
#include <fstream>
#include <iomanip>
#include <random>
using namespace gte;

static void TestThinPlateSplines2D()
{
    std::ofstream output("output2.txt");
    output << std::scientific << std::setprecision(8);

    // Tabulated data on a 3x3 regular grid, points of form (x,y,f(x,y)).
    int const numPoints = 9;
    std::array<double, numPoints> x = { 0.0, 0.5, 1.0, 0.0, 0.5, 1.0, 0.0, 0.5, 1.0 };
    std::array<double, numPoints> y = { 0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0 };
    std::array<double, numPoints> f = { 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 1.0, 2.0, 3.0 };

    // Resample on a 7x7 regular grid.
    int const numResample = 6;
    double const invResample = 1.0 / static_cast<double>(numResample);
    double smooth, interp, functional;

    // No smoothing, exact interpolation at grid points.
    smooth = 0.0;
    IntpThinPlateSpline2<double> noSmooth(
        numPoints, x.data(), y.data(), f.data(), smooth, false);
    output << "no smoothing (smooth parameter is 0.0)" << std::endl;
    for (int j = 0; j <= numResample; ++j)
    {
        for (int i = 0; i <= numResample; ++i)
        {
            interp = noSmooth(invResample * i, invResample * j);
            output << interp << " ";
        }
        output << std::endl;
    }
    functional = noSmooth.ComputeFunctional();
    output << "functional = " << functional << std::endl << std::endl;

    // Increasing amounts of smoothing.
    smooth = 0.1;
    for (int k = 1; k <= 6; ++k, smooth *= 10.0)
    {
        IntpThinPlateSpline2<double> spline(
            numPoints, x.data(), y.data(), f.data(), smooth, false);
        output << "smoothing (parameter is " << smooth << ")" << std::endl;
        for (int j = 0; j <= numResample; ++j)
        {
            for (int i = 0; i <= numResample; ++i)
            {
                interp = spline(invResample * i, invResample * j);
                interp = noSmooth(invResample * i, invResample * j);
                output << interp << " ";
            }
            output << std::endl;
        }
        functional = noSmooth.ComputeFunctional();
        output << "functional = " << functional << std::endl << std::endl;
    }

    output.close();
}

static void TestThinPlateSplines3D()
{
    std::ofstream output("output3.txt");
    output << std::scientific << std::setprecision(8);

    // Tabulated data on a 3x3x3 regular grid, points (x,y,z,f(x,y,z)).
    std::default_random_engine dre;
    std::uniform_real_distribution<double> urd(0.0, 1.0);
    int const numPoints = 27;
    std::array<double, numPoints> x, y, z, f;
    double xdomain, ydomain, zdomain;
    for (int k = 0, index = 0; k < 3; ++k)
    {
        zdomain = 0.5 * k;
        for (int j = 0; j < 3; ++j)
        {
            ydomain = 0.5 * j;
            for (int i = 0; i < 3; ++i, ++index)
            {
                xdomain = 0.5 * i;
                x[index] = xdomain;
                y[index] = ydomain;
                z[index] = zdomain;
                f[index] = urd(dre);
            }
        }
    }

    // Resample on a 7x7x7 regular grid.
    int const numResample = 6;
    double const invResample = 1.0 / static_cast<double>(numResample);
    double smooth, interp, functional;

    // No smoothing, exact interpolation at grid points.
    smooth = 0.0;
    IntpThinPlateSpline3<double> noSmooth(
        numPoints, x.data(), y.data(), z.data(), f.data(), smooth, false);
    output << "no smoothing (smooth parameter is 0.0)" << std::endl;
    for (int k = 0; k <= numResample; ++k)
    {
        zdomain = invResample * k;
        for (int j = 0; j <= numResample; ++j)
        {
            ydomain = invResample * j;
            for (int i = 0; i <= numResample; ++i)
            {
                xdomain = invResample * i;
                interp = noSmooth(xdomain, ydomain, zdomain);
                output << interp << " ";
            }
            output << std::endl;
        }
        output << std::endl;
    }
    functional = noSmooth.ComputeFunctional();
    output << "functional = " << functional << std::endl << std::endl;

    // Increasing amounts of smoothing.
    smooth = 0.1;
    for (int ell = 1; ell <= 6; ++ell, smooth *= 10.0)
    {
        IntpThinPlateSpline3<double> spline(
            numPoints, x.data(), y.data(), z.data(), f.data(), smooth, false);
        output << "smoothing (parameter is " << smooth << ")" << std::endl;
        for (int k = 0; k <= numResample; ++k)
        {
            zdomain = invResample * k;
            for (int j = 0; j <= numResample; ++j)
            {
                ydomain = invResample * j;
                for (int i = 0; i <= numResample; ++i)
                {
                    xdomain = invResample * i;
                    interp = spline(xdomain, ydomain, zdomain);
                    output << interp << " ";
                }
                output << std::endl;
            }
            output << std::endl;
        }
        functional = noSmooth.ComputeFunctional();
        output << "functional = " << functional << std::endl << std::endl;
    }

    output.close();
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

    TestThinPlateSplines2D();
    TestThinPlateSplines3D();
    return 0;
}
