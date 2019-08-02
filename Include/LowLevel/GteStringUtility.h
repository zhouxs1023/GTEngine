// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.26.0 (2019/07/30)

#pragma once

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>

namespace gte
{
    inline static std::wstring ConvertNarrowToWide(std::string const& input)
    {
        std::wstring output;
        std::transform(input.begin(), input.end(), std::back_inserter(output),
            [](char c) { return static_cast<wchar_t>(c); });
        return output;
    }

    inline static std::string ConvertWideToNarrow(std::wstring const& input)
    {
        std::string output;
        std::transform(input.begin(), input.end(), std::back_inserter(output),
            [](wchar_t c) { return static_cast<char>(c); });
        return output;
    }

    inline static std::string ToLower(std::string const& input)
    {
        std::string output;
        std::transform(input.begin(), input.end(), std::back_inserter(output),
            [](int c) { return static_cast<char>(::tolower(c)); });
        return output;
    }

    inline static std::string ToUpper(std::string const& input)
    {
        std::string output;
        std::transform(input.begin(), input.end(), std::back_inserter(output),
            [](int c) { return static_cast<char>(::toupper(c)); });
        return output;
    }
}
