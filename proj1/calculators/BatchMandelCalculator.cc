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
	data = (int *)(malloc(height * width * sizeof(int)));
}

BatchMandelCalculator::~BatchMandelCalculator() {
	if (data != NULL) {
		free(data);
		data = NULL;
	}
}


int * BatchMandelCalculator::calculateMandelbrot () {
	// pojedeme pres vsechny radky po batchich
	for (int batchStart = 0; batchStart < height; batchStart += batchSize) {
		// kontrola konce -> bud jedu po batchSize nebo po max vysku
		int batchEnd = std::min(batchStart + batchSize, height);

		// paralelizace -> kazdy radek batche je na jednom vlakne
		#pragma omp parallel for
		for (int i = batchStart; i < batchEnd; i++) {
			float y = y_start + i * dy;

			// vektorizace pro kazdy sloupec v radku v batchi
			#pragma omp simd
			for (int j = 0; j < width; j++) {
				float x = x_start + j * dx;

				// vypocet mandelbrotovy mnoziny
				float zReal = x;
				float zImag = y;
				int value = limit;

				for (int k = 0; k < limit; k++) {
					float r2 = zReal * zReal;
					float i2 = zImag * zImag;

					if (r2 + i2 > 4.0f) {
						value = k;
						break;
					}

					zImag = 2.0f * zReal * zImag + y;
					zReal = r2 - i2 + x;
				}

				data[i * width + j] = value;
			}
		}
	}

	return data;
}