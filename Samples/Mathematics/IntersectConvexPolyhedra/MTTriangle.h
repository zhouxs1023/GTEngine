// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.0 (2019/04/09)

#pragma once

#include <array>

class MTTriangle
{
public:
    MTTriangle(int label = -1)
        :
        mLabel(label)
    {
        mVertex.fill(-1);
        mEdge.fill(-1);
        mAdjacent.fill(-1);
    }

    inline int GetLabel() const
    {
        return mLabel;
    }

    inline void SetLabel(int label)
    {
        mLabel = label;
    }

    inline int GetVertex(int i) const
    {
        return mVertex[i];
    }

    inline void SetVertex(int i, int label)
    {
        mVertex[i] = label;
    }

    bool ReplaceVertex(int vOld, int vNew)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (mVertex[i] == vOld)
            {
                mVertex[i] = vNew;
                return true;
            }
        }
        return false;
    }

    inline int GetEdge(int i) const
    {
        return mEdge[i];
    }

    inline void SetEdge(int i, int label)
    {
        mEdge[i] = label;
    }

    bool ReplaceEdge(int eOld, int eNew)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (mEdge[i] == eOld)
            {
                mEdge[i] = eNew;
                return true;
            }
        }

        return false;
    }

    inline int GetAdjacent(int i) const
    {
        return mAdjacent[i];
    }

    inline void SetAdjacent(int i, int label)
    {
        mAdjacent[i] = label;
    }

    bool ReplaceAdjacent(int aOld, int aNew)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (mAdjacent[i] == aOld)
            {
                mAdjacent[i] = aNew;
                return true;
            }
        }

        return false;
    }

    bool operator==(MTTriangle const& other) const
    {
        if (mVertex[0] == other.mVertex[0])
        {
            return mVertex[1] == other.mVertex[1] && mVertex[2] == other.mVertex[2];
        }

        if (mVertex[0] == other.mVertex[1])
        {
            return mVertex[1] == other.mVertex[2] && mVertex[2] == other.mVertex[0];
        }

        if (mVertex[0] == other.mVertex[2])
        {
            return mVertex[1] == other.mVertex[0] && mVertex[2] == other.mVertex[1];
        }

        return false;
    }

protected:
    int mLabel;
    std::array<int, 3> mVertex;
    std::array<int, 3> mEdge;
    std::array<int, 3> mAdjacent;
};
