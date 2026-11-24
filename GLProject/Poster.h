#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

class Poster {
public:
    glm::vec3 position;
    float width;
    float height;
    unsigned int textureID;
    glm::vec3 placeholderColor;

    Poster(const glm::vec3& pos, float w, float h,
           const glm::vec3& tempColor = glm::vec3(0.78f, 0.68f, 0.52f),
           unsigned int texID = 0)
        : position(pos), width(w), height(h), textureID(texID), placeholderColor(tempColor) {
        setupTornMesh();
    }

    ~Poster() {
        if (VAO != 0) glDeleteVertexArrays(1, &VAO);
        if (VBO != 0) glDeleteBuffers(1, &VBO);
    }

    void setTexture(unsigned int texID) {
        textureID = texID;
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
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(width, height, 1.0f));
        shader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }

private:
    unsigned int VAO = 0, VBO = 0;
    int vertexCount = 0;

    // تمزق غير متماثل وواقعي في الزاوية السفلية اليمنى بدون تكرار منتظم للنتوءات
    void setupTornMesh() {
        glm::vec2 pTL(-0.50f,  0.50f); // أعلى يسار سليم
        glm::vec2 pTR( 0.50f,  0.50f); // أعلى يمين سليم
        glm::vec2 pMR( 0.50f, -0.24f); // نهاية الجزء السليم من الحافة اليمنى

        // نقاط تمزق غير متشابهة: أطوال وزوايا وانكسارات مختلفة لتبدو طبيعية
        glm::vec2 pTear1( 0.45f, -0.27f); // انكسار خفيف
        glm::vec2 pTear2( 0.49f, -0.31f); // نتوء رفيع بارز
        glm::vec2 pTear3( 0.38f, -0.34f); // دخول أعمق للداخل
        glm::vec2 pTear4( 0.44f, -0.365f);// لسان قصير مائل
        glm::vec2 pTear5( 0.40f, -0.39f); // ثلمة صغيرة
        glm::vec2 pTear6( 0.46f, -0.42f); // نتوء عريض نسبياً قرب الأسفل
        glm::vec2 pTear7( 0.33f, -0.45f); // انكسار واضح نحو اليسار
        glm::vec2 pTear8( 0.36f, -0.475f);// نتوء صغير غير منتظم
        glm::vec2 pTear9( 0.29f, -0.485f);// انحدار ناعم
        glm::vec2 pTear10(0.31f, -0.497f);// طرف دقيق جداً
        glm::vec2 pTear11(0.25f, -0.50f); // التقاء التمزق بالحافة السفلية

        glm::vec2 pBL(-0.50f, -0.50f); // أسفل يسار سليم

        auto uvFromPos = [](const glm::vec2& p) -> glm::vec2 {
            return glm::vec2(p.x + 0.5f, p.y + 0.5f);
        };

        glm::vec2 hub(-0.05f, 0.05f);
        glm::vec2 contour[] = {
            pTL, pTR, pMR,
            pTear1, pTear2, pTear3, pTear4, pTear5, pTear6,
            pTear7, pTear8, pTear9, pTear10, pTear11,
            pBL, pTL
        };

        const int segCount = 15;
        float verts[segCount * 3 * 5];
        int idx = 0;

        auto pushVert = [&](const glm::vec2& pos) {
            glm::vec2 uv = uvFromPos(pos);
            verts[idx++] = pos.x;
            verts[idx++] = pos.y;
            verts[idx++] = 0.0f;
            verts[idx++] = uv.x;
            verts[idx++] = uv.y;
        };

        for (int i = 0; i < segCount; ++i) {
            pushVert(hub);
            pushVert(contour[i]);
            pushVert(contour[i + 1]);
        }

        vertexCount = segCount * 3;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }
};