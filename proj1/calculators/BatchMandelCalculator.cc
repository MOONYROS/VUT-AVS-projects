/**
 * @file BatchMandelCalculator.cc
 * @author Ondrej Lukasek (xlukas15@stud.fit.vutbr.cz)
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date 2024-10-20
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <stdexcept>

#include "BatchMandelCalculator.h"

BatchMandelCalculator::BatchMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator")
{
	data = (int *)(aligned_alloc(64, height * width * sizeof(int)));
}

BatchMandelCalculator::~BatchMandelCalculator() {
	if (data != NULL) {
		free(data);
		data = NULL;
	}
}


int * BatchMandelCalculator::calculateMandelbrot () {
	const int TILE_SIZE = 8;

	// budeme postupovat po kusech matice
	#pragma omp simd parallel for
	for (int tileIndex = 0; tileIndex < (height * width); tileIndex += TILE_SIZE) {

		// vypocet aktualni pozice v matici
		int tileRowStart = (tileIndex / width) * TILE_SIZE;
		int tileColStart = (tileIndex % width) / TILE_SIZE * TILE_SIZE;

		// jedeme pres radky v ramci tilu nebo do konce matice
		for (int i = tileRowStart; i < std::min(tileRowStart + TILE_SIZE, height); i++) {
			float y = y_start + i * dy;

			// budu pro radek pocitat uniky
			int escapeCounter = 0;

			// tady valime pres sloupce aktualniho tilu nebo do konce matice
			#pragma omp simd aligned(data: 64) simdlen(8) reduction(+: escapeCounter)
			for (int j = tileColStart; j < std::min(tileColStart + TILE_SIZE, width); j++) {
				float x = x_start + j * dx;

				// zaciname pocitat mandelbrotovu mnozinu
				float zReal = x;
				float zImag = y;
				int value = limit;

				for (int k = 0; k < limit; k++) {
					float r2 = zReal * zReal;
					float i2 = zImag * zImag;

					if (r2 + i2 > 4.0f) {
						value = k;
						escapeCounter++; // mame unik -> inkrementujeme pocitadlo
						break;
					}

					zImag = 2.0f * zReal * zImag + y;
					zReal = r2 - i2 + x;
				}

				data[i * width + j] = value;
			}

			// pokud vsechny body v radku unikly, nema smysl dal pocitat
			if (escapeCounter >= width) {
				break;
			}
		}
	}

	return data;
}
