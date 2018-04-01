#include <Graphics/DX11/GteHLSLProgramFactory.h>
#include <Mathematics/GteMatrix4x4.h>
#include "ContourDrawerEffect.h"
using namespace gte;

static ID3DBlob* LoadShaderBlob(std::string const& filename)
{
    std::ifstream input(filename, std::ios::binary);
    if (!input)
    {
        return nullptr;
    }

    // Get the number of bytes in the file.
    std::streampos fpos = input.tellg();
    input.seekg(0, std::ios::end);
    fpos = input.tellg() - fpos;
    SIZE_T numBytes = static_cast<SIZE_T>(fpos);
    input.seekg(0, std::ios::beg);

    ID3DBlob* compiled = nullptr;
    HRESULT hr = D3DCreateBlob(numBytes, &compiled);
    if (FAILED(hr))
    {
        input.close();
        return nullptr;
    }

    if (input.read((char*)compiled->GetBufferPointer(), compiled->GetBufferSize()).bad())
    {
        compiled->Release();
        return nullptr;
    }

    return compiled;
}

ContourDrawerEffect::ContourDrawerEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::string const& vsPath, std::string const& psPath, std::string const& gsPath,
    std::shared_ptr<ConstantBuffer> const& meshID,
    std::shared_ptr<ConstantBuffer> const& pvwMatrix,
    std::shared_ptr<StructuredBuffer> const& vertices,
    std::shared_ptr<Texture2> const& meshTriangleIDTexture,
    std::shared_ptr<Texture2> const& baseTexture,
    std::shared_ptr<SamplerState> const& baseSampler)
{
    auto hfactory = std::static_pointer_cast<HLSLProgramFactory>(factory);

    ID3DBlob* vsBlob = LoadShaderBlob(vsPath);
    std::vector<uint8_t> vsBytecode(vsBlob->GetBufferSize());
    memcpy(vsBytecode.data(), vsBlob->GetBufferPointer(), vsBytecode.size());

    ID3DBlob* psBlob = LoadShaderBlob(psPath);
    std::vector<uint8_t> psBytecode(psBlob->GetBufferSize());
    memcpy(psBytecode.data(), psBlob->GetBufferPointer(), psBytecode.size());

    ID3DBlob* gsBlob = LoadShaderBlob(gsPath);
    std::vector<uint8_t> gsBytecode(gsBlob->GetBufferSize());
    memcpy(gsBytecode.data(), gsBlob->GetBufferPointer(), gsBytecode.size());

    mProgram = hfactory->CreateFromBytecode(vsBytecode, psBytecode, gsBytecode);

    if (mProgram)
    {
        auto gshader = mProgram->GetGShader();
        auto pshader = mProgram->GetPShader();
        gshader->Set("MeshID", meshID);
        gshader->Set("PVWMatrix", pvwMatrix);
        gshader->Set("vertices", vertices);
        pshader->Set("attributesTexture", meshTriangleIDTexture);
        pshader->Set("baseTexture", baseTexture);
        pshader->Set("baseSampler", baseSampler);
    }
}
