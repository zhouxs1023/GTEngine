// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2017/07/06)

#include <cstdlib>
#include <iostream>
#include "ProjectTemplate.v12.h"
#include "ProjectTemplate.v14.h"
#include "ProjectTemplate.v15.h"

int main(int numArguments, char* arguments[])
{
    if (3 != numArguments)
    {
        std::cout <<
            "usage: GenerateProject nesting projname" << std::endl;
        std::cout <<
            "Nesting is number of levels from GTEngine folder" << std::endl;
        std::cout << "Example: nesting is 3 for ";
        std::cout << "GTEngine/Samples/Basics/VertexColoring" << std::endl;
        return -1;
    }

    int nesting = atoi(arguments[1]);
    if (nesting <= 0)
    {
        std::cout << "Nesting must be positive" << std::endl;
        return -2;
    }

    // Generate the relative path to GTEngine.v<version>.vcxproj.
    std::string gtPath;
    for (int i = 0; i < nesting; ++i)
    {
        gtPath += "..\\";
    }

    // Generate the files for the project.
    std::string name = arguments[2];
    bool success = false;
    TemplateV12 generatev12(name, gtPath, success);
    if (!success)
    {
        std::cout << "Could not create the V12 project files." << std::endl;
        return -5;
    }

    TemplateV14 generatev14(name, gtPath, success);
    if (!success)
    {
        std::cout << "Could not create the V14 project files." << std::endl;
        return -6;
    }

    TemplateV15 generatev15(name, gtPath, success);
    if (!success)
    {
        std::cout << "Could not create the V15 project files." << std::endl;
        return -7;
    }

    return 0;
}

