#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "Shader.h"

class SmallCabinet {
public:
    // مركز الخزانة أفقياً (X, Z) وارتفاع الأرضية (Y) الذي تستند عليه القاعدة
    glm::vec3 baseCenter = glm::vec3(0.0f);
    float size = 1.75f;          // شكل مكعب: العرض = الارتفاع = العمق
    float plankThickness = 0.09f;

    glm::vec3 cabinetColor   = glm::vec3(0.75f, 0.62f, 0.48f); // لون احتياطي إذا woodTex == 0
    glm::vec3 innerColor     = glm::vec3(0.09f, 0.05f, 0.03f); // لون التجويف الخلفي الداكن
    glm::vec3 innerSideColor = glm::vec3(0.11f, 0.06f, 0.04f); // لون أغمق للحيط الداخلي الأيسر ليفصله عن الخارج
    unsigned int woodTex     = 0;                                // تكستشر الخشب للحواف الخارجية والرف

    struct BookSpec {
        float offsetX;
        float offsetZ;
        glm::vec3 dims; // (العرض، الارتفاع، العمق)
        float tiltZ;
        glm::vec3 color;
    };

    std::vector<BookSpec> bottomBooks;
    std::vector<BookSpec> middleShelfBooks;

    SmallCabinet() {
        buildCubeMesh();
        initDefaultBooks();
    }

    SmallCabinet(const glm::vec3& centerOnFloor, float cubeSize, float thickness = 0.09f, unsigned int texID = 0)
        : baseCenter(centerOnFloor), size(cubeSize), plankThickness(thickness), woodTex(texID) {
        buildCubeMesh();
        initDefaultBooks();
    }

    ~SmallCabinet() {
        cleanup();
    }

    SmallCabinet(const SmallCabinet&) = delete;
    SmallCabinet& operator=(const SmallCabinet&) = delete;

    void draw(const Shader& shader) const {
        if (cubeVAO == 0 || cubeCount == 0) return;

        shader.use();

        float halfS = size * 0.5f;
        float t = plankThickness;
        float innerWidth = size - 2.0f * t;
        float innerDepth = size - t;
        float midY = baseCenter.y + size * 0.5f;
        float innerHeight = size - 2.0f * t;

        // 1. القاعدة السفلية الخارجية (بتكستشر الخشب)
        drawBox(
            shader,
            glm::vec3(baseCenter.x, baseCenter.y + t * 0.5f, baseCenter.z),
            glm::vec3(size, t, size),
            0.0f,
            cabinetColor,
            woodTex
        );

        // 2. السقف العلوي الخارجي (بتكستشر الخشب)
        float topY = baseCenter.y + size - t * 0.5f;
        drawBox(
            shader,
            glm::vec3(baseCenter.x, topY, baseCenter.z),
            glm::vec3(size, t, size),
            0.0f,
            cabinetColor,
            woodTex
        );

        // 3. الجدار الخلفي الداخلي المجوف (لون داكن لإبراز التجويف)
        drawBox(
            shader,
            glm::vec3(baseCenter.x, midY, baseCenter.z - halfS + t * 0.5f),
            glm::vec3(innerWidth, innerHeight, t * 0.7f),
            0.0f,
            innerColor,
            0
        );

        // شريحة رقيقة خلفية داخلية لتعميق الظل الداخلي فوق وتحت الرف
        drawBox(
            shader,
            glm::vec3(baseCenter.x, midY, baseCenter.z - halfS + t * 0.9f),
            glm::vec3(innerWidth * 0.98f, innerHeight * 0.98f, t * 0.25f),
            0.0f,
            innerColor * 0.7f,
            0
        );

        // 4. الحواف الجانبية الخارجية اليسرى واليمنى (بتكستشر الخشب)
        float leftWallCenterX  = baseCenter.x - halfS + t * 0.5f;
        float rightWallCenterX = baseCenter.x + halfS - t * 0.5f;

        drawBox(
            shader,
            glm::vec3(leftWallCenterX, midY, baseCenter.z),
            glm::vec3(t, innerHeight, size),
            0.0f,
            cabinetColor,
            woodTex
        );

        // بطانة داخلية أغمق على الحيط الداخلي الأيسر ليبين بوضوح أنو داخلي ومو متصل مع الخارج
        drawBox(
            shader,
            glm::vec3(leftWallCenterX + t * 0.42f, midY, baseCenter.z + t * 0.2f),
            glm::vec3(t * 0.22f, innerHeight * 0.99f, innerDepth * 0.94f),
            0.0f,
            innerSideColor,
            0
        );

        drawBox(
            shader,
            glm::vec3(rightWallCenterX, midY, baseCenter.z),
            glm::vec3(t, innerHeight, size),
            0.0f,
            cabinetColor,
            woodTex
        );

        // 5. الرف الوسطي (بتكستشر الخشب)
        float middleShelfCenterY = baseCenter.y + size * 0.5f;
        drawBox(
            shader,
            glm::vec3(baseCenter.x, middleShelfCenterY, baseCenter.z + t * 0.25f),
            glm::vec3(innerWidth, t * 0.85f, innerDepth),
            0.0f,
            cabinetColor,
            woodTex
        );

        // 6. كتب في الجزء التحتاني (فوق القاعدة السفلية)
        float bottomSupportY = baseCenter.y + t;
        for (const auto& b : bottomBooks) {
            drawBox(
                shader,
                glm::vec3(baseCenter.x + b.offsetX, bottomSupportY + b.dims.y * 0.5f, baseCenter.z + b.offsetZ),
                b.dims,
                b.tiltZ,
                b.color,
                0
            );
        }

        // 7. كتب فوق الرف الوسطي
        float middleSupportY = middleShelfCenterY + (t * 0.85f) * 0.5f;
        for (const auto& b : middleShelfBooks) {
            drawBox(
                shader,
                glm::vec3(baseCenter.x + b.offsetX, middleSupportY + b.dims.y * 0.5f, baseCenter.z + b.offsetZ),
                b.dims,
                b.tiltZ,
                b.color,
                0
            );
        }
    }

