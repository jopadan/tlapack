/// @file lantr.hpp
/// @author Weslley S Pereira, University of Colorado Denver, USA
/// Adapted from @see https://github.com/langou/latl/blob/master/include/lantr.h
//
// Copyright (c) 2012-2022, University of Colorado Denver. All rights reserved.
//
// This file is part of <T>LAPACK.
// <T>LAPACK is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#ifndef __TLAPACK_LANTR_HH__
#define __TLAPACK_LANTR_HH__

#include "base/types.hpp"
#include "lapack/lassq.hpp"

namespace tlapack {

/** Calculates the norm of a symmetric matrix.
 * 
 * @tparam norm_t Either Norm or any class that implements `operator Norm()`.
 * @tparam uplo_t Either Uplo or any class that implements `operator Uplo()`.
 * 
 * @param[in] normType
 *      - Norm::Max: Maximum absolute value over all elements of the matrix.
 *          Note: this is not a consistent matrix norm.
 *      - Norm::One: 1-norm, the maximum value of the absolute sum of each column.
 *      - Norm::Inf: Inf-norm, the maximum value of the absolute sum of each row.
 *      - Norm::Fro: Frobenius norm of the matrix.
 *          Square root of the sum of the square of each entry in the matrix.
 * 
 * @param[in] uplo
 *      - Uplo::Upper: A is a upper triangle matrix;
 *      - Uplo::Lower: A is a lower triangle matrix.
 *
 * @param[in] diag
 *     Whether A has a unit or non-unit diagonal:
 *     - Diag::Unit:    A is assumed to be unit triangular.
 *     - Diag::NonUnit: A is not assumed to be unit triangular.
 * 
 * @param[in] A n-by-n triangular matrix.
 * 
 * @ingroup auxiliary
 */
template< class norm_t, class uplo_t, class diag_t, class matrix_t >
auto
lantr( norm_t normType, uplo_t uplo, diag_t diag, const matrix_t& A )
{
    using real_t = real_type< type_t<matrix_t> >;
    using idx_t  = size_type< matrix_t >;
    using pair   = pair<idx_t,idx_t>;

    // constants
    const idx_t m = nrows(A);
    const idx_t n = ncols(A);

    // check arguments
    tlapack_check_false(  normType != Norm::Fro &&
                    normType != Norm::Inf &&
                    normType != Norm::Max &&
                    normType != Norm::One );
    tlapack_check_false(  uplo != Uplo::Lower &&
                    uplo != Uplo::Upper );
    tlapack_check_false( diag != Diag::NonUnit &&
                         diag != Diag::Unit );
    tlapack_check_false(  access_denied( uplo, read_policy(A) ) );

    // quick return
    if (m == 0 || n == 0)
        return real_t( 0 );

    // Norm value
    real_t norm( 0 );

    if( normType == Norm::Max )
    {
        if( uplo == Uplo::Upper ) {
            for (idx_t j = 0; j < n; ++j) {
                if( diag == Diag::NonUnit )
                    for (idx_t i = 0; i <= j; ++i)
                    {
                        real_t temp = tlapack::abs( A(i,j) );

                        if (temp > norm)
                            norm = temp;
                        else {
                            if ( isnan(temp) ) 
                                return temp;
                        }
                    }
                else
                    for (idx_t i = 0; i < j; ++i)
                    {
                        real_t temp = tlapack::abs( A(i,j) );

                        if (temp > norm)
                            norm = temp;
                        else {
                            if ( isnan(temp) ) 
                                return temp;
                        }
                    }
            }
        }
        else {
            for (idx_t j = 0; j < n; ++j) {
                if( diag == Diag::NonUnit )
                    for (idx_t i = j; i < m; ++i)
                    {
                        real_t temp = tlapack::abs( A(i,j) );

                        if (temp > norm)
                            norm = temp;
                        else {
                            if ( isnan(temp) ) 
                                return temp;
                        }
                    }
                else
                    for (idx_t i = j+1; i < m; ++i)
                    {
                        real_t temp = tlapack::abs( A(i,j) );

                        if (temp > norm)
                            norm = temp;
                        else {
                            if ( isnan(temp) ) 
                                return temp;
                        }
                    }
            }
        }
        if (norm < 1)
            norm = 1;
    }
    else if( normType == Norm::Inf )
    {
        if( uplo == Uplo::Upper ) {
            for (idx_t i = 0; i < m; ++i)
            {
                real_t sum( 0 );
                if( diag == Diag::NonUnit )
                    for (idx_t j = i; j < n; ++j)
                        sum += tlapack::abs( A(i,j) );
                else {
                    for (idx_t j = i+1; j < n; ++j)
                        sum += tlapack::abs( A(i,j) );
                    sum += 1;
                }

                if (sum > norm)
                    norm = sum;
                else {
                    if ( isnan(sum) ) 
                        return sum;
                }
            }
        }
        else {
            for (idx_t i = 0; i < m; ++i)
            {
                real_t sum( 0 );
                if( diag == Diag::NonUnit )
                    for (idx_t j = 0; j <= i; ++j)
                        sum += tlapack::abs( A(i,j) );
                else {
                    for (idx_t j = 0; j < i; ++j)
                        sum += tlapack::abs( A(i,j) );
                    sum += 1;
                }

                if (sum > norm)
                    norm = sum;
                else {
                    if ( isnan(sum) ) 
                        return sum;
                }
            }
        }
    }
    else if( normType == Norm::One )
    {
        if( uplo == Uplo::Upper ) {
            for (idx_t j = 0; j < n; ++j)
            {
                real_t sum( 0 );
                if( diag == Diag::NonUnit )
                    for (idx_t i = 0; i <= j; ++i)
                        sum += tlapack::abs( A(i,j) );
                else {
                    for (idx_t i = 0; i < j; ++i)
                        sum += tlapack::abs( A(i,j) );
                    sum += 1;
                }

                if (sum > norm)
                    norm = sum;
                else {
                    if ( isnan(sum) ) 
                        return sum;
                }
            }
        }
        else {
            for (idx_t j = 0; j < n; ++j)
            {
                real_t sum( 0 );
                if( diag == Diag::NonUnit )
                    for (idx_t i = j; i < m; ++i)
                        sum += tlapack::abs( A(i,j) );
                else {
                    for (idx_t i = j+1; i < m; ++i)
                        sum += tlapack::abs( A(i,j) );
                    sum += 1;
                }

                if (sum > norm)
                    norm = sum;
                else {
                    if ( isnan(sum) ) 
                        return sum;
                }
            }
        }
    }
    else
    {
        real_t scale(1), sum;

        if( uplo == Uplo::Upper ) {
            if( diag == Diag::NonUnit ) {
                sum = real_t( 0 );
                for (idx_t j = 0; j < n; ++j)
                    lassq( slice(A,range<idx_t>(0,j+1),j), scale, sum );
            }
            else {
                sum = real_t( std::min(m,n) );
                for (idx_t j = 0; j < n; ++j)
                    lassq( slice(A,range<idx_t>(0,j),j), scale, sum );
            }
        }
        else {
            if( diag == Diag::NonUnit ) {
                sum = real_t( 0 );
                for (idx_t j = 0; j < n; ++j)
                    lassq( slice(A,range<idx_t>(j,m),j), scale, sum );
            }
            else {
                sum = real_t( std::min(m,n) );
                for (idx_t j = 0; j < n; ++j)
                    lassq( slice(A,range<idx_t>(j+1,m),j), scale, sum );
            }
        }
        norm = scale * sqrt(sum);
    }

    return norm;
}

/** Calculates the norm of a triangular matrix.
 * 
 * Code optimized for the infinity norm on column-major layouts using a workspace
 * of size at least m, where m is the number of rows of A.
 * @see lantr( norm_t normType, uplo_t uplo, diag_t diag, const matrix_t& A ).
 * 
 * @param work Vector of size at least m.
 * 
 * @ingroup auxiliary
 */
template< class norm_t, class uplo_t, class diag_t, class matrix_t, class work_t >
auto
lantr( norm_t normType, uplo_t uplo, diag_t diag, const matrix_t& A, work_t& work )
{
    using T      = type_t< matrix_t >;
    using real_t = real_type< T >;
    using idx_t  = size_type< matrix_t >;
        
    // constants
    const idx_t m = nrows(A);
    const idx_t n = ncols(A);

    // check arguments
    tlapack_check_false(  normType != Norm::Fro &&
                    normType != Norm::Inf &&
                    normType != Norm::Max &&
                    normType != Norm::One );
    tlapack_check_false(  uplo != Uplo::Lower &&
                    uplo != Uplo::Upper );
    tlapack_check_false( diag != Diag::NonUnit &&
                         diag != Diag::Unit );
    tlapack_check_false(  access_denied( uplo, read_policy(A) ) );

    // quick return
    if (m == 0 || n == 0)
        return real_t( 0 );

    // redirect for max-norm, one-norm and Frobenius norm
    if      ( normType == Norm::Max  ) return lantr( max_norm, uplo, diag, A );
    else if ( normType == Norm::One  ) return lantr( one_norm, uplo, diag,  A );
    else if ( normType == Norm::Fro  ) return lantr( frob_norm, uplo, diag, A );
    else if ( normType == Norm::Inf  ) {

        // the code below uses a workspace and is meant for column-major layout
        // so as to do one pass on the data in a contiguous way when computing
	    // the infinite norm.

        // Norm value
        real_t norm( 0 );

        if( uplo == Uplo::Upper ) {
            if( diag == Diag::NonUnit ) {
                work[0] = tlapack::abs( A(0,0) );
                for (idx_t i = 1; i < m; ++i)
                    work[i] = real_t(0);
    
                for (idx_t j = 1; j < n; ++j)
                    for (idx_t i = 0; i <= j; ++i)
                        work[i] += tlapack::abs( A(i,j) );
            }
            else {
                work[0] = real_t(1);
                for (idx_t i = 1; i < m; ++i)
                    work[i] = real_t(0);
    
                for (idx_t j = 1; j < n; ++j) {
                    for (idx_t i = 0; i < j; ++i)
                        work[i] += tlapack::abs( A(i,j) );
                    work[j] += real_t(1);
                }
            }
        }
        else {
            if( diag == Diag::NonUnit ) {
                for (idx_t i = 0; i < m; ++i)
                    work[i] = tlapack::abs( A(i,0) );
    
                for (idx_t j = 1; j < n; ++j)
                    for (idx_t i = j; i < m; ++i)
                        work[i] += tlapack::abs( A(i,j) );
            }
            else {
                work[0] = real_t(1);
                for (idx_t i = 1; i < m; ++i)
                    work[i] = tlapack::abs( A(i,0) );
    
                for (idx_t j = 1; j < n; ++j) {
                    for (idx_t i = j+1; i < m; ++i)
                        work[i] += tlapack::abs( A(i,j) );
                    work[j] += real_t(1);
                }
            }
        }

        for (idx_t i = 0; i < m; ++i)
        {
            real_t temp = work[i];

            if (temp > norm)
                norm = temp;
            else {
                if (isnan(temp))
                    return temp;
            }
        }

        return norm;
    }
}

} // lapack

#endif // __LANTR_HH__
