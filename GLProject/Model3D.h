#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <iostream>
#include "Shader.h"
#include "TextureLoader.h"

struct MeshVertex {
    glm::vec3 Position;
    glm::vec2 TexCoords;
};

class SimpleMesh {
public:
    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int textureID;

    SimpleMesh(const std::vector<MeshVertex>& verts,
               const std::vector<unsigned int>& inds,
               unsigned int texID)
        : vertices(verts), indices(inds), textureID(texID) {
        setupMesh();
    }

    void draw(const Shader& shader) const {
        shader.use();
        if (textureID != 0) {
            shader.setBool("uUseSolidColor", false);
            shader.setVec3("uTintColor", glm::vec3(1.0f));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            shader.setInt("texture1", 0);
        } else {
            shader.setBool("uUseSolidColor", true);
            shader.setVec3("uSolidColor", glm::vec3(0.76f, 0.58f, 0.28f)); // لون نحاسي/ذهبي دافئ كبديل آمن
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup() {
        if (VAO != 0) glDeleteVertexArrays(1, &VAO);
        if (VBO != 0) glDeleteBuffers(1, &VBO);
        if (EBO != 0) glDeleteBuffers(1, &EBO);
        VAO = VBO = EBO = 0;
    }

private:
    unsigned int VAO = 0, VBO = 0, EBO = 0;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, TexCoords));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};

class Model3D {
public:
    std::vector<SimpleMesh> meshes;
    std::string directory;
    bool loadedSuccessfully = false;

    Model3D(const std::string& path) {
        loadModel(path);
    }

    ~Model3D() {
        for (auto& m : meshes) {
            m.cleanup();
        }
    }

    // رسم الموديل بتحويلات موقع وحجم ودوران محددة دون تكرار بيانات الموديل
    void drawTransformed(const Shader& shader,
                         const glm::vec3& position,
                         const glm::vec3& scaleVec,
                         float rotationYDeg = 0.0f) const {
        if (!loadedSuccessfully) return;

        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, position);
        modelMat = glm::rotate(modelMat, glm::radians(rotationYDeg), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMat = glm::scale(modelMat, scaleVec);

        shader.use();
        shader.setMat4("model", modelMat);

        for (const auto& mesh : meshes) {
            mesh.draw(shader);
        }
    }

private:
    void loadModel(const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "[Model3D] Failed to load model: " << path
                      << " | Error: " << importer.GetErrorString() << "\n";
            loadedSuccessfully = false;
            return;
        }

        size_t slashPos = path.find_last_of("/\\");
        directory = (slashPos == std::string::npos) ? "" : path.substr(0, slashPos);

        processNode(scene->mRootNode, scene);
        loadedSuccessfully = true;
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

    SimpleMesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<MeshVertex> verts;
        std::vector<unsigned int> inds;

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            MeshVertex v;
            v.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            if (mesh->mTextureCoords[0]) {
                v.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            } else {
                v.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            verts.push_back(v);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                inds.push_back(face.mIndices[j]);
            }
        }

        unsigned int texID = 0;
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString str;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
                std::string texPath = directory.empty() ? std::string(str.C_Str()) : (directory + "/" + std::string(str.C_Str()));
                texID = TextureLoader::loadFromFile(texPath, glm::vec3(0.76f, 0.58f, 0.28f));
            }
        }

        return SimpleMesh(verts, inds, texID);
    }
};