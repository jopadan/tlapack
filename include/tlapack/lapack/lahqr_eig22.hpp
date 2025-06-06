/// @file lahqr_eig22.hpp
/// @author Thijs Steel, KU Leuven, Belgium
//
// Copyright (c) 2025, University of Colorado Denver. All rights reserved.
//
// This file is part of <T>LAPACK.
// <T>LAPACK is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#ifndef TLAPACK_LAHQR_EIG22_HH
#define TLAPACK_LAHQR_EIG22_HH

#include "tlapack/base/utils.hpp"

namespace tlapack {

/** Computes the eigenvalues of a 2x2 matrix A
 *
 * @param[in] a00
 *      Element (0,0) of A.
 * @param[in] a01
 *      Element (0,1) of A.
 * @param[in] a10
 *      Element (1,0) of A.
 * @param[in] a11
 *      Element (1,1) of A.
 * @param[out] s1
 * @param[out] s2
 *      s1 and s2 are the eigenvalues of A
 *
 * @ingroup auxiliary
 */
template <TLAPACK_SCALAR T>
void lahqr_eig22(
    T a00, T a01, T a10, T a11, complex_type<T>& s1, complex_type<T>& s2)
{
    // Using
    using real_t = real_type<T>;

    // Constants
    const real_t zero(0);
    const real_t two(2);

    const T s = abs1(a00) + abs1(a01) + abs1(a10) + abs1(a11);
    if (s == zero) {
        s1 = zero;
        s2 = zero;
        return;
    }

    a00 = a00 / s;
    a01 = a01 / s;
    a10 = a10 / s;
    a11 = a11 / s;
    const T tr = (a00 + a11) / two;
    const complex_type<T> det = (a00 - tr) * (a00 - tr) + a01 * a10;
    const complex_type<T> rtdisc = sqrt(det);

    s1 = s * (tr + rtdisc);
    s2 = s * (tr - rtdisc);
}

}  // namespace tlapack

#endif  // TLAPACK_LAHQR_EIG22_HH
