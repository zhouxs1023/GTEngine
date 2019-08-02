// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/04/17)

#pragma once

#include <Applications/GteWindow3.h>
using namespace gte;

class TextureUpdatingWindow : public Window3
{
public:
    TextureUpdatingWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();

    bool mForward;
    std::shared_ptr<Texture2> mTexture;
    std::shared_ptr<Visual> mSquare;
};
