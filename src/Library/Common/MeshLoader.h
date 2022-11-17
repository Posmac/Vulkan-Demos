#pragma once

#include <string>
#include <fstream>
#include <vector>

#include "Library/Core/Core.h"
#include "../../external/tiny_obj_loader.h"

namespace vk
{
    struct Mesh
    {
        std::vector<float> data;

        struct Part
        {
            uint32_t vertexOffset;
            uint32_t vertexCount;
        };

        std::vector<Part> meshes;
    };

    class MeshLoader
    {
    public:
        static Mesh LoadMesh(std::string& path, bool loadTexCoords, bool loadNormals)
        {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string error, warn;

            bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, path.c_str());
            if (!result)
            {
                ERROR_LOG("Model wasnt loaded", error, warn);
            }

            Mesh mesh;
            uint32_t totalOffset = 0;
            for (auto& shape : shapes)
            {
                uint32_t localOffset = totalOffset;

                for (auto& index : shape.mesh.indices)
                {
                    mesh.data.emplace_back(attrib.vertices[3 * index.vertex_index + 0]);
                    mesh.data.emplace_back(attrib.vertices[3 * index.vertex_index + 1]);
                    mesh.data.emplace_back(attrib.vertices[3 * index.vertex_index + 2]);
                    ++localOffset;

                    if (loadNormals && attrib.normals.size() > 0)
                    {
                        mesh.data.emplace_back(attrib.normals[3 * index.normal_index + 0]);
                        mesh.data.emplace_back(attrib.normals[3 * index.normal_index + 1]);
                        mesh.data.emplace_back(attrib.normals[3 * index.normal_index + 2]);
                    }

                    if (loadTexCoords && attrib.texcoords.size() > 0)
                    {
                        mesh.data.emplace_back(attrib.texcoords[2 * index.texcoord_index + 0]);
                        mesh.data.emplace_back(attrib.texcoords[2 * index.texcoord_index + 1]);
                    }

                    uint32_t localVertexCount = totalOffset - localOffset;
                    if (localVertexCount > 0)
                    {
                        mesh.meshes.push_back({ localOffset, localVertexCount });
                    }
                }
            }
            return mesh;
        }
    };
}