#!/bin/bash

ml intel

./build/PMC --builder tree --grid 64 data/bun_zipper_res3.pts test.obj

ml purge
