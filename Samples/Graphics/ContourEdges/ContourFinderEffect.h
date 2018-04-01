#pragma once

#include <Graphics/GteVisualEffect.h>
#include <Mathematics/GteVector2.h>

namespace gte
{

class ContourFinderEffect : public VisualEffect
{
public:
    // Construction.
    ContourFinderEffect(std::shared_ptr<ProgramFactory> const& factory,
        std::string const& vsPath, std::string const& psPath, std::string const& gsPath,
        std::shared_ptr<ConstantBuffer> const& meshID,
        uint32_t windowWidth, uint32_t windowHeight, float lineThickness,
        std::shared_ptr<ConstantBuffer> const& pvwMatrix,
        std::shared_ptr<StructuredBuffer> const& vertices);
};

}
