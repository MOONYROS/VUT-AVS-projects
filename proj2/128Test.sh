#!/bin/bash

ml intel

./build/PMC --builder tree --grid 128 data/bun_zipper_res3.pts test.obj

ml purge
