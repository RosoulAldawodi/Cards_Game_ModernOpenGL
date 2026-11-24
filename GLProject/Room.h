#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

class Room {
public:
    float width;
    float height;
    float depth;
    float floorY;

    unsigned int floorTex;
    unsigned int wallTex;
    unsigned int ceilingTex;

    Room(float w, float h, float d, float yFloor,
         unsigned int fTex, unsigned int wTex, unsigned int cTex)
        : width(w), height(h), depth(d), floorY(yFloor),
          floorTex(fTex), wallTex(wTex), ceilingTex(cTex) {
        setupMeshes();
    }

    ~Room() {
        if (quadVAO != 0) glDeleteVertexArrays(1, &quadVAO);
        if (quadVBO != 0) glDeleteBuffers(1, &quadVBO);
        if (cubeVAO != 0) glDeleteVertexArrays(1, &cubeVAO);
        if (cubeVBO != 0) glDeleteBuffers(1, &cubeVBO);
    }

    void draw(const Shader& shader) const {
        shader.use();

        float halfW = width * 0.5f;
        float halfD = depth * 0.5f;
        float ceilingY = floorY + height;

        // 1. الأرضية (تواجه للأعلى +Y)
        drawTexturedSurface(
            shader,
            floorTex,
            glm::vec3(0.0f, floorY, 0.0f),
            glm::vec3(-glm::half_pi<float>(), 0.0f, 0.0f),
            glm::vec2(width, depth)
        );

        // 2. السقف (يواجه للأسفل -Y)
        drawTexturedSurface(
            shader,
            ceilingTex,
            glm::vec3(0.0f, ceilingY, 0.0f),
            glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f),
            glm::vec2(width, depth)
        );

        // 3. الجدار الخلفي (يواجه نحو +Z)
        drawTexturedSurface(
            shader,
            wallTex,
            glm::vec3(0.0f, floorY + height * 0.5f, -halfD),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec2(width, height)
        );

        // 4. الجدار الأمامي (يواجه نحو -Z)
        drawTexturedSurface(
            shader,
            wallTex,
            glm::vec3(0.0f, floorY + height * 0.5f, halfD),
            glm::vec3(0.0f, glm::pi<float>(), 0.0f),
            glm::vec2(width, height)
        );

        // 5. الجدار الأيسر (يواجه نحو +X)
        drawTexturedSurface(
            shader,
            wallTex,
            glm::vec3(-halfW, floorY + height * 0.5f, 0.0f),
            glm::vec3(0.0f, glm::half_pi<float>(), 0.0f),
            glm::vec2(depth, height)
        );

        // 6. الجدار الأيمن (يواجه نحو -X)
        drawTexturedSurface(
            shader,
            wallTex,
            glm::vec3(halfW, floorY + height * 0.5f, 0.0f),
            glm::vec3(0.0f, -glm::half_pi<float>(), 0.0f),
            glm::vec2(depth, height)
        );

        // 7. فواصل الزوايا الرفيعة بلون بني هادئ بين الجدران
        drawCornerDividers(shader);

        // 8. الحدود العلوية الرفيعة بلون بني هادئ بين الجدران والسقف
        drawCeilingBorders(shader);
    }

private:
    unsigned int quadVAO = 0, quadVBO = 0;
    unsigned int cubeVAO = 0, cubeVBO = 0;

    glm::vec3 trimColor = glm::vec3(0.56f, 0.50f, 0.42f);

    void setupMeshes() {
        float quadVerts[] = {
            // positions          // texCoords
            -0.5f,  0.5f, 0.0f,    0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,    1.0f, 0.0f,

            -0.5f,  0.5f, 0.0f,    0.0f, 1.0f,
             0.5f, -0.5f, 0.0f,    1.0f, 0.0f,
             0.5f,  0.5f, 0.0f,    1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        float cubeVerts[] = {
            // back (-Z)
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            // front (+Z)
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            // left (-X)
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            // right (+X)
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            // bottom (-Y)
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            // top (+Y)
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void drawTexturedSurface(const Shader& shader,
                             unsigned int texID,
                             const glm::vec3& pos,
                             const glm::vec3& eulerRot,
                             const glm::vec2& size) const {
        shader.setBool("uUseSolidColor", false);
        shader.setVec3("uTintColor", glm::vec3(1.0f));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);
        shader.setInt("texture1", 0);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, eulerRot.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, eulerRot.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, eulerRot.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
        shader.setMat4("model", model);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void drawBoxes(const Shader& shader,
                   const glm::vec3* positions,
                   const glm::vec3* scales,
                   int count) const {
        shader.setBool("uUseSolidColor", true);
        shader.setVec3("uSolidColor", trimColor);

        glBindVertexArray(cubeVAO);
        for (int i = 0; i < count; ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, positions[i]);
            model = glm::scale(model, scales[i]);
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);
    }

    void drawCornerDividers(const Shader& shader) const {
        float halfW = width * 0.5f;
        float halfD = depth * 0.5f;
        float thickness = 0.018f;
        float centerY = floorY + height * 0.5f;

        glm::vec3 positions[4] = {
            glm::vec3(-halfW + thickness * 0.5f, centerY, -halfD + thickness * 0.5f),
            glm::vec3( halfW - thickness * 0.5f, centerY, -halfD + thickness * 0.5f),
            glm::vec3(-halfW + thickness * 0.5f, centerY,  halfD - thickness * 0.5f),
            glm::vec3( halfW - thickness * 0.5f, centerY,  halfD - thickness * 0.5f)
        };

        glm::vec3 scales[4] = {
            glm::vec3(thickness, height, thickness),
            glm::vec3(thickness, height, thickness),
            glm::vec3(thickness, height, thickness),
            glm::vec3(thickness, height, thickness)
        };

        drawBoxes(shader, positions, scales, 4);
    }

    void drawCeilingBorders(const Shader& shader) const {
        float halfW = width * 0.5f;
        float halfD = depth * 0.5f;
        float ceilingY = floorY + height;
        float thickness = 0.02f;
        float borderY = ceilingY - thickness * 0.5f;

        glm::vec3 positions[4] = {
            glm::vec3(0.0f, borderY, -halfD + thickness * 0.5f),
            glm::vec3(0.0f, borderY,  halfD - thickness * 0.5f),
            glm::vec3(-halfW + thickness * 0.5f, borderY, 0.0f),
            glm::vec3( halfW - thickness * 0.5f, borderY, 0.0f)
        };

        glm::vec3 scales[4] = {
            glm::vec3(width,     thickness, thickness),
            glm::vec3(width,     thickness, thickness),
            glm::vec3(thickness, thickness, depth),
            glm::vec3(thickness, thickness, depth)
        };

        drawBoxes(shader, positions, scales, 4);
    }
};