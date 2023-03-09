#
# Copyright (c) 2022, The OpenThread Authors.
# All rights reserved.
# BDS-3
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(bbtc)

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/otPlatBle.c)
# NORDIC SDK APP END
