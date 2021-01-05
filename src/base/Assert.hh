//----------------------------------*-C++-*----------------------------------//
// Copyright 2020 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Assert.hh
//---------------------------------------------------------------------------//
#pragma once

#include "celeritas_config.h"
#include "Macros.hh"
#ifndef __CUDA_ARCH__
#    include <stdexcept>
#endif

//---------------------------------------------------------------------------//
// MACROS
//---------------------------------------------------------------------------//
/*!
 * \def REQUIRE
 *
 * Precondition debug assertion macro. It is to "require" that the input values
 * or initial state satisfy a precondition.
 */
/*!
 * \def CHECK
 *
 * Internal debug assertion macro. This replaces standard \c assert usage.
 */
/*!
 * \def ENSURE
 *
 * Postcondition debug assertion macro. Use to "ensure" that return values or
 * side effects are as expected when leaving a function.
 */
/*!
 * \def CHECK_UNREACHABLE
 *
 * Assert if the code point is reached. When debug assertions are turned off,
 * this changes to a compiler hint that improves optimization.
 */

//! \cond
#define CELER_CUDA_ASSERT_(COND) \
    do                           \
    {                            \
        assert(COND);            \
    } while (0)
#define CELER_ASSERT_(COND)                                            \
    do                                                                 \
    {                                                                  \
        if (CELER_UNLIKELY(!(COND)))                                   \
            ::celeritas::throw_debug_error(#COND, __FILE__, __LINE__); \
    } while (0)
#define CELER_NOASSERT_(COND)   \
    do                          \
    {                           \
        if (false && (COND)) {} \
    } while (0)
//! \endcond

#if CELERITAS_DEBUG && defined(__CUDA_ARCH__)
#    define REQUIRE(x) CELER_CUDA_ASSERT_(x)
#    define CHECK(x) CELER_CUDA_ASSERT_(x)
#    define ENSURE(x) CELER_CUDA_ASSERT_(x)
#    define CHECK_UNREACHABLE CELER_CUDA_ASSERT_(false)
#elif CELERITAS_DEBUG && !defined(__CUDA_ARCH__)
#    define REQUIRE(x) CELER_ASSERT_(x)
#    define CHECK(x) CELER_ASSERT_(x)
#    define ENSURE(x) CELER_ASSERT_(x)
#    define CHECK_UNREACHABLE CELER_ASSERT_(false)
#else
#    define REQUIRE(x) CELER_NOASSERT_(x)
#    define CHECK(x) CELER_NOASSERT_(x)
#    define ENSURE(x) CELER_NOASSERT_(x)
#    define CHECK_UNREACHABLE CELER_UNREACHABLE
#endif

/*!
 * \def CELER_CUDA_CALL
 *
 * Execute the wrapped statement and throw a message if it fails.
 *
 * If it fails, we call \c cudaGetLastError to clear the error code.
 *
 * \code
 *     CELER_CUDA_CALL(cudaMalloc(&ptr_gpu, 100 * sizeof(float)));
 *     CELER_CUDA_CALL(cudaDeviceSynchronize());
 * \endcode
 */
#define CELER_CUDA_CALL(STATEMENT)                       \
    do                                                   \
    {                                                    \
        cudaError_t cuda_result_ = (STATEMENT);          \
        if (CELER_UNLIKELY(cuda_result_ != cudaSuccess)) \
        {                                                \
            cudaGetLastError();                          \
            ::celeritas::throw_cuda_call_error(          \
                cudaGetErrorString(cuda_result_),        \
                #STATEMENT,                              \
                __FILE__,                                \
                __LINE__);                               \
        }                                                \
    } while (0)

/*!
 * \def CELER_CUDA_CHECK_ERROR
 *
 * After a kernel launch or other call, check that no CUDA errors have
 * occurred. This is also useful for checking success after external CUDA
 * libraries have been called.
 */
#define CELER_CUDA_CHECK_ERROR() CELER_CUDA_CALL(cudaPeekAtLastError())

namespace celeritas
{
//---------------------------------------------------------------------------//
// FUNCTIONS
//---------------------------------------------------------------------------//
// Construct and throw a DebugError.
[[noreturn]] void
throw_debug_error(const char* condition, const char* file, int line);

// Construct and throw a RuntimeError for failed CUDA calls.
[[noreturn]] void throw_cuda_call_error(const char* error_string,
                                        const char* code,
                                        const char* file,
                                        int         line);

#ifndef __CUDA_ARCH__
//---------------------------------------------------------------------------//
// TYPES
//---------------------------------------------------------------------------//
/*!
 * Error thrown by Celeritas assertions.
 */
class DebugError : public std::logic_error
{
  public:
    //!@{
    //! Delegating constructor
    explicit DebugError(const char* msg);
    explicit DebugError(const std::string& msg);
    //!@}
};

//---------------------------------------------------------------------------//
/*!
 * Error thrown by working code from unexpected runtime conditions.
 */
class RuntimeError : public std::runtime_error
{
  public:
    //!@{
    //! Delegating constructor
    explicit RuntimeError(const char* msg);
    explicit RuntimeError(const std::string& msg);
    //!@}
};

#endif //__CUDA_ARCH__

//---------------------------------------------------------------------------//
} // namespace celeritas
