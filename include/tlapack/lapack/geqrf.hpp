/// @file geqrf.hpp
/// @author Thijs Steel, KU Leuven, Belgium
/// @note Adapted from @see
/// https://github.com/Reference-LAPACK/lapack/blob/master/SRC/zgeqrf.f
//
// Copyright (c) 2021-2023, University of Colorado Denver. All rights reserved.
//
// This file is part of <T>LAPACK.
// <T>LAPACK is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#ifndef TLAPACK_GEQRF_HH
#define TLAPACK_GEQRF_HH

#include "tlapack/base/utils.hpp"
#include "tlapack/lapack/geqr2.hpp"
#include "tlapack/lapack/larfb.hpp"
#include "tlapack/lapack/larft.hpp"

namespace tlapack {
/**
 * Options struct for gelqf
 */
template <class TT_t, class idx_t = size_t>
struct geqrf_opts_t : public workspace_opts_t<> {
    inline constexpr geqrf_opts_t(const workspace_opts_t<>& opts = {})
        : workspace_opts_t<>(opts){};

    idx_t nb = 32;    // Block size
    TT_t* TT = NULL;  // k-by-nb Matrix to hold the triangular factors
};

/** Worspace query of geqrf()
 *
 * @param[in] A m-by-n matrix.
 *
 * @param tau min(n,m) vector.
 *
 * @param[in] opts Options.
 *
 * @param[in,out] workinfo
 *      On output, the amount workspace required. It is larger than or equal
 *      to that given on input.
 *
 * @ingroup workspace_query
 */
template <typename A_t, typename tau_t, typename TT_t>
inline constexpr void geqrf_worksize(
    const A_t& A,
    const tau_t& tau,
    workinfo_t& workinfo,
    const geqrf_opts_t<TT_t, size_type<A_t>>& opts = {})
{
    using idx_t = size_type<A_t>;
    using T = type_t<A_t>;

    // constants
    const idx_t m = nrows(A);
    const idx_t n = ncols(A);
    const idx_t k = min(m, n);
    const idx_t nb = opts.nb;
    const idx_t ib = std::min<idx_t>(nb, k);

    auto A11 = cols(A, range<idx_t>(0, ib));
    auto TT1 = slice(A, range<idx_t>(0, ib), range<idx_t>(0, ib));
    auto A12 = slice(A, range<idx_t>(0, m), range<idx_t>(ib, n));
    auto tauw1 = slice(tau, range<idx_t>(0, ib));

    geqr2_worksize(A11, tauw1, workinfo);
    larfb_worksize(Side::Left, Op::ConjTrans, Direction::Forward,
                   StoreV::Columnwise, A11, TT1, A12, workinfo);

    if (opts.TT == NULL) {
        workinfo_t workinfo2(sizeof(T) * nb, nb);
        workinfo += workinfo2;
    }
}

/** Computes an QR factorization of an m-by-n matrix A using
 *  a blocked algorithm.
 *
 * The matrix Q is represented as a product of elementary reflectors.
 * \[
 *          Q = H(k)**H ... H(2)**H H(1)**H,
 * \]
 * where k = min(m,n). Each H(j) has the form
 * \[
 *          H(j) = I - tauw * w * w**H
 * \]
 * where tauw is a scalar, and w is a vector with
 * \[
 *          w[0] = w[1] = ... = w[j-1] = 0; w[j] = 1,
 * \]
 * where w[j+1]**H through w[n]**H are stored on exit in the jth row of A.
 *
 * @return  0 if success
 *
 * @param[in,out] A m-by-n matrix.
 *      On exit, the elements on and below the diagonal of the array
 *      contain the m by min(m,n) lower trapezoidal matrix L (L is
 *      lower triangular if m <= n); the elements above the diagonal,
 *      with the array tauw, represent the unitary matrix Q as a
 *      product of elementary reflectors.
 *
 * @param[out] tau min(n,m) vector.
 *      The scalar factors of the elementary reflectors.
 *
 * @param[in] opts Options.
 *      - @c opts.work is used if whenever it has sufficient size.
 *        The sufficient size can be obtained through a workspace query.
 *
 * @ingroup computational
 */
template <typename A_t, typename tau_t, typename TT_t>
int geqrf(A_t& A,
          tau_t& tau,
          const geqrf_opts_t<TT_t, size_type<A_t>>& opts = {})
{
    Create<A_t> new_matrix;

    using idx_t = size_type<A_t>;
    using range = std::pair<idx_t, idx_t>;

    // constants
    const idx_t m = nrows(A);
    const idx_t n = ncols(A);
    const idx_t k = min(m, n);
    const idx_t nb = opts.nb;

    // check arguments
    tlapack_check((idx_t)size(tau) >= k);

    // Allocate or get workspace
    vectorOfBytes localworkdata;
    Workspace work = [&]() {
        workinfo_t workinfo;
        geqrf_worksize(A, tau, workinfo, opts);
        return alloc_workspace(localworkdata, workinfo, opts.work);
    }();

    if (opts.TT == NULL) {
        Workspace sparework;
        auto TT = new_matrix(work, nb, nb, sparework);

        // Options to forward
        auto&& geqr2Opts = workspace_opts_t<>{sparework};
        auto&& larfbOpts = workspace_opts_t<void>{sparework};

        // Main computational loop
        for (idx_t j = 0; j < k; j += nb) {
            idx_t ib = std::min<idx_t>(nb, k - j);

            // Compute the QR factorization of the current block A(j:m,j:j+ib)
            auto A11 = slice(A, range(j, m), range(j, j + ib));
            auto tauw1 = slice(tau, range(j, j + ib));

            geqr2(A11, tauw1, geqr2Opts);

            if (j + ib < n) {
                // Form the triangular factor of the block reflector H = H(j)
                // H(j+1) . . . H(j+ib-1)
                auto TT1 = slice(TT, range(0, ib), range(0, ib));
                larft(Direction::Forward, StoreV::Columnwise, A11, tauw1, TT1);

                // Apply H to A(j:m,j+ib:n) from the left
                auto A12 = slice(A, range(j, m), range(j + ib, n));
                larfb(Side::Left, Op::ConjTrans, Direction::Forward,
                      StoreV::Columnwise, A11, TT1, A12, larfbOpts);
            }
        }
    }
    else {
        auto TT = *opts.TT;

        // Options to forward
        auto&& geqr2Opts = workspace_opts_t<>{work};
        auto&& larfbOpts = workspace_opts_t<void>{work};

        // Main computational loop
        for (idx_t j = 0; j < k; j += nb) {
            idx_t ib = std::min<idx_t>(nb, k - j);

            // Compute the QR factorization of the current block A(j:m,j:j+ib)
            auto A11 = slice(A, range(j, m), range(j, j + ib));
            auto tauw1 = slice(tau, range(j, j + ib));

            geqr2(A11, tauw1, geqr2Opts);

            // Form the triangular factor of the block reflector H = H(j)
            // H(j+1) . . . H(j+ib-1)
            auto TT1 = slice(TT, range(j, j + ib), range(0, ib));
            larft(Direction::Forward, StoreV::Columnwise, A11, tauw1, TT1);

            if (j + ib < n) {
                // Apply H to A(j:m,j+ib:n) from the left
                auto A12 = slice(A, range(j, m), range(j + ib, n));
                larfb(Side::Left, Op::ConjTrans, Direction::Forward,
                      StoreV::Columnwise, A11, TT1, A12, larfbOpts);
            }
        }
    }

    return 0;
}
}  // namespace tlapack
#endif  // TLAPACK_GEQRF_HH
