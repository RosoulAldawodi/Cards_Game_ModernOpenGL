#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include "Shader.h"

class Card {
public:
    glm::vec3 deckPos;
    glm::vec3 playedPos;
    glm::vec3 currentPos;

    float width;
    float height;

    // -90 درجة = مسطحة أفقياً على الطاولة، 0 درجة = واقفة عمودياً نحو اللاعب
    float currentPitchDeg;
    float targetPitchDeg;

    bool isPlayed;
    bool isMovingToPlayed;
    float moveProgress; // من 0.0 إلى 1.0

    unsigned int textureID;
    glm::vec3 placeholderColor;

    Card(glm::vec3 startDeckPos, glm::vec3 targetPlayedPos, float w, float h, glm::vec3 tempColor, unsigned int texID = 0)
        : deckPos(startDeckPos),
          playedPos(targetPlayedPos),
          currentPos(startDeckPos),
          width(w),
          height(h),
          currentPitchDeg(-90.0f),
          targetPitchDeg(-90.0f),
          isPlayed(false),
          isMovingToPlayed(false),
          moveProgress(0.0f),
          textureID(texID),
          placeholderColor(tempColor) {
        initSharedQuad();
    }

    void setTexture(unsigned int texID) {
        textureID = texID;
    }

    void playCard() {
        if (!isPlayed) {
            isPlayed = true;
            isMovingToPlayed = true;
            moveProgress = 0.0f;
            targetPitchDeg = -90.0f;
        }
    }

    void update(float dt, int activeCamera) {
        // منع قفزات deltaTime الكبيرة إذا تغيرت نافذة البرنامج
        dt = glm::min(dt, 0.05f);

        targetPitchDeg = (!isPlayed && activeCamera == 2) ? 0.0f : -90.0f;

        // دمج أُسّي ناعم (Frame-rate independent smoothing) لوقوف وتسطيح البطاقات
        float rotSmoothing = 1.0f - std::exp(-9.0f * dt);
        currentPitchDeg += (targetPitchDeg - currentPitchDeg) * rotSmoothing;

        // حركة انتقال سلسة بقوس ارتفاع خفيف عند الضغط على G
        if (isMovingToPlayed) {
            moveProgress += dt * 1.65f;
            if (moveProgress >= 1.0f) {
                moveProgress = 1.0f;
                isMovingToPlayed = false;
            }

            // Smootherstep لبدء ونهاية حريرية
            float t = moveProgress;
            float smoothT = t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);

            currentPos = glm::mix(deckPos, playedPos, smoothT);
            float arcHeight = std::sin(smoothT * glm::pi<float>()) * 0.22f;
            currentPos.y += arcHeight;
        }
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
            shader.setVec3("uSolidColor", placeholderColor);
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, currentPos + glm::vec3(0.0f, 0.004f, 0.0f));
        model = glm::rotate(model, glm::radians(currentPitchDeg), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(width, height, 1.0f));

        shader.setMat4("model", model);

        glBindVertexArray(sharedVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    static void cleanupSharedMesh() {
        if (sharedVAO != 0) {
            glDeleteVertexArrays(1, &sharedVAO);
            glDeleteBuffers(1, &sharedVBO);
            sharedVAO = 0;
            sharedVBO = 0;
        }
    }

private:
    static inline unsigned int sharedVAO = 0;
    static inline unsigned int sharedVBO = 0;

    static void initSharedQuad() {
        if (sharedVAO != 0) return;

        float vertices[] = {
            -0.5f, 1.0f, 0.0f,    0.0f, 1.0f,
            -0.5f, 0.0f, 0.0f,    0.0f, 0.0f,
             0.5f, 0.0f, 0.0f,    1.0f, 0.0f,

            -0.5f, 1.0f, 0.0f,    0.0f, 1.0f,
             0.5f, 0.0f, 0.0f,    1.0f, 0.0f,
             0.5f, 1.0f, 0.0f,    1.0f, 1.0f
        };

        glGenVertexArrays(1, &sharedVAO);
        glGenBuffers(1, &sharedVBO);
        glBindVertexArray(sharedVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sharedVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }
};