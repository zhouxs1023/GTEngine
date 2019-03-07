// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.0 (2019/02/04)

#pragma once

#include <Graphics/GteParticleController.h>
#include <random>

namespace gte
{
    class BloodCellController : public ParticleController
    {
    public:
        BloodCellController(std::shared_ptr<Camera> const&, Updater const& postUpdate);

    protected:
        virtual void UpdatePointMotion(float ctrlTime) override;

        std::default_random_engine mDRE;
        std::uniform_real_distribution<float> mURD;
    };
}
