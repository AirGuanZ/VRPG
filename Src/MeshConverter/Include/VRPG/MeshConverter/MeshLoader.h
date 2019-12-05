#pragma once

#include <assimp/scene.h>

#include <VRPG/MeshConverter/Common.h>

VRPG_MESH_CONVERTER_BEGIN

/**
 * @brief 从aiScene中加载带骨骼和动画的完整物体
 */
Mesh::Mesh LoadMesh(const aiScene *scene);

VRPG_MESH_CONVERTER_END
