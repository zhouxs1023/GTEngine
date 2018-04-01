#include <Graphics/DX11/GteHLSLProgramFactory.h>
#include <Mathematics/GteMatrix4x4.h>
#include "ContourFinderEffect.h"
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

ContourFinderEffect::ContourFinderEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::string const& vsPath, std::string const& psPath, std::string const& gsPath,
    std::shared_ptr<ConstantBuffer> const& meshID,
    uint32_t windowWidth, uint32_t windowHeight, float lineThickness,
    std::shared_ptr<ConstantBuffer> const& pvwMatrix,
    std::shared_ptr<StructuredBuffer> const& vertices)
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
        auto windowExtentBuffer = std::make_shared<ConstantBuffer>(2 * sizeof(float), false);
        float* windowExtent = windowExtentBuffer->Get<float>();
        windowExtent[0] = 0.5f * windowWidth;
        windowExtent[1] = 0.5f * windowHeight;

        auto lineThicknessBuffer = std::make_shared<ConstantBuffer>(sizeof(float), false);
        *lineThicknessBuffer->Get<float>() = lineThickness;

        auto gshader = mProgram->GetGShader();
        gshader->Set("MeshID", meshID);
        gshader->Set("WindowExtent", windowExtentBuffer);
        gshader->Set("LineThickness", lineThicknessBuffer);
        gshader->Set("PVWMatrix", pvwMatrix);
        gshader->Set("vertices", vertices);
    }
}
