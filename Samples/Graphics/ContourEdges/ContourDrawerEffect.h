#pragma once

#include <Graphics/GteVisualEffect.h>

namespace gte
{

class ContourDrawerEffect : public VisualEffect
{
public:
    // Construction.
    ContourDrawerEffect(std::shared_ptr<ProgramFactory> const& factory,
        std::string const& vsPath, std::string const& psPath, std::string const& gsPath,
        std::shared_ptr<ConstantBuffer> const& meshID,
        std::shared_ptr<ConstantBuffer> const& pvwMatrix,
        std::shared_ptr<StructuredBuffer> const& vertices,
        std::shared_ptr<Texture2> const& meshTriangleIDTexture,
        std::shared_ptr<Texture2> const& baseTexture,
        std::shared_ptr<SamplerState> const& baseSampler);
};

}
