/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  ONDREJ LUKASEK <xlukas15@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    DATE
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{

}

// TODO paralelizace
unsigned TreeMeshBuilder::processNode(const Vec3_t<float> &minCorner, 
                                      float edgelength,
                                      const ParametricScalarField &field)
{
    // zjistim si, kde lezi stred aktualniho bloku
    Vec3_t<float> center = {
        minCorner.x + edgelength / 2,
        minCorner.y + edgelength / 2,
        minCorner.z + edgelength / 2
    };

    // vypocitam si podminku prazdnosti bloku
    float centerValue = evaluateFieldAt(center, field);
    float maxDistance = mIsoLevel + sqrt(3.0f) / 2 * edgelength;

    if (centerValue > maxDistance) {
        // blok je prazdny, neobsahuje povrch => vracim 0
        return 0;
    }

    if (edgelength <= mGridResolution) {
        // blok je dostatecne maly => volam buildcube
        return buildCube(minCorner, field);
    }

    // ani jedna z podminek neplatila => rozdeluji na podbloky (2^3 = 8)
    // a pro kazdy z nich zavolam rekurzivne processNode
    unsigned totalTriangles = 0;
    float childEdgeLength = edgelength / 2;

    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            for (int z = 0; z < 2; ++z) {
                // vypocitam znovu minCorner pro dany podblok 
                Vec3_t<float> childMinCorner = {
                    minCorner.x + x * childEdgeLength,
                    minCorner.y + y * childEdgeLength,
                    minCorner.z + z * childEdgeLength,
                };

                // rekurzivne zpracuji kazdy podblok
                totalTriangles += processNode(childMinCorner, childEdgeLength, field);
            }
        }
    }

    return totalTriangles;
}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    // Suggested approach to tackle this problem is to add new method to
    // this class. This method will call itself to process the children.
    // It is also strongly suggested to first implement Octree as sequential
    // code and only when that works add OpenMP tasks to achieve parallelism.

    // zase mam na startu 0 trojuhelniku
    unsigned totalTriangles = 0;

    // zacneme na zacatku celeho gridu
    Vec3_t<float> rootStartCorner = {0.0f,
                                     0.0f,
                                     0.0f};
    
    // velikost hrany je cele hrany gridu
    float rootEdgeLength = mGridSize;

    // spustim rekurzivni funkci, co jsem si vytvoril drive
    totalTriangles = processNode(rootStartCorner, rootEdgeLength, field);
    
    // vracim ten rekurzivne spocitany pocet trojuhelniku
    return totalTriangles;
}

float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    return 0.0f;
}

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    
}
