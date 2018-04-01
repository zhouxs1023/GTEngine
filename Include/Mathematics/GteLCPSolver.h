// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.4.0 (2016/11/08)

#pragma once

#include <Mathematics/GtePolynomial1.h>
#include <algorithm>
#include <array>

// The comments at the end of this file explain what the preprocessor symbol
// means regarding the LCP solver implementation.
//#define GTE_REPORT_FAILED_TO_CONVERGE

namespace gte
{

template <typename Real, int n>
class LCPSolver
{
public:
    // A class for solving the Linear Complementarity Problem (LCP)
    // w = q + M * z, w^T * z = 0, w >= 0, z >= 0.  The vectors q, w, and z
    // are n-tuples and the matrix M is n-by-n.  The inputs are q and M.
    // The outputs are w and z, which are valid when the returned bool is
    // true but are invalid when the returned bool is false.

    enum Result
    {
        HAS_TRIVIAL_SOLUTION,
        HAS_NONTRIVIAL_SOLUTION,
        NO_SOLUTION,
        FAILED_TO_CONVERGE
    };

    // If you want to know specifically why 'true' or 'false' was returned,
    // pass the address of a Result variable as the last parameter.
    bool Solve(std::array<Real, n> const& q, std::array<std::array<Real, n>, n> const& M,
        std::array<Real, n>& w, std::array<Real, n>& z, Result* result = nullptr);

private:
    // Support for polynomial with n+1 coefficients and degree no larger
    // than n.
    static void MakeZero(Real* poly);
    static void Copy(Real const* poly0, Real* poly1);
    static bool LessThan(Real const* poly0, Real const* poly1);
    static void Multiply(Real const* poly, Real scalar, Real* product);

    // Bookkeeping of variables during the iterations of the solver.  The name
    // is either 'w' or 'z' and is used for human-readable debugging help.
    // The 'index' is that for the original variables w[index] or z[index].
    // The 'complementary' index is the location of the complementary variable
    // in mVarBasic[] or in mVarNonbasic[].  The 'tuple' is a pointer to
    // &w[0] or &z[0], the choice based on name of 'w' or 'z', and is used to
    // fill in the solution values (the variables are permuted during the
    // pivoting algorithm).
    struct Variable
    {
        char name;
        int index;
        int complementary;
        Real* tuple;
    };

    Variable mVarBasic[n + 1];
    Variable mVarNonbasic[n + 1];

