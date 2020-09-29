//----------------------------------*-C++-*----------------------------------//
// Copyright 2020 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Types.hh
//---------------------------------------------------------------------------//
#pragma once

#include "base/OpaqueId.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//

using EventId = OpaqueId<struct Event>;
using TrackId = OpaqueId<struct Track>;

//---------------------------------------------------------------------------//
} // namespace celeritas
