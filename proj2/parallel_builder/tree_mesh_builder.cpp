/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  ONDREJ LUKASEK <xlukas15@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    24.11.2024
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{

}

// TODO OpenMP tasky
/**
 * @brief Recursively processes a node in scalar field and generates triangles.
 * 
 * @param pos Position of node's minimum corner.
 * @param currentGridSize Current node's size of the grid.
 * @param field Parametric field to be evaluated.
 * 
 * @return Total number of triangles generated within the initial node (and its child nodes).
 */
unsigned TreeMeshBuilder::processNode(const Vec3_t<float> &pos, float currentGridSize, const ParametricScalarField &field)
{
    // pokud je blok prazdny, vracime 0
    if (isNodeEmpty(pos, currentGridSize, field)) {
        return 0;
    }

    // pokud jsme dosahli cut-offu, tak koncime...
    if (currentGridSize <= CUT_OFF) {
        return buildCube(pos, field);
    }
    // ...jinak se rozdelime node na 8 dalsich potomku
    else {
        unsigned totalTriangles = 0;
        float childGridSize = currentGridSize / 2.0f;

        #pragma omp parallel
        {
            #pragma omp single nowait
            {
                for (int x = 0; x < 2; ++x)
                {
                    for (int y = 0; y < 2; ++y)
                    {
                        for (int z = 0; z < 2; ++z)
                        {
                            // vypocitame minimalni rozek pro kazdeho potomka...
                            Vec3_t<float> childPos = {
                                pos.x + x * childGridSize,
                                pos.y + y * childGridSize,
                                pos.z + z * childGridSize};

                            #pragma omp task shared(totalTriangles)
                            {
                                unsigned childTriangles = processNode(childPos, childGridSize, field);

                                // ...a pro kazdeho z nich metodu zavolame rekurzivne znovu
                                #pragma omp atomic update
                                totalTriangles += childTriangles;
                            }
                        }
                    }
                }
            }
        }

        // vracime konecny pocet vsech trojuhelniku
        return totalTriangles;
    }
}

// TODO OpenMP tasky
/**
 * @brief Determines if a node in the scalar field is empty.
 * 
 * @param pos Position of node's minimum corner.
 * @param currentGridSize Current node's size of the grid.
 * @param field Parametric field to be evaluated.
 * 
 * @return True if the node is empty, false if not.
 */
bool TreeMeshBuilder::isNodeEmpty(const Vec3_t<float> &pos, float currentGridSize, const ParametricScalarField &field)
{
    // pomocna promenna pro zapis podilu ve vzorci
    float fractionConst = sqrt(3.0f) / 2.0f;
    // vypocet delky hrany ("a" ve vzorci)
    float edgeLength = currentGridSize * mGridResolution;

    // vypocet souradnice stredu bloku
    const Vec3_t<float> blockCenter = {
        (pos.x + currentGridSize / 2.0f) * mGridResolution,
        (pos.y + currentGridSize / 2.0f) * mGridResolution,
        (pos.z + currentGridSize / 2.0f) * mGridResolution
    };

    // hodnota ve stredu bloku
    const float blockCenterValue = evaluateFieldAt(blockCenter, field);

    // finalni vypocet podminky
    return blockCenterValue > mIsoLevel + fractionConst * edgeLength;;
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
    // STEJNE JAKO LOOP
    const Vec3_t<float> *pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());

    float minDistanceSquared = std::numeric_limits<float>::max();

    for (unsigned i = 0; i < count; ++i)
    {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        minDistanceSquared = std::min(minDistanceSquared, distanceSquared);
    }
    

    return sqrt(minDistanceSquared);
}

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    // STEJNE JAKO LOOP
    static thread_local std::vector<Triangle_t> localTriangles;

    localTriangles.push_back(triangle);

    #pragma omp critical
    {
        mTriangles.insert(mTriangles.end(), localTriangles.begin(), localTriangles.end());
    }

    localTriangles.clear();
}
