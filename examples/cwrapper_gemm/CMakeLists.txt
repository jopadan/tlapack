# Copyright (c) 2025, University of Colorado Denver. All rights reserved.
#
# This file is part of <T>LAPACK.
# <T>LAPACK is free software: you can redistribute it and/or modify it under
# the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

cmake_minimum_required(VERSION 3.5)

project( cwrapper_gemm C )

# Load <T>LAPACK
if( NOT TARGET tlapack_c )
  find_package( tlapack REQUIRED )
endif()

# add the example gemm
add_executable( example_cwrapper_gemm example_cwrapper_gemm.c )
target_link_libraries( example_cwrapper_gemm PRIVATE tlapack_c )
