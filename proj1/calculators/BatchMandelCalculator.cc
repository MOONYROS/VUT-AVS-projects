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
	realArr = (float *)(aligned_alloc(64, width * sizeof(float)));
	imagArr = (float *)(aligned_alloc(64, (height / 2) * sizeof(float)));

	// predpocitani realnych a imaginarnich hodnot v konstruktoru -> nemusi se pocitat v kazde iteraci
    for (int j = 0; j < width; j++) {
        realArr[j] = x_start + j * dx;
    }

    for (int i = 0; i < height / 2; i++) {
        imagArr[i] = y_start + i * dy;
    }
}

BatchMandelCalculator::~BatchMandelCalculator() {
	if (data != NULL) {
		free(data);
		data = NULL;
	}
	if (realArr != NULL) {
		free(realArr);
		realArr = NULL;
	}
	if (imagArr != NULL) {
		free(imagArr);
		imagArr = NULL;
	}
}


int * BatchMandelCalculator::calculateMandelbrot () {
	// pro -s 4096 vychazela tile size 512 nejlepe
	const int TILE_SIZE = 512;
	const int HALF_HEIGHT = height / 2;

	// budeme postupovat po kusech matice
	#pragma omp parallel for
	for (int tileIndex = 0; tileIndex < (HALF_HEIGHT * width); tileIndex += TILE_SIZE) {

		// vypocet aktualni pozice v matici
		int tileRowStart = (tileIndex / width) * TILE_SIZE;
		int tileColStart = (tileIndex % width) / TILE_SIZE * TILE_SIZE;

		// jedeme pres radky v ramci tilu nebo do konce matice
		for (int i = tileRowStart; i < std::min(tileRowStart + TILE_SIZE, HALF_HEIGHT); i++) {
			float y = imagArr[i];

			// budu pro radek pocitat uniky
			int escapeCounter = 0;

			// zkusil jsem simdlen nastavit na 16 - po ruznych zkouskach mi to vychazelo jako nejlepsi pro -s 4096
			#pragma omp simd aligned(data: 512) simdlen(16) reduction(+: escapeCounter)
			for (int j = tileColStart; j < std::min(tileColStart + TILE_SIZE, width); j++) {
				float x = realArr[j];

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
				data[(height - i - 1) * width + j] = value;
			}

			// pokud vsechny body v radku unikly, nema smysl dal pocitat
			if (escapeCounter >= width) {
				break;
			}
		}
	}

	return data;
}
