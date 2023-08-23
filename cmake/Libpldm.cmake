# ++
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.
#
#Module Name:
#
#	Libpldm.cmake
#
# Abstract:
#
#	CMake build script for libpldm.
#
# --

set(LIBPLDM_DIR ${CERBERUS_ROOT}/external/libpldm)
file(GLOB LIBPLDM_SOURCES ${LIBPLDM_DIR}/*.c)
set(LIBPLDM_INCLUDES ${LIBPLDM_DIR}/include)

