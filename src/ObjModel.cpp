#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include "ObjModel.hpp"
#include "Triangle.hpp"
#include "Texture.hpp"
#include "BVH.hpp"

void ObjModel::load() {
    using glm::vec2;
    using glm::vec3;
    namespace fs = std::filesystem;

    const fs::path objPath(filename);
    const fs::path objDir = objPath.parent_path();

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;
    reader_config.mtl_search_path = objDir.string();
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filename, reader_config))
        if (!reader.Error().empty())
            throw std::runtime_error("TinyObjReader: " + reader.Error());

    if (!reader.Warning().empty())
        std::cout << "TinyObjReader: " << reader.Warning();

    auto &attrib = reader.GetAttrib();
    auto &shapes = reader.GetShapes();
    triangles.clear();
    textures.clear();
    std::vector<bool> materialHasTexture;
    size_t total_faces = 0;
    for (const auto &shape : shapes) {
        total_faces += shape.mesh.num_face_vertices.size();
    }
    triangles.reserve(total_faces);

    materialHasTexture.reserve(reader.GetMaterials().size());
    textures.reserve(reader.GetMaterials().size());
    for (const auto &mat : reader.GetMaterials()) {
        const std::string &diffuseTexname = mat.diffuse_texname;
        if (diffuseTexname.empty()) {
            materialHasTexture.push_back(false);
            textures.emplace_back("");
            continue;
        }

        fs::path texturePath(diffuseTexname);
        if (texturePath.is_relative())
            texturePath = objDir / texturePath;

        materialHasTexture.push_back(true);
        textures.emplace_back(texturePath.lexically_normal().string());
    }

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;

        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset + 0];
            tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + 1];
            tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + 2];

            vec3 v0(attrib.vertices[3 * idx2.vertex_index + 0], attrib.vertices[3 * idx2.vertex_index + 1],
                    attrib.vertices[3 * idx2.vertex_index + 2]);
            vec3 v1(attrib.vertices[3 * idx1.vertex_index + 0], attrib.vertices[3 * idx1.vertex_index + 1],
                    attrib.vertices[3 * idx1.vertex_index + 2]);
            vec3 v2(attrib.vertices[3 * idx0.vertex_index + 0], attrib.vertices[3 * idx0.vertex_index + 1],
                    attrib.vertices[3 * idx0.vertex_index + 2]);

            vec2 uv0(0.0f);
            vec2 uv1(0.0f);
            vec2 uv2(0.0f);

            if (idx2.texcoord_index >= 0) {
                size_t texcoordOffset = static_cast<size_t>(2 * idx2.texcoord_index);
                if (texcoordOffset + 1 < attrib.texcoords.size())
                    uv0 = vec2(attrib.texcoords[texcoordOffset + 0], attrib.texcoords[texcoordOffset + 1]);
            }

            if (idx1.texcoord_index >= 0) {
                size_t texcoordOffset = static_cast<size_t>(2 * idx1.texcoord_index);
                if (texcoordOffset + 1 < attrib.texcoords.size())
                    uv1 = vec2(attrib.texcoords[texcoordOffset + 0], attrib.texcoords[texcoordOffset + 1]);
            }

            if (idx0.texcoord_index >= 0) {
                size_t texcoordOffset = static_cast<size_t>(2 * idx0.texcoord_index);
                if (texcoordOffset + 1 < attrib.texcoords.size())
                    uv2 = vec2(attrib.texcoords[texcoordOffset + 0], attrib.texcoords[texcoordOffset + 1]);
            }

            int materialId = shapes[s].mesh.material_ids[f];
            size_t textureIdx = static_cast<size_t>(-1);
            if (materialId >= 0) {
                size_t materialIdx = static_cast<size_t>(materialId);
                if (materialIdx < materialHasTexture.size() && materialHasTexture[materialIdx])
                    textureIdx = materialIdx;
            }

            triangles.push_back(Triangle(v0, v1, v2, uv0, uv1, uv2, textureIdx, vec3(1, 1, 1)));
            index_offset += fv;
        }
    }

    scaleToUnitCube();

    bvh = BVH(triangles);
}

void ObjModel::scaleToUnitCube() {
    if (triangles.empty())
        return;

    glm::vec3 minPos(1e9f);
    glm::vec3 maxPos(-1e9f);
    for (const Triangle &triangle : triangles) {
        minPos = glm::min(minPos, glm::min(triangle.v0, glm::min(triangle.v1, triangle.v2)));
        maxPos = glm::max(maxPos, glm::max(triangle.v0, glm::max(triangle.v1, triangle.v2)));
    }

    glm::vec3 center = (minPos + maxPos) * 0.5f;
    glm::vec3 size = maxPos - minPos;
    float minAxisLength = std::min({size.x, size.y, size.z});
    float scaleSize = 2.0f / minAxisLength;

    for (Triangle &triangle : triangles) {
        triangle.v0 = (triangle.v0 - center) * scaleSize;
        triangle.v1 = (triangle.v1 - center) * scaleSize;
        triangle.v2 = (triangle.v2 - center) * scaleSize;

        triangle.v0.x *= -1;
        triangle.v1.x *= -1;
        triangle.v2.x *= -1;

        triangle.v0.y *= -1;
        triangle.v1.y *= -1;
        triangle.v2.y *= -1;

        triangle.computeNormal();

        triangle.computeCentroid();
    }
}