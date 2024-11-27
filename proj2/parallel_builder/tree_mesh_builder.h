/**
 * @file    tree_mesh_builder.h
 *
 * @author  ONDREJ LUKASEK <xlukas15@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    24.11.2024
 **/

#ifndef TREE_MESH_BUILDER_H
#define TREE_MESH_BUILDER_H

#include "base_mesh_builder.h"

class TreeMeshBuilder : public BaseMeshBuilder
{
public:
    TreeMeshBuilder(unsigned gridEdgeSize);

protected:
    unsigned marchCubes(const ParametricScalarField &field);
    float evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field);
    void emitTriangle(const Triangle_t &triangle);
    const Triangle_t *getTrianglesArray() const { return mTriangles.data(); }
    
    // MOJE METODY
    unsigned processNode(const Vec3_t<float> &pos, float currentGridSize, const ParametricScalarField &field);
    bool isNodeEmpty(const Vec3_t<float> &pos, float currentGridSize, const ParametricScalarField &field);

    std::vector<Triangle_t> mTriangles; ///< Temporary array of triangles
    const unsigned CUT_OFF = 1;
};

#endif // TREE_MESH_BUILDER_H