    void cleanup() {
        if (cubeEBO != 0) { glDeleteBuffers(1, &cubeEBO); cubeEBO = 0; }
        if (cubeVBO != 0) { glDeleteBuffers(1, &cubeVBO); cubeVBO = 0; }
        if (cubeVAO != 0) { glDeleteVertexArrays(1, &cubeVAO); cubeVAO = 0; }
        cubeCount = 0;
    }

private:
    unsigned int cubeVAO = 0, cubeVBO = 0, cubeEBO = 0;
    int cubeCount = 0;

    void initDefaultBooks() {
        bottomBooks.clear();
        middleShelfBooks.clear();

        glm::vec3 beigeColor        = glm::vec3(0.76f, 0.67f, 0.52f); // بيج
        glm::vec3 lightBrownColor   = glm::vec3(0.56f, 0.36f, 0.20f); // بني فاتح
        glm::vec3 darkOliveColor    = glm::vec3(0.08f, 0.13f, 0.06f); // أخضر زيتوني غامق جداً
        glm::vec3 darkBurgundyColor = glm::vec3(0.18f, 0.04f, 0.06f); // برغندي غامق جداً

        // كتب إضافية قليلة في الجزء التحتاني
        bottomBooks.push_back({ -0.56f, 0.12f, glm::vec3(0.10f, 0.41f, 0.35f), 0.0f, lightBrownColor });
        bottomBooks.push_back({ -0.45f, 0.12f, glm::vec3(0.11f, 0.44f, 0.36f), 0.0f, darkBurgundyColor });
        bottomBooks.push_back({ -0.33f, 0.12f, glm::vec3(0.10f, 0.40f, 0.34f), 0.0f, darkOliveColor });
        bottomBooks.push_back({ -0.22f, 0.12f, glm::vec3(0.10f, 0.43f, 0.35f), 0.0f, beigeColor });
        bottomBooks.push_back({ -0.09f, 0.12f, glm::vec3(0.10f, 0.41f, 0.35f), glm::radians(-8.0f), lightBrownColor });

        // كتب إضافية قليلة فوق الرف الوسطي
        middleShelfBooks.push_back({ -0.46f, 0.14f, glm::vec3(0.09f, 0.35f, 0.31f), 0.0f, darkOliveColor });
        middleShelfBooks.push_back({ -0.35f, 0.14f, glm::vec3(0.10f, 0.36f, 0.32f), 0.0f, lightBrownColor });
        middleShelfBooks.push_back({ -0.23f, 0.14f, glm::vec3(0.09f, 0.32f, 0.30f), 0.0f, beigeColor });
        middleShelfBooks.push_back({ -0.12f, 0.14f, glm::vec3(0.09f, 0.34f, 0.31f), 0.0f, darkBurgundyColor });
        middleShelfBooks.push_back({  0.00f, 0.14f, glm::vec3(0.09f, 0.33f, 0.31f), glm::radians(-6.0f), beigeColor });
    }

    void drawBox(const Shader& shader,
                 const glm::vec3& centerPos,
                 const glm::vec3& boxSize,
                 float tiltZ,
                 const glm::vec3& color,
                 unsigned int texID) const {
        glm::mat4 model(1.0f);
        model = glm::translate(model, centerPos);
        if (tiltZ != 0.0f) {
            model = glm::rotate(model, tiltZ, glm::vec3(0.0f, 0.0f, 1.0f));
        }
        model = glm::scale(model, boxSize);

        shader.setMat4("model", model);

        if (texID != 0) {
            shader.setBool("uUseSolidColor", false);
            shader.setVec3("uTintColor", glm::vec3(1.0f));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texID);
            shader.setInt("texture1", 0);
        } else {
            shader.setBool("uUseSolidColor", true);
            shader.setVec3("uSolidColor", color);
        }

        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, cubeCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void buildCubeMesh() {
        cleanup();

        std::vector<float> verts = {
            // أمامي (+Z)
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            // خلفي (-Z)
             0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            // يسار (-X)
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            // يمين (+X)
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            // علوي (+Y)
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            // سفلي (-Y)
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 1.0f
        };

        std::vector<unsigned int> idx;
        for (unsigned int face = 0; face < 6; ++face) {
            unsigned int base = face * 4;
            idx.insert(idx.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
        }

        cubeCount = static_cast<int>(idx.size());

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glGenBuffers(1, &cubeEBO);

        glBindVertexArray(cubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};