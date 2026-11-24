#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

class Table {
public:
    float topY;
    float thickness;
    float width;
    float depth;
    float floorY;
    unsigned int tableTex;

    glm::vec3 deckZoneCenter;
    glm::vec3 playedZoneCenter;
    glm::vec3 opponentZoneCenter;

    Table(float w, float d, float thick, float surfaceY, float roomFloorY, unsigned int woodTexture)
        : width(w), depth(d), thickness(thick), topY(surfaceY), floorY(roomFloorY), tableTex(woodTexture) {

        // توزيع الأقسام الثلاثة لتشمل معظم سطح الطاولة مع فواصل رفيعة جداً بينها
        opponentZoneCenter = glm::vec3(0.0f, topY, -depth * 0.285f);
        playedZoneCenter   = glm::vec3(0.0f, topY,  0.000f);
        deckZoneCenter     = glm::vec3(0.0f, topY,  depth * 0.285f);

        setupUnitCube();
        setupSoftBorder();
    }

    ~Table() {
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteVertexArrays(1, &borderVAO);
        glDeleteBuffers(1, &borderVBO);
    }

    void draw(const Shader& shader) const {
        shader.use();
        glBindVertexArray(cubeVAO);

        // 1. سطح الطاولة
        shader.setBool("uUseSolidColor", false);
        shader.setVec3("uTintColor", glm::vec3(1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tableTex);
        shader.setInt("texture1", 0);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, topY - thickness * 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(width, thickness, depth));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2. أرجل الطاولة
        float legHeight = (topY - thickness) - floorY;
        float legCenterY = floorY + legHeight * 0.5f;
        float legSize = 0.20f;
        float offsetX = width * 0.43f;
        float offsetZ = depth * 0.42f;

        glm::vec3 legPositions[4] = {
            glm::vec3(-offsetX, legCenterY, -offsetZ),
            glm::vec3( offsetX, legCenterY, -offsetZ),
            glm::vec3(-offsetX, legCenterY,  offsetZ),
            glm::vec3( offsetX, legCenterY,  offsetZ)
        };

        for (int i = 0; i < 4; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, legPositions[i]);
            model = glm::scale(model, glm::vec3(legSize, legHeight, legSize));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 3. حواف بنية صغيرة وناعمة لأقسام شاملة لمعظم سطح الطاولة
        glm::vec3 brownBorderColor(0.42f, 0.25f, 0.14f);
        float zoneW = width * 0.90f;
        float zoneD = depth * 0.275f;

        drawSoftZoneFrame(shader, opponentZoneCenter, zoneW, zoneD, brownBorderColor);
        drawSoftZoneFrame(shader, playedZoneCenter,   zoneW, zoneD, brownBorderColor);
        drawSoftZoneFrame(shader, deckZoneCenter,     zoneW, zoneD, brownBorderColor);

        glBindVertexArray(0);
    }

private:
    unsigned int cubeVAO = 0, cubeVBO = 0;
    unsigned int borderVAO = 0, borderVBO = 0;

    void drawSoftZoneFrame(const Shader& shader, const glm::vec3& center, float zoneW, float zoneD, const glm::vec3& color) const {
        shader.setBool("uUseSolidColor", true);
        shader.setVec3("uSolidColor", color);

        glBindVertexArray(borderVAO);
        float lineThick = 0.0045f;
        float yPos = topY + 0.002f;

        glm::mat4 m(1.0f);
        // أعلى
        m = glm::translate(glm::mat4(1.0f), glm::vec3(center.x, yPos, center.z - zoneD * 0.5f));
        m = glm::scale(m, glm::vec3(zoneW, 1.0f, lineThick));
        shader.setMat4("model", m);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // أسفل
        m = glm::translate(glm::mat4(1.0f), glm::vec3(center.x, yPos, center.z + zoneD * 0.5f));
        m = glm::scale(m, glm::vec3(zoneW, 1.0f, lineThick));
        shader.setMat4("model", m);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // يسار
        m = glm::translate(glm::mat4(1.0f), glm::vec3(center.x - zoneW * 0.5f, yPos, center.z));
        m = glm::scale(m, glm::vec3(lineThick, 1.0f, zoneD));
        shader.setMat4("model", m);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // يمين
        m = glm::translate(glm::mat4(1.0f), glm::vec3(center.x + zoneW * 0.5f, yPos, center.z));
        m = glm::scale(m, glm::vec3(lineThick, 1.0f, zoneD));
        shader.setMat4("model", m);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(cubeVAO);
    }

    void setupUnitCube() {
        float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f, -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, -0.5f,  0.5f,  0.5f,  0.0f, 0.0f
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void setupSoftBorder() {
        float quad[] = {
            -0.5f, 0.0f, -0.5f,
            -0.5f, 0.0f,  0.5f,
             0.5f, 0.0f,  0.5f,
            -0.5f, 0.0f, -0.5f,
             0.5f, 0.0f,  0.5f,
             0.5f, 0.0f, -0.5f
        };

        glGenVertexArrays(1, &borderVAO);
        glGenBuffers(1, &borderVBO);
        glBindVertexArray(borderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, borderVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }
};