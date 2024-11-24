/**
 * @file    loop_mesh_builder.cpp
 *
 * @author  ONDREJ LUKASEK <xlukas15@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP loops
 *
 * @date    24.11.2024
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "loop_mesh_builder.h"

LoopMeshBuilder::LoopMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "OpenMP Loop")
{

}

unsigned LoopMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    size_t totalCubesCount = mGridSize*mGridSize*mGridSize;

    unsigned totalTriangles = 0;

    // rozdeleni foru na vice vlaken
    // redukce je kvuli pripocitavani k totalTriangles v kazde iteraci
    // predchazi se tak race conditions
    #pragma omp parallel for reduction(+: totalTriangles)
    for (size_t i = 0; i < totalCubesCount; ++i) {
        Vec3_t<float> cubeOffset( i % mGridSize,
                                 (i / mGridSize) % mGridSize,
                                  i / (mGridSize*mGridSize));

        totalTriangles += buildCube(cubeOffset, field);
    }

    return totalTriangles;
}

float LoopMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    // ulozim si do vlastniho pole ty bodiky
    const Vec3_t<float> *pPoints = field.getPoints().data();

    // spocitam si, kolik mam tech bodiku
    const unsigned count = unsigned(field.getPoints().size());

    // vezmu si maximalni vzdalenost pole, kterou budu postupne zmensovat
    float minDistanceSquared = std::numeric_limits<float>::max();

    #pragma omp parallel for reduction(min:minDistanceSquared)
    for (unsigned i = 0; i < count; ++i) {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        minDistanceSquared = std::min(minDistanceSquared, distanceSquared);
    }

    return sqrt(minDistanceSquared);
}

void LoopMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    // buffer pro trojuhelniky - pro kazde vlakno vlastni
    // pokud vlakno spousti metodu vicekrat, uklada si data do stejneho vektoru
    static thread_local std::vector<Triangle_t> localTriangles;

    // pridam trojuhelnik do bufferu
    localTriangles.push_back(triangle);

    #pragma omp critical
    {
        // pridava na konec mTriangles buffer localTriangles
        mTriangles.insert(mTriangles.end(), localTriangles.begin(), localTriangles.end());
    }

    localTriangles.clear(); // vycisteni bufferu
}
