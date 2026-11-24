#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "UnitCube.h"

class Fireplace {
public:
    glm::vec3 position; // منتصف المدفأة أفقياً وعمقياً، و y يمثل floorY
    float width;
    float height;
    float depth;

    unsigned int bodyTex   = 0; // للجسم الأساسي (حجر/جدران المدفأة)
    unsigned int woodTex   = 0; // للرف العلوي
    unsigned int cavityTex = 0; // للقسم الداخلي من الشومينة
    unsigned int logTex    = 0; // للخشب المحترق
    unsigned int fireTex   = 0; // للنار

    glm::vec3 stoneColor   = glm::vec3(0.42f, 0.34f, 0.27f);
    glm::vec3 mantelColor  = glm::vec3(0.30f, 0.18f, 0.10f);
    glm::vec3 cavityColor  = glm::vec3(0.10f, 0.08f, 0.08f);
    glm::vec3 logColor     = glm::vec3(0.24f, 0.13f, 0.07f);
    glm::vec3 fireColor    = glm::vec3(0.90f, 0.42f, 0.10f);

    Fireplace(const glm::vec3& floorPos, float w, float h, float d)
        : position(floorPos), width(w), height(h), depth(d) {}

    void draw(const Shader& shader) const {
        shader.use();

        float baseY = position.y;
        float hearthH = height * 0.08f;
        float mantelH = height * 0.12f;
        float bodyH   = height - hearthH - mantelH;

        float pillarW  = width * 0.20f;
        float openingW = width - 2.0f * pillarW;
        float openingH = bodyH * 0.72f;

        // 1. القاعدة السفلية على الأرض تماماً
        drawBox(
            shader,
            glm::vec3(position.x, baseY + hearthH * 0.5f, position.z + depth * 0.06f),
            glm::vec3(width * 1.08f, hearthH, depth * 1.12f),
            glm::vec3(0.0f),
            stoneColor,
            bodyTex
        );

        // 2. العمود الأيسر
        drawBox(
            shader,
            glm::vec3(position.x - (openingW * 0.5f + pillarW * 0.5f), baseY + hearthH + bodyH * 0.5f, position.z),
            glm::vec3(pillarW, bodyH, depth * 0.85f),
            glm::vec3(0.0f),
            stoneColor,
            bodyTex
        );

        // 3. العمود الأيمن
        drawBox(
            shader,
            glm::vec3(position.x + (openingW * 0.5f + pillarW * 0.5f), baseY + hearthH + bodyH * 0.5f, position.z),
            glm::vec3(pillarW, bodyH, depth * 0.85f),
            glm::vec3(0.0f),
            stoneColor,
            bodyTex
        );

        // 4. الجزء العلوي فوق فتحة النار
        float topAboveOpeningH = bodyH - openingH;
        if (topAboveOpeningH > 0.001f) {
            drawBox(
                shader,
                glm::vec3(position.x, baseY + hearthH + openingH + topAboveOpeningH * 0.5f, position.z),
                glm::vec3(openingW, topAboveOpeningH, depth * 0.85f),
                glm::vec3(0.0f),
                stoneColor,
                bodyTex
            );
        }

        // 5. الرف العلوي للمدفأة
        drawBox(
            shader,
            glm::vec3(position.x, baseY + hearthH + bodyH + mantelH * 0.5f, position.z + depth * 0.04f),
            glm::vec3(width * 1.12f, mantelH, depth * 1.05f),
            glm::vec3(0.0f),
            mantelColor,
            woodTex
        );

        // 6. القسم الداخلي من الشومينة
        drawBox(
            shader,
            glm::vec3(position.x, baseY + hearthH + openingH * 0.5f, position.z - depth * 0.08f),
            glm::vec3(openingW, openingH, depth * 0.65f),
            glm::vec3(0.0f),
            cavityColor,
            cavityTex
        );

        // 7. كومة الخشب المحترق داخل الفتحة (4 قطع متقاطعة)
        float logY = baseY + hearthH + openingH * 0.15f;
        float logZ = position.z + depth * 0.06f;

        glm::vec3 logPositions[4] = {
            glm::vec3(position.x - openingW * 0.14f, logY,                    logZ),
            glm::vec3(position.x + openingW * 0.14f, logY + openingH * 0.03f, logZ + 0.04f),
            glm::vec3(position.x - openingW * 0.04f, logY + openingH * 0.07f, logZ + 0.02f),
            glm::vec3(position.x + openingW * 0.06f, logY + openingH * 0.10f, logZ - 0.02f)
        };

        glm::vec3 logScales[4] = {
            glm::vec3(openingW * 0.56f, openingH * 0.13f, openingH * 0.13f),
            glm::vec3(openingW * 0.54f, openingH * 0.12f, openingH * 0.12f),
            glm::vec3(openingW * 0.48f, openingH * 0.11f, openingH * 0.11f),
            glm::vec3(openingW * 0.45f, openingH * 0.10f, openingH * 0.10f)
        };

        glm::vec3 logRots[4] = {
            glm::vec3(0.0f,  0.22f,  0.10f),
            glm::vec3(0.0f, -0.26f, -0.09f),
            glm::vec3(0.0f,  0.12f, -0.06f),
            glm::vec3(0.0f, -0.18f,  0.07f)
        };

        for (int i = 0; i < 4; ++i) {
            drawBox(shader, logPositions[i], logScales[i], logRots[i], logColor, logTex);
        }

        // 8. أشكال النار فوق الخشب المحترق (عدد أكبر وحجم أكبر لتناسب تكستشر النار)
        float fireBaseY = logY + openingH * 0.14f;
        float fireZ = logZ + 0.03f;

        glm::vec3 flamePositions[5] = {
            glm::vec3(position.x - openingW * 0.24f, fireBaseY,                    fireZ - 0.01f),
            glm::vec3(position.x - openingW * 0.11f, fireBaseY + openingH * 0.02f, fireZ + 0.02f),
            glm::vec3(position.x,                    fireBaseY + openingH * 0.03f, fireZ + 0.04f),
            glm::vec3(position.x + openingW * 0.12f, fireBaseY + openingH * 0.02f, fireZ + 0.02f),
            glm::vec3(position.x + openingW * 0.24f, fireBaseY,                    fireZ - 0.01f)
        };

        float flameWidths[5] = {
            openingW * 0.34f,
            openingW * 0.38f,
            openingW * 0.42f,
            openingW * 0.38f,
            openingW * 0.34f
        };

        float flameHeights[5] = {
            openingH * 0.46f,
            openingH * 0.54f,
            openingH * 0.62f,
            openingH * 0.54f,
            openingH * 0.46f
        };

        float flameRots[5] = {
            0.18f,
            -0.10f,
            0.0f,
            0.12f,
            -0.18f
        };

        for (int i = 0; i < 5; ++i) {
            drawFlameShape(shader, flamePositions[i], flameWidths[i], flameHeights[i], flameRots[i]);
        }
    }

private:
    UnitCube cube;