    // The augmented problem is w = q + M*z + z[n]*U = 0, where U is an
    // n-tuple of 1-values.  We manipulate the augmented matrix [M | U | p(t)]
    // where p(t) is a column vector of polynomials of at most degree n.  If
    // p[r](t) is the polynomial for row r, then p[r](0) = q[r].  These are
    // perturbations of q[r] designed so that the algorithm avoids degeneracies
    // (a q-term becomes zero during the iterations).  The basic variables are
    // w[0] through w[n-1] and the nonbasic variables are z[0] through z[n].
    // The returned z consists only of z[0] through z[n-1].
    Real mAugmented[n][2 * (n + 1)];
};

template <typename Real, int n>
bool LCPSolver<Real, n>::Solve(
    std::array<Real, n> const& q, std::array<std::array<Real, n>, n> const& M,
    std::array<Real, n>& w, std::array<Real, n>& z, Result* result)
{
    // Perturb the q[r] constants to be polynomials of degree r+1 represented
    // as an array of n+1 coefficients.  The coefficient with index r+1 is 1
    // and the coefficients with indices larger than r+1 are 0.
    Real* p[n];
    for (int r = 0; r < n; ++r)
    {
        p[r] = &mAugmented[r][n + 1];
        MakeZero(p[r]);
        p[r][0] = q[r];
        p[r][r + 1] = (Real)1;
    }

    // Determine whether there is the trivial solution w = z = 0.
    Real qmin[n + 1];
    Copy(p[0], qmin);
    int basic = 0;
    for (int r = 1; r < n; ++r)
    {
        if (LessThan(p[r], qmin))
        {
            Copy(p[r], qmin);
            basic = r;
        }
    }

    Real zero[n + 1];
    MakeZero(zero);
    if (!LessThan(qmin, zero))
    {
        w = q;
        z.fill((Real)0);
        if (result)
        {
            *result = HAS_TRIVIAL_SOLUTION;
        }
        return true;
    }

    // Initialize the remainder of the augmented matrix with M and U.
    for (int r = 0; r < n; ++r)
    {
        for (int c = 0; c < n; ++c)
        {
            mAugmented[r][c] = M[r][c];
        }
        mAugmented[r][n] = (Real)1;
    }

    // Keep track of when the variables enter and exit the dictionary,
    // including where complementary variables are relocated.
    for (int i = 0; i <= n; ++i)
    {
        mVarBasic[i].name = 'w';
        mVarBasic[i].index = i;
        mVarBasic[i].complementary = i;
        mVarBasic[i].tuple = w.data();
        mVarNonbasic[i].name = 'z';
        mVarNonbasic[i].index = i;
        mVarNonbasic[i].complementary = i;
        mVarNonbasic[i].tuple = z.data();
    }

    // The augmented variable z[n] is the initial driving variable for
    // pivoting.  The equation 'basic' is the one to solve for z[n] and
    // pivoting with w[basic].  The last column of M remains all 1-values
    // for this initial step, so no algebraic computations occur for M[r][n].
    int driving = n;
    for (int r = 0; r < n; ++r)
    {
        if (r != basic)
        {
            for (int c = 0; c < 2 * (n + 1); ++c)
            {
                if (c != n)
                {
                    mAugmented[r][c] -= mAugmented[basic][c];
                }
            }
        }
    }

    for (int c = 0; c < 2 * (n + 1); ++c)
    {
        if (c != n)
        {
            mAugmented[basic][c] = -mAugmented[basic][c];
        }
    }

    for (int i = 0; i <= n+1; ++i)
    {
        // The basic variable of equation 'basic' exited the dictionary, so
        // its complementary (nonbasic) variable must become the next driving
        // variable in order for it to enter the dictionary.
        int nextDriving = mVarBasic[basic].complementary;
        mVarNonbasic[nextDriving].complementary = driving;
        std::swap(mVarBasic[basic], mVarNonbasic[driving]);
        if (mVarNonbasic[driving].index == n)
        {
            // The algorithm has converged.
            for (int r = 0; r < n; ++r)
            {
                mVarBasic[r].tuple[mVarBasic[r].index] = p[r][0];
            }
            for (int c = 0; c <= n; ++c)
            {
                int index = mVarNonbasic[c].index;
                if (index < n)
                {
                    mVarNonbasic[c].tuple[index] = (Real)0;
                }
            }
            if (result)
            {
                *result = HAS_NONTRIVIAL_SOLUTION;
            }
            return true;
        }

        // Determine the 'basic' equation for which the ratio -q[r]/M(r,driving)
        // is minimized among all equations r with M(r,driving) < 0.
        Real minRatio[n + 1], ratio[n + 1];
        driving = nextDriving;
        basic = -1;
        for (int r = 0; r < n; ++r)
        {
            if (mAugmented[r][driving] < (Real)0)
            {
                Real factor = (Real)-1 / mAugmented[r][driving];
                Multiply(p[r], factor, ratio);
                if (basic == -1 || LessThan(ratio, minRatio))
                {
                    Copy(ratio, minRatio);
                    basic = r;
                }
            }
        }

        if (basic == -1)
        {
            // The coefficients of z[driving] in all the equations are
            // nonnegative, so the z[driving] variable cannot leave the
            // dictionary.  There is no solution to the LCP.
            w.fill((Real)0);
            z.fill((Real)0);
            if (result)
            {
                *result = NO_SOLUTION;
            }
            return false;
        }

        // Solve the basic equation so that z[driving] enters the dictionary
        // and w[basic] exits the dictionary.
        Real invDenom = (Real)1 / mAugmented[basic][driving];
        for (int r = 0; r < n; ++r)
        {
            if (r != basic && mAugmented[r][driving] != (Real)0)
            {
                Real multiplier = mAugmented[r][driving] * invDenom;
                for (int c = 0; c < 2 * (n + 1); ++c)
                {
                    if (c != driving)
                    {
                        mAugmented[r][c] -= mAugmented[basic][c] * multiplier;
                    }
                    else
                    {
                        mAugmented[r][driving] = multiplier;
                    }
                }
            }
        }

        for (int c = 0; c < 2 * (n + 1); ++c)
        {
            if (c != driving)
            {
                mAugmented[basic][c] = -mAugmented[basic][c] * invDenom;
            }
            else
            {
                mAugmented[basic][driving] = invDenom;
            }
        }
    }

    // Numerical round-off errors can cause the Lemke algorithm not to
    // converge.  In particular, the code above has a test
    //   if (mAugmented[r][driving] < (Real)0) { ... }
    // to determine the 'basic' equation with which to pivot.  It is
    // possible that theoretically mAugmented[r][driving]is zero but
    // rounding errors cause it to be slightly negative.  If theoretically
    // all mAugmented[r][driving] >= 0, there is no solution to the LCP.
    // With the rounding errors, if the algorithm fails to converge within
    // the specified number of iterations, NO_SOLUTION is returned, which
    // is hopefully the correct result. It is also possible that the rounding
    // errors lead to a NO_SOLUTION (returned from inside the loop) when in
    // fact there is a solution. [When the LCP solver is used by intersection
    // testing algorithms, the hope is that misclassifications occur only
    // when the two objects are nearly in tangential contact.]
    //
    // To determine whether the rounding errors are the problem, you can
    // execute the query using exact arithmetic with the following type
    // used for 'Real' (replacing 'float' or 'double') of
    // BSRational<UIntegerAP32> Rational.
    //
    // That said, if the algorithm fails to converge and you believe that
    // the rounding errors are not causing this, please file a bug report
    // and provide the input data to the solver.

#if defined(GTE_REPORT_FAILED_TO_CONVERGE)
    LogInformation("The algorithm failed to converge.");
#endif

    if (result)
    {
        *result = FAILED_TO_CONVERGE;
    }
    return false;
}

template <typename Real, int n>
void LCPSolver<Real, n>::MakeZero(Real* poly)
{
    for (int i = 0; i <= n; ++i)
    {
        poly[i] = (Real)0;
    }
}

template <typename Real, int n>
void LCPSolver<Real, n>::Copy(Real const* poly0, Real* poly1)
{
    for (int i = 0; i <= n; ++i)
    {
        poly1[i] = poly0[i];
    }
}

template <typename Real, int n>
bool LCPSolver<Real, n>::LessThan(Real const* poly0, Real const* poly1)
{
    for (int i = 0; i <= n; ++i)
    {
        if (poly0[i] < poly1[i])
        {
            return true;
        }

        if (poly0[i] > poly1[i])
        {
            return false;
        }
    }

    return false;
}

template <typename Real, int n>
void LCPSolver<Real, n>::Multiply(Real const* poly, Real scalar, Real* product)
{
    for (int i = 0; i <= n; ++i)
    {
        product[i] = poly[i] * scalar;
    }
}

}
