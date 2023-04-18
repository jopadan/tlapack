/// @file example_starpu.cpp
/// @author Weslley S Pereira, University of Colorado Denver, USA
//
// Copyright (c) 2021-2023, University of Colorado Denver. All rights reserved.
//
// This file is part of <T>LAPACK.
// <T>LAPACK is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

// Plugins for <T>LAPACK (must come before <T>LAPACK headers)
#include <tlapack/plugins/starpu.hpp>

// <T>LAPACK headers
#include <tlapack/blas/gemm.hpp>
#include <tlapack/lapack/lange.hpp>
#include <tlapack/starpu/gemm.hpp>

// C++ headers
#include <iostream>

int main(int argc, char** argv)
{
    using namespace tlapack;
    using starpu::Matrix;
    using T = float;

    srand(3);

    size_t m = 100;
    size_t n = 50;
    size_t k = 10;
    size_t r = 20;
    size_t s = 10;

    if (argc > 1) m = atoi(argv[1]);
    if (argc > 2) n = atoi(argv[2]);
    if (argc > 3) k = atoi(argv[3]);
    if (argc > 4) r = atoi(argv[4]);
    if (argc > 5) s = atoi(argv[5]);
    if (argc > 6 || (m % r != 0) || (n % s != 0) || (r > m) || (s > n) ||
        (r == 0) || (s == 0)) {
        std::cout << "Usage: " << argv[0] << " [m] [n] [k] [r] [s]"
                  << std::endl;
        std::cout << "  m:      number of rows of C (default: 14)" << std::endl;
        std::cout << "  n:      number of columns of C (default: 7)"
                  << std::endl;
        std::cout << "  k:      number of columns of A and rows of B "
                     "(default: 6)"
                  << std::endl;
        std::cout << "  r:      number of tiles in x (rows) direction of C "
                     "(default: 14)."
                  << std::endl;
        std::cout << "  s:      number of tiles in y (columns) direction of C "
                     "(default: 7)."
                  << std::endl;
        return 1;
    }

    // Print input parameters
    std::cout << "m = " << m << std::endl;
    std::cout << "n = " << n << std::endl;
    std::cout << "k = " << k << std::endl;
    std::cout << "r = " << r << std::endl;
    std::cout << "s = " << s << std::endl << std::endl;

    /* initialize StarPU */
    const int ret = starpu_init(NULL);
    if (ret == -ENODEV) return 77;
    STARPU_CHECK_RETURN_VALUE(ret, "starpu_init");

    {
        /* create matrix A */
        T* A_;
        starpu_malloc((void**)&A_, m * k * sizeof(T));
        for (size_t i = 0; i < m * k; i++) {
            A_[i] = T((float)rand() / RAND_MAX);
        }
        Matrix<T> A(A_, m, k);

        /* create matrix B */
        T* B_;
        starpu_malloc((void**)&B_, k * n * sizeof(T));
        for (size_t i = 0; i < k * n; i++) {
            B_[i] = T((float)rand() / RAND_MAX);
        }
        Matrix<T> B(B_, k, n);

        /* create matrix C */
        T* C_;
        starpu_malloc((void**)&C_, m * n * sizeof(T));
        for (size_t i = 0; i < m * n; i++) {
            C_[i] = T(0xdeadbeef);
        }
        Matrix<T> C(C_, m, n);

        /* GEMM */
        A.create_grid(r, 1);
        B.create_grid(1, s);
        C.create_grid(r, s);
        gemm(noTranspose, noTranspose, T(1), A, B, T(0), C);

        /* check the result */
        for (idx_t i = 0; i < m; ++i)
            for (idx_t j = 0; j < n; ++j)
                for (idx_t l = 0; l < k; ++l)
                    C(i, j) -= A(i, l) * B(l, j);

        C.destroy_grid();
        C.create_grid(m, n);
        std::cout << lange(frob_norm, C) << std::endl;

        starpu_free_noflag(A_, m * k * sizeof(T));
        starpu_free_noflag(B_, k * n * sizeof(T));
        starpu_free_noflag(C_, m * n * sizeof(T));
    }

    /* terminate StarPU */
    starpu_shutdown();

    return 0;
}
