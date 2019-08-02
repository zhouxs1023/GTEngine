// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2019/07/31)

#pragma once

#include <Graphics/GteIndexBuffer.h>
#include <Graphics/GteTextEffect.h>
#include <Graphics/GteVertexBuffer.h>

namespace gte
{
    class GTE_IMPEXP Font
    {
    public:
        // Construction and destruction.
        virtual ~Font() = default;
        Font(std::shared_ptr<ProgramFactory> const& factory, unsigned int width, unsigned int height,
            char const* texels, float const* characterData, unsigned int maxMessageLength);

        // Member access.
        inline std::shared_ptr<VertexBuffer> const& GetVertexBuffer() const
        {
            return mVertexBuffer;
        }

        inline std::shared_ptr<IndexBuffer> const& GetIndexBuffer() const
        {
            return mIndexBuffer;
        }

        inline std::shared_ptr<TextEffect> const& GetTextEffect() const
        {
            return mTextEffect;
        }

        int GetHeight() const;
        int GetWidth(std::string const& message) const;

        // Populate the vertex buffer for the specified string.
        void Typeset(int viewportWidth, int viewportHeight, int x, int y,
            Vector4<float> const& color, std::string const& message) const;

    protected:
        unsigned int mMaxMessageLength;
        std::shared_ptr<VertexBuffer> mVertexBuffer;
        std::shared_ptr<IndexBuffer> mIndexBuffer;
        std::shared_ptr<Texture2> mTexture;
        std::shared_ptr<TextEffect> mTextEffect;
        float mCharacterData[257];
    };
}