    void applyAppearance(const Shader& shader,
                         const glm::vec3& solidColor,
                         unsigned int texID) const {
        if (texID != 0) {
            shader.setBool("uUseSolidColor", false);
            shader.setVec3("uTintColor", glm::vec3(1.0f));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texID);
            shader.setInt("texture1", 0);
        } else {
            shader.setBool("uUseSolidColor", true);
            shader.setVec3("uSolidColor", solidColor);
        }
    }

    void drawBox(const Shader& shader,
                 const glm::vec3& pos,
                 const glm::vec3& scale,
                 const glm::vec3& rotEuler,
                 const glm::vec3& solidColor,
                 unsigned int texID) const {
        applyAppearance(shader, solidColor, texID);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, rotEuler.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotEuler.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotEuler.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        shader.setMat4("model", model);

        glBindVertexArray(cube.VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void drawFlameShape(const Shader& shader,
                        const glm::vec3& basePos,
                        float w,
                        float h,
                        float rotY) const {
        applyAppearance(shader, fireColor, fireTex);

        float flameVerts[] = {
            // positions          // texCoords
            -0.5f, 0.0f, 0.0f,    0.0f, 0.0f,
             0.5f, 0.0f, 0.0f,    1.0f, 0.0f,
             0.0f, 1.0f, 0.0f,    0.5f, 1.0f
        };

        unsigned int fVAO = 0, fVBO = 0;
        glGenVertexArrays(1, &fVAO);
        glGenBuffers(1, &fVBO);
        glBindVertexArray(fVAO);
        glBindBuffer(GL_ARRAY_BUFFER, fVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(flameVerts), flameVerts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDisable(GL_CULL_FACE);
        for (int i = 0; i < 2; ++i) {
            float angle = rotY + (i == 0 ? 0.55f : -0.55f);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, basePos);
            model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(w, h, 1.0f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        glBindVertexArray(0);
        glDeleteBuffers(1, &fVBO);
        glDeleteVertexArrays(1, &fVAO);
    }
};