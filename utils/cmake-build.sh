#!/bin/bash

# Setting exit on error
set -e

cmake -B build -S .

cmake --build build -j$(nproc)
