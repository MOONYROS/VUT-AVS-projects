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
}

LineMandelCalculator::~LineMandelCalculator() {
	if (data != NULL) {
		_mm_free(data);
		data = NULL;
	}
}


int * LineMandelCalculator::calculateMandelbrot () {
	#pragma omp parallel for
	for (int i = 0; i < height; i++) {
		float y = y_start + i * dy;

		// zkusil jsem simdlen nastavit na 8 - po ruznych zkouskach mi to vychazelo jako nejlepsi
		#pragma omp simd aligned(data: 64) simdlen(8)
		for (int j = 0; j < width / 2; j++) {
			float x = x_start + j * dx;

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
