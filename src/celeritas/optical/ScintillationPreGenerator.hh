//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/optical/ScintillationPreGenerator.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Assert.hh"
#include "corecel/Macros.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/phys/ParticleTrackView.hh"
#include "celeritas/random/distribution/NormalDistribution.hh"
#include "celeritas/random/distribution/PoissonDistribution.hh"
#include "celeritas/track/SimTrackView.hh"

#include "OpticalDistributionData.hh"
#include "OpticalGenData.hh"
#include "OpticalPropertyData.hh"
#include "ScintillationData.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Sample the number of scintillation photons to be generated.
 *
 * This populates the \c OpticalDistributionData used by the \c
 * ScintillationGenerator to generate optical photons using post-step and
 * cached pre-step data.
 */
class ScintillationPreGenerator
{
  public:
    // Construct with optical properties, scintillation, and step data
    inline CELER_FUNCTION
    ScintillationPreGenerator(ParticleTrackView const& particle,
                              SimTrackView const& sim,
                              Real3 const& pos,
                              OpticalMaterialId optmat_id,
                              units::MevEnergy energy_deposition,
                              NativeCRef<ScintillationData> const& shared,
                              OpticalPreStepData const& step_data);

    // Populate an optical distribution data for the Scintillation Generator
    template<class Generator>
    inline CELER_FUNCTION OpticalDistributionData operator()(Generator& rng);

  private:
    units::ElementaryCharge charge_;
    real_type step_length_;
    OpticalMaterialId optmat_id_;
    OpticalPreStepData pre_step_;
    OpticalStepData post_step_;
    NativeCRef<ScintillationData> const& shared_;
    real_type mean_num_photons_;
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Construct with input parameters.
 */
CELER_FUNCTION ScintillationPreGenerator::ScintillationPreGenerator(
    ParticleTrackView const& particle,
    SimTrackView const& sim,
    Real3 const& pos,
    OpticalMaterialId optmat_id,
    units::MevEnergy energy_deposition,
    NativeCRef<ScintillationData> const& shared,
    OpticalPreStepData const& step_data)
    : charge_(particle.charge())
    , step_length_(sim.step_length())
    , optmat_id_(optmat_id)
    , pre_step_(step_data)
    , post_step_({particle.speed(), pos})
    , shared_(shared)
{
    CELER_EXPECT(step_length_ > 0);
    CELER_EXPECT(optmat_id_);
    CELER_EXPECT(shared_);
    CELER_EXPECT(pre_step_);

    if (shared_.scintillation_by_particle())
    {
        // TODO: implement sampling for particles, assert particle data, and
        // cache mean number of photons
        CELER_ASSERT_UNREACHABLE();
    }
    else
    {
        // Scintillation will be performed on materials only
        CELER_ASSERT(optmat_id_ < shared_.materials.size());
        auto const& material = shared_.materials[optmat_id_];

        // TODO: Use visible energy deposition when Birks law is implemented
        mean_num_photons_ = material ? material.yield_per_energy
                                           * energy_deposition.value()
                                     : 0;
    }
}

//---------------------------------------------------------------------------//
/*!
 * Return an \c OpticalDistributionData object. If no photons are sampled, an
 * empty object is returned and can be verified via its own operator bool.
 */
template<class Generator>
CELER_FUNCTION OpticalDistributionData
ScintillationPreGenerator::operator()(Generator& rng)
{
    // Material-only sampling
    OpticalDistributionData result;
    if (mean_num_photons_ > 10)
    {
        real_type sigma = shared_.resolution_scale[optmat_id_]
                          * std::sqrt(mean_num_photons_);
        result.num_photons = clamp_to_nonneg(
            NormalDistribution<real_type>(mean_num_photons_, sigma)(rng)
            + real_type{0.5});
    }
    else if (mean_num_photons_ > 0)
    {
        result.num_photons
            = PoissonDistribution<real_type>(mean_num_photons_)(rng);
    }

    if (result.num_photons > 0)
    {
        // Assign remaining data
        result.time = pre_step_.time;
        result.step_length = step_length_;
        result.charge = charge_;
        result.material = optmat_id_;
        result.points[StepPoint::pre].speed = pre_step_.speed;
        result.points[StepPoint::pre].pos = pre_step_.pos;
        result.points[StepPoint::post] = post_step_;
    }
    return result;
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
