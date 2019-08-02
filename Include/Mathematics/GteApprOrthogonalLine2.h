// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/07/30)

#pragma once

#include <Mathematics/GteVector2.h>
#include <Mathematics/GteApprQuery.h>
#include <Mathematics/GteLine.h>
#include <Mathematics/GteSymmetricEigensolver2x2.h>

// Least-squares fit of a line to (x,y) data by using distance measurements
// orthogonal to the proposed line. The return value is 'true' if and only
// if the fit is unique (always successful, 'true' when a minimum eigenvalue
// is unique). The mParameters value is a line with (P,D) =
// (origin,direction). The error for S = (x0,y0) is (S-P)^T*(I - D*D^T)*(S-P).

namespace gte
{
    template <typename Real>
    class ApprOrthogonalLine2 : public ApprQuery<Real, Vector2<Real>>
    {
    public:
        // Initialize the model parameters to zero.
        ApprOrthogonalLine2()
            :
            mParameters(Vector2<Real>::Zero(), Vector2<Real>::Zero())
        {
        }

        // Basic fitting algorithm. See ApprQuery.h for the various Fit(...)
        // functions that you can call.
        virtual bool FitIndexed(
            size_t numPoints, Vector2<Real> const* points,
            size_t numIndices, int const* indices) override
        {
            if (this->ValidIndices(numPoints, points, numIndices, indices))
            {
                // Compute the mean of the points.
                Vector2<Real> mean = Vector2<Real>::Zero();
                int const* currentIndex = indices;
                for (size_t i = 0; i < numIndices; ++i)
                {
                    mean += points[*currentIndex++];
                }
                mean /= (Real)numIndices;

                if (std::isfinite(mean[0]) && std::isfinite(mean[1]))
                {
                    // Compute the covariance matrix of the points.
                    Real covar00 = (Real)0, covar01 = (Real)0, covar11 = (Real)0;
                    currentIndex = indices;
                    for (size_t i = 0; i < numIndices; ++i)
                    {
                        Vector2<Real> diff = points[*currentIndex++] - mean;
                        covar00 += diff[0] * diff[0];
                        covar01 += diff[0] * diff[1];
                        covar11 += diff[1] * diff[1];
                    }

                    // Solve the eigensystem.
                    SymmetricEigensolver2x2<Real> es;
                    std::array<Real, 2> eval;
                    std::array<std::array<Real, 2>, 2> evec;
                    es(covar00, covar01, covar11, +1, eval, evec);

                    // The line direction is the eigenvector in the direction
                    // of largest variance of the points.
                    mParameters.origin = mean;
                    mParameters.direction = evec[1];

                    // The fitted line is unique when the maximum eigenvalue
                    // has multiplicity 1.
                    return eval[0] < eval[1];
                }
            }

            mParameters = Line2<Real>(Vector2<Real>::Zero(), Vector2<Real>::Zero());
            return false;
        }

        // Get the parameters for the best fit.
        Line2<Real> const& GetParameters() const
        {
            return mParameters;
        }

        virtual size_t GetMinimumRequired() const override
        {
            return 2;
        }

        virtual Real Error(Vector2<Real> const& point) const override
        {
            Vector2<Real> diff = point - mParameters.origin;
            Real sqrlen = Dot(diff, diff);
            Real dot = Dot(diff, mParameters.direction);
            Real error = std::fabs(sqrlen - dot * dot);
            return error;
        }

    private:
        Line2<Real> mParameters;
    };
}
