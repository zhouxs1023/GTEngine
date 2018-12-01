// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2018/11/29)

#pragma once

#include <LowLevel/GteLogger.h>
#include <Mathematics/GteMath.h>
#include <Mathematics/GteRay.h>

// An acute cone is Dot(A,X-V) = |X-V| cos(t) where V is the vertex, A is the
// unit-length direction of the axis of the cone, and T is the cone angle with
// 0 < t < pi/2.  The cone interior is defined by the inequality
// Dot(A,X-V) >= |X-V| cos(t).  Since cos(t) > 0, we can avoid computing
// square roots.  The solid cone is defined by the inequality
// Dot(A,X-V)^2 >= Dot(X-V,X-V) cos(t)^2.  This is an infinite, single-sided
// cone.
//
// The cone may be truncated by a plane perpendicular to its axis at a height
// h from the vertex (distance from the vertex to the intersection of the
// plane and the axis).  The infinite cone has h = infinity.  The finite cone
// has a disk of intersection between the plane and infinite cone.  The radius
// r of the disk is r = h*tan(t).

namespace gte
{
    template <int N, typename Real>
    class Cone
    {
    public:
        // The default constructor creates an infinite cone with
        //   center = (0,...,0)
        //   axis = (0,...,0,1)
        //   angle = pi/4
        //   minimum height = 0
        //   maximum height = std::numeric_limits<Real>::max()
        Cone()
            :
            minHeight((Real)0),
            maxHeight(std::numeric_limits<Real>::max())
        {
            ray.origin.MakeZero();
            ray.direction.MakeUnit(N - 1);
            SetAngle((Real)GTE_C_QUARTER_PI);
        }

        // This constructor creates an infinite cone with
        //   center = inRay.origin
        //   axis = inRay.direction
        //   angle = inAngle
        //   minimum height = 0
        //   maximum height = std::numeric_limits<Real>::max()
        Cone(Ray<N, Real> const& inRay, Real inAngle)
            :
            ray(inRay),
            minHeight((Real)0),
            maxHeight(std::numeric_limits<Real>::max())
        {
            SetAngle(inAngle);
        }

        // This constructor creates a finite cone (cone frustum) with
        //   center = inRay.origin
        //   axis = inRay.direction
        //   angle = inAngle
        //   minimum height = inMinHeight
        //   maximum height = inMaxHeight
        Cone(Ray<N, Real> const& inRay, Real inAngle, Real inMinHeight, Real inMaxHeight)
            :
            ray(inRay),
            minHeight(inMinHeight),
            maxHeight(inMaxHeight)
        {
            LogAssert((Real)0 <= minHeight && minHeight < maxHeight, "Invalid height interval.");
            SetAngle(inAngle);
        }

        // The angle must be in (0,pi/2).  The function sets 'angle' and
        // computes 'cosAngle', 'sinAngle' and 'cosAngleSqr'.
        void SetAngle(Real inAngle)
        {
            angle = inAngle;
            cosAngle = std::cos(angle);
            sinAngle = std::sin(angle);
            cosAngleSqr = cosAngle * cosAngle;
        }

        // The cone vertex is the ray origin and the cone axis direction is
        // the ray direction.  The direction must be unit length.  The angle
        // must be in (0,pi/2).  The heights must satisfy
        // 0 <= minHeight < maxHeight <= std::numeric_limits<Real>::max().
        Ray<N, Real> ray;
        Real angle;
        Real minHeight, maxHeight;

        // Members derived from 'angle', to avoid calling trigonometric
        // functions in geometric queries (for speed).  You may set 'angle'
        // and compute these by calling SetAngle(inAngle).
        Real cosAngle, sinAngle, cosAngleSqr;

    public:
        // Comparisons to support sorted containers.  These based only on
        // 'ray', 'angle', 'minHeight' and 'maxHeight'.
        bool operator==(Cone const& cone) const
        {
            return ray == cone.ray
                && angle == cone.angle
                && minHeight == cone.minHeight
                && maxHeight == cone.maxHeight;
        }

        bool operator!=(Cone const& cone) const
        {
            return !operator==(cone);
        }

        bool operator< (Cone const& cone) const
        {
            if (ray < cone.ray)
            {
                return true;
            }

            if (ray > cone.ray)
            {
                return false;
            }

            if (angle < cone.angle)
            {
                return true;
            }

            if (angle > cone.angle)
            {
                return false;
            }

            if (minHeight < cone.minHeight)
            {
                return true;
            }

            if (minHeight > cone.minHeight)
            {
                return false;
            }

            return maxHeight < cone.maxHeight;
        }

        bool operator<=(Cone const& cone) const
        {
            return !cone.operator<(*this);
        }

        bool operator> (Cone const& cone) const
        {
            return cone.operator<(*this);
        }

        bool operator>=(Cone const& cone) const
        {
            return !operator<(cone);
        }
    };

    // Template alias for convenience.
    template <typename Real>
    using Cone3 = Cone<3, Real>;
}
