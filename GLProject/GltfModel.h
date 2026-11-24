#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

#include "Shader.h"
#include "TextureLoader.h"

class GltfModel {
public:
    struct Vertex {
        glm::vec3 Position;
        glm::vec2 TexCoords;
    };

    struct MeshPart {
        unsigned int VAO = 0;
        unsigned int VBO = 0;
        unsigned int EBO = 0;
        unsigned int indexCount = 0;
        unsigned int textureID = 0;
        bool ownsTexture = false;
        glm::vec3 fallbackColor = glm::vec3(0.65f, 0.48f, 0.22f);
    };

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale    = glm::vec3(1.0f);
    glm::vec3 rotation = glm::vec3(0.0f); // بالراديان (x, y, z)

    GltfModel() = default;

    explicit GltfModel(const std::string& path) {
        loadModel(path);
    }

    ~GltfModel() {
        cleanup();
    }

    GltfModel(const GltfModel&) = delete;
    GltfModel& operator=(const GltfModel&) = delete;

    bool loadModel(const std::string& path) {
        cleanup();

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_ImproveCacheLocality
        );

        if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
            std::cout << "[GltfModel] ASSIMP ERROR (" << path << "): " << importer.GetErrorString() << "\n";
            return false;
        }

        size_t slashPos = path.find_last_of("/\\");
        directory = (slashPos == std::string::npos) ? "" : path.substr(0, slashPos + 1);

        processNode(scene->mRootNode, scene);
        std::cout << "[GltfModel] Loaded '" << path << "' | meshes: " << meshes.size()
                  << " | embedded textures: " << scene->mNumTextures << "\n";
        return !meshes.empty();
    }

    void draw(const Shader& shader) const {
        drawCustom(shader, position, rotation, scale);
    }

    void drawCustom(const Shader& shader,
                    const glm::vec3& pos,
                    const glm::vec3& rot,
                    const glm::vec3& scl) const {
        if (meshes.empty()) return;

        shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scl);
        shader.setMat4("model", model);

        for (const auto& part : meshes) {
            if (part.textureID != 0) {
                shader.setBool("uUseSolidColor", false);
                shader.setVec3("uTintColor", glm::vec3(1.0f));
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, part.textureID);
                shader.setInt("texture1", 0);
            } else {
                shader.setBool("uUseSolidColor", true);
                shader.setVec3("uSolidColor", part.fallbackColor);
            }

            glBindVertexArray(part.VAO);
            glDrawElements(GL_TRIANGLES, part.indexCount, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    void cleanup() {
        for (auto& part : meshes) {
            if (part.ownsTexture && part.textureID != 0) {
                glDeleteTextures(1, &part.textureID);
            }
            if (part.EBO != 0) glDeleteBuffers(1, &part.EBO);
            if (part.VBO != 0) glDeleteBuffers(1, &part.VBO);
            if (part.VAO != 0) glDeleteVertexArrays(1, &part.VAO);
        }
        meshes.clear();
    }

private:
    std::vector<MeshPart> meshes;
    std::string directory;

    unsigned int loadEmbeddedTexture(const aiTexture* aiTex) const {
        if (!aiTex) return 0;

        int width = 0, height = 0, nrChannels = 0;
        unsigned char* data = nullptr;

        if (aiTex->mHeight == 0) {
            data = stbi_load_from_memory(
                reinterpret_cast<const unsigned char*>(aiTex->pcData),
                static_cast<int>(aiTex->mWidth),
                &width,
                &height,
                &nrChannels,
                0
            );
        } else {
            width = static_cast<int>(aiTex->mWidth);
            height = static_cast<int>(aiTex->mHeight);
            nrChannels = 4;
            size_t byteSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
            data = static_cast<unsigned char*>(stbi_load_from_memory(
                reinterpret_cast<const unsigned char*>(aiTex->pcData),
                static_cast<int>(byteSize),
                &width,
                &height,
                &nrChannels,
                4
            ));
        }

        if (!data) {
            std::cout << "[GltfModel] Failed to decode embedded GLB texture.\n";
            return 0;
        }

        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        unsigned int texID = 0;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        return texID;
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            processNode(node->mChildren[i], scene);
        }
    }

    MeshPart processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        vertices.reserve(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            Vertex v;
            v.Position = glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            );

            if (mesh->mTextureCoords[0]) {
                v.TexCoords = glm::vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                );
            } else {
                v.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(v);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        MeshPart part;
        part.indexCount = static_cast<unsigned int>(indices.size());

        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            aiColor4D baseCol4(0.65f, 0.48f, 0.22f, 1.0f);
            aiColor3D diffCol3(0.65f, 0.48f, 0.22f);
            if (material->Get(AI_MATKEY_BASE_COLOR, baseCol4) == AI_SUCCESS) {
                part.fallbackColor = glm::vec3(baseCol4.r, baseCol4.g, baseCol4.b);
            } else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffCol3) == AI_SUCCESS) {
                part.fallbackColor = glm::vec3(diffCol3.r, diffCol3.g, diffCol3.b);
            }

            aiString texPath;
            if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &texPath) == AI_SUCCESS ||
                material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
                std::string rel = texPath.C_Str();
                if (!rel.empty() && rel[0] == '*') {
                    int texIndex = std::atoi(rel.c_str() + 1);
                    if (texIndex >= 0 && static_cast<unsigned int>(texIndex) < scene->mNumTextures) {
                        part.textureID = loadEmbeddedTexture(scene->mTextures[texIndex]);
                        part.ownsTexture = (part.textureID != 0);
                    }
                } else if (!rel.empty()) {
                    std::string fullPath = directory + rel;
                    part.textureID = TextureLoader::loadFromFile(fullPath, part.fallbackColor);
                    part.ownsTexture = false;
                }
            }
        }

        glGenVertexArrays(1, &part.VAO);
        glGenBuffers(1, &part.VBO);
        glGenBuffers(1, &part.EBO);

        glBindVertexArray(part.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, part.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, part.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        return part;
    }
};