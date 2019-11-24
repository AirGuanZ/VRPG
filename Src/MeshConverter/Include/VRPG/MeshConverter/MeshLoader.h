#pragma once

#include <assimp/scene.h>

#include <VRPG/MeshConverter/Common.h>

VRPG_MESH_CONVERTER_BEGIN

/**
 * @brief 从aiScene中提取静态骨架结构
 */
Mesh::StaticSkeleton LoadStaticSkeleton(const aiScene *scene);

/**
 * @brief 从aiScene中提取指定骨架上的所有骨骼动画
 */
std::vector<std::pair<std::string, Mesh::SkeletonAnimation>> LoadSkeletonAnimation(
    const aiScene *scene, const Mesh::StaticSkeleton &skeleton);

/**
 * @brief 从aiScene中加载带骨骼和动画的完整物体
 */
Mesh::Mesh LoadMesh(const aiScene *scene);

VRPG_MESH_CONVERTER_END
