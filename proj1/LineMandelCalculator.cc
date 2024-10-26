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
	data = (int *)(_mm_malloc(height * width * sizeof(int), 64));
	xVals = (float *)(_mm_malloc(height * width * sizeof(float), 64));
	yVals = (float *)(_mm_malloc(height * width * sizeof(float), 64));

	// predpocitam si realne casti
	for (int j = 0; j < wjdth; j++) {
		xVals[j] = x_start + j * dx;
	}

	// a taky imaginarni casti
	for (int i = 0; i < height; i++) {
		yVals[i] = y_start + i * dy;
	}
}

LineMandelCalculator::~LineMandelCalculator() {
	if (data != NULL) {
		_mm_free(data);
		data = NULL;
	}
	if (xVals != NULL) {
		_mm_free(xVals);
		xVals = NULL;
	}
	if (yVals != NULL) {
		_mm_free(yVals);
		yVals = NULL;
	}
}


int * LineMandelCalculator::calculateMandelbrot () {
	#pragma omp parallel for
	for (int i = 0; i < height; i++) {
		float y = yVals[i];

		// zkusil jsem simdlen nastavit na 8 - po ruznych zkouskach mi to vychazelo jako nejlepsi
		#pragma omp simd aligned(data: 64) simdlen(8)
		for (int j = 0; j < width / 2; j++) {
			float x = xVals[j];

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
			data[i * width + (width - j - 1)] = value;
		}
	}

	return data;
}
