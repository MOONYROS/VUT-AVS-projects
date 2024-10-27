/**
 * @file LineMandelCalculator.cc
 * @author Ondrej Lukasek (xlukas15@stud.fit.vutbr.cz)
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date 2024-10-20
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>

#include "LineMandelCalculator.h"

LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
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

LineMandelCalculator::~LineMandelCalculator() {
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


int * LineMandelCalculator::calculateMandelbrot () {
	#pragma omp parallel for
	for (int i = 0; i < height / 2; i++) {
		float y = imagArr[i];

		// zkusil jsem simdlen nastavit na 8 - po ruznych zkouskach mi to vychazelo jako nejlepsi
		#pragma omp simd aligned(data: 64) simdlen(8)
		for (int j = 0; j < width; j++) {
			float x = realArr[j];

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
			data[(height - i - 1) * width + j] = value;
		}
	}

	
	return data;
}
