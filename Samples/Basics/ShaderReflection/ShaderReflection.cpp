// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2017/06/29)

#include <LowLevel/GteLogReporter.h>
#include <fstream>
using namespace gte;

#if defined(GTE_DEV_OPENGL)

#if defined(__MSWINDOWS__)
#include <Graphics/GL4/WGL/GteWGLEngine.h>
#else
#include <Graphics/GL4/GLX/GteGLXEngine.h>
#endif
#include <Graphics/GL4/GteGLSLProgramFactory.h>
#include <Graphics/GL4/GteGLSLVisualProgram.h>
#include <Graphics/GL4/GteGLSLComputeProgram.h>

void ReflectVertexColoring()
{
#if defined(__MSWINDOWS__)
    WGLEngine engine(true, false);
#else
    GLXEngine engine(true, false);
#endif
    GLSLProgramFactory factory;
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(factory.CreateFromFiles(
        "Shaders/VertexColoringVS.glsl", "Shaders/VertexColoringPS.glsl", ""));
    if (program)
    {
        std::ofstream out("Shaders/VertexColoring.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
}

void ReflectTexturing()
{
#if defined(__MSWINDOWS__)
    WGLEngine engine(true, false);
#else
    GLXEngine engine(true, false);
#endif
    GLSLProgramFactory factory;
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(factory.CreateFromFiles(
        "Shaders/TexturingVS.glsl", "Shaders/TexturingPS.glsl", ""));
    if (program)
    {
        std::ofstream out("Shaders/Texturing.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
}

void ReflectBillboards()
{
#if defined(__MSWINDOWS__)
    WGLEngine engine(true, false);
#else
    GLXEngine engine(true, false);
#endif
    GLSLProgramFactory factory;
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(factory.CreateFromFiles(
        "Shaders/BillboardsVS.glsl", "Shaders/BillboardsPS.glsl", "Shaders/BillboardsGS.glsl"));
    if (program)
    {
        std::ofstream out("Shaders/Billboards.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
}

void ReflectNestedStruct()
{
#if defined(__MSWINDOWS__)
    WGLEngine engine(true, false);
#else
    GLXEngine engine(true, false);
#endif
    GLSLProgramFactory factory;
    auto program = std::dynamic_pointer_cast<GLSLComputeProgram>(factory.CreateFromFile(
        "Shaders/NestedStruct.glsl"));
    if (program)
    {
        std::ofstream out("Shaders/NestedStruct.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
}

void ReflectTextureArrays()
{
#if defined(__MSWINDOWS__)
    WGLEngine engine(true, false);
#else
    GLXEngine engine(true, false);
#endif
    GLSLProgramFactory factory;
    auto program = std::dynamic_pointer_cast<GLSLVisualProgram>(factory.CreateFromFiles(
        "Shaders/TextureArraysVS.glsl", "Shaders/TextureArraysPS.glsl", ""));
    if (program)
    {
        std::ofstream out("Shaders/TextureArrays.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
}

void ReflectSimpleBuffers()
{
#if defined(__MSWINDOWS__)
    WGLEngine engine(true, false);
#else
    GLXEngine engine(true, false);
#endif
    GLSLProgramFactory factory;
    auto program = std::dynamic_pointer_cast<GLSLComputeProgram>(factory.CreateFromFile(
        "Shaders/SimpleBuffers.glsl"));
    if (program)
    {
        std::ofstream out("Shaders/SimpleBuffers.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
}

void ReflectAppendConsume()
{
#if defined(__MSWINDOWS__)
    WGLEngine engine(true, false);
#else
    GLXEngine engine(true, false);
#endif
    GLSLProgramFactory factory;
    auto program = std::dynamic_pointer_cast<GLSLComputeProgram>(factory.CreateFromFile(
        "Shaders/AppendConsume.glsl"));
    if (program)
    {
        std::ofstream out("Shaders/AppendConsume.glslreflect.txt");
        if (out)
        {
            auto& reflection = program->GetReflector();
            reflection.Print(out);
            out.close();
        }
    }
}

#else

#include <Graphics/DX11/GteHLSLFactory.h>

unsigned int const gCompileFlags =
    D3DCOMPILE_ENABLE_STRICTNESS |
    D3DCOMPILE_IEEE_STRICTNESS |
    D3DCOMPILE_PACK_MATRIX_ROW_MAJOR |
    D3DCOMPILE_OPTIMIZATION_LEVEL3;

void ReflectVertexColoring()
{
    HLSLShader vshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/VertexColoring.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Shaders/VertexColoring.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLShader pshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/VertexColoring.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Shaders/VertexColoring.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
}

void ReflectTexturing()
{
    HLSLShader vshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Texturing.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Shaders/Texturing.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLShader pshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Texturing.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Shaders/Texturing.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
}

void ReflectBillboards()
{
    HLSLShader vshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Billboards.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Shaders/Billboards.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLShader gshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Billboards.hlsl",
        "GSMain",
        "gs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (gshader.IsValid())
    {
        std::ofstream gsout("Shaders/Billboards.gsreflect.txt");
        if (gsout)
        {
            gshader.Print(gsout);
            gsout.close();
        }
    }

    HLSLShader pshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/Billboards.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Shaders/Billboards.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
}

void ReflectNestedStruct()
{
    HLSLShader cshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/NestedStruct.hlsl",
        "CSMain",
        "cs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout("Shaders/NestedStruct.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
}

void ReflectTextureArrays()
{
    HLSLShader vshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/TextureArrays.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Shaders/TextureArrays.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLShader pshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/TextureArrays.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Shaders/TextureArrays.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
}

void ReflectAppendConsume()
{
    HLSLShader cshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/AppendConsume.hlsl",
        "CSMain",
        "cs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout("Shaders/AppendConsume.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
}

void ReflectSimpleBuffers()
{
    HLSLShader cshader = HLSLShaderFactory::CreateFromFile(
        "Shaders/SimpleBuffers.hlsl",
        "CSMain",
        "cs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout("Shaders/SimpleBuffers.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
}

#endif

int main(int, char const*[])
{
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);

    ReflectVertexColoring();
    ReflectTexturing();
    ReflectBillboards();
    ReflectNestedStruct();
    ReflectTextureArrays();
    ReflectSimpleBuffers();
    ReflectAppendConsume();

    return 0;
}
