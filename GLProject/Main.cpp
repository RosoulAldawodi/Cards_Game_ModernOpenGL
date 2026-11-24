#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <random>

#include "Shader.h"
#include "TextureLoader.h"
#include "Room.h"
#include "Table.h"
#include "Card.h"
#include "Poster.h"
#include "Fireplace.h"
#include "SmallCabinet.h"
#include "GltfModel.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

int activeCamera = 1; // 1 = فوق الطاولة، 2 = منظور اللاعب
bool key1Prev = false;
bool key2Prev = false;
bool keyGPrev = false;

std::mt19937 rng(std::random_device{}());

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void triggerRandomCardPlay(std::vector<Card>& cards) {
    std::vector<int> availableIndices;
    for (int i = 0; i < (int)cards.size(); ++i) {
        if (!cards[i].isPlayed) {
            availableIndices.push_back(i);
        }
    }

    // عند الضغط على G للمرة السادسة تكون القائمة فارغة فلا يحدث شيء
    if (availableIndices.empty()) {
        return;
    }

    std::uniform_int_distribution<int> dist(0, (int)availableIndices.size() - 1);
    int chosenIndex = availableIndices[dist(rng)];
    cards[chosenIndex].playCard();
}

void processInput(GLFWwindow* window, std::vector<Card>& cards) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    bool key1Now = (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS);
    bool key2Now = (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS);
    bool keyGNow = (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS);

    if (key1Now && !key1Prev) activeCamera = 1;
    if (key2Now && !key2Prev) activeCamera = 2;

    if (keyGNow && !keyGPrev) {
        triggerRandomCardPlay(cards);
    }

    key1Prev = key1Now;
    key2Prev = key2Now;
    keyGPrev = keyGNow;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Card Game Scene", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("shaders/vs/L4.vs", "shaders/fs/L4.fs");

    unsigned int floorTex = TextureLoader::loadFromFile("textures/stained_pine_diff_4k.jpg", glm::vec3(0.45f, 0.28f, 0.16f));
    unsigned int wallTex  = TextureLoader::loadFromFile("textures/wallpaper.jpg",  glm::vec3(0.76f, 0.70f, 0.60f));
    unsigned int tableTex = TextureLoader::loadFromFile("textures/lacquered_cherry_wood_diff_4k.jpg", glm::vec3(0.35f, 0.20f, 0.11f));

    unsigned int fireplaceBodyTex = TextureLoader::loadFromFile(
        "textures/stone_brick_wall_001_diff_4k.jpg",
        glm::vec3(0.42f, 0.34f, 0.27f)
    );
    unsigned int fireplaceWoodTex = TextureLoader::loadFromFile(
        "textures/brick_wall_10_diff_4k.jpg",
        glm::vec3(0.30f, 0.18f, 0.10f)
    );

    unsigned int fireplaceCavityTex = TextureLoader::loadFromFile(
        "textures/rock_wall_16_diff_4k.jpg",
        glm::vec3(0.10f, 0.08f, 0.08f)
    );
    unsigned int fireplaceLogTex = TextureLoader::loadFromFile(
        "textures/burnt_wood.jpg",
        glm::vec3(0.24f, 0.13f, 0.07f)
    );
    unsigned int fireplaceFireTex = TextureLoader::loadFromFile(
        "textures/fire.jpg",
        glm::vec3(0.90f, 0.42f, 0.10f)
    );

    float floorY = -2.0f;
    float tableTopY = 0.0f;

    float roomWidth  = 24.0f;
    float roomHeight = 8.0f;
    float roomDepth  = 24.0f;

    unsigned int ceilingTex = TextureLoader::loadFromFile("textures/ceiling.jpg", glm::vec3(0.55f, 0.48f, 0.40f));
    Room room(roomWidth, roomHeight, roomDepth, floorY, floorTex, wallTex, ceilingTex);
    Table table(3.8f, 2.3f, 0.18f, tableTopY, floorY, tableTex);

    // التحكم بأبعاد البوستر وموقعه على الجدار الخلفي مع التكستشر
    unsigned int posterTex = TextureLoader::loadFromFile(
        "textures/poster.jpg",
        glm::vec3(0.78f, 0.68f, 0.52f)
    );

    float posterWidth  = 3.5f;
    float posterHeight = 4.f;
    float posterX      = 7.2f;
    float posterY      = floorY + roomHeight * 0.5f;
    float posterZ      = -roomDepth * 0.5f + 0.02f;

    Poster wallPoster(
        glm::vec3(posterX, posterY, posterZ),
        posterWidth,
        posterHeight,
        glm::vec3(0.78f, 0.68f, 0.52f),
        posterTex
    );

    // التحكم بأبعاد المدفأة وموقعها (أعرض وأطول ومقربة قليلاً للأمام مع بقائها على الأرض)
    float fireplaceW = 5.5f;
    float fireplaceH = 4.2f;
    float fireplaceD = 1.4f;
    float fireplaceZ = -roomDepth * 0.5f + fireplaceD * 0.5f + 0.25f;

    Fireplace fireplace(
        glm::vec3(0.0f, floorY, fireplaceZ),
        fireplaceW,
        fireplaceH,
        fireplaceD
    );
    fireplace.bodyTex   = fireplaceBodyTex;
    fireplace.woodTex   = fireplaceWoodTex;
    fireplace.cavityTex = fireplaceCavityTex;
    fireplace.logTex    = fireplaceLogTex;
    fireplace.fireTex   = fireplaceFireTex;

    // خزانة صغيرة مكعبة عالأرض: حواف خارجية ورف بتكستشر خشبي، وتجويف داخلي داكن، وكتب بألوان مخصصة
    unsigned int cabinetWoodTex = TextureLoader::loadFromFile(
        "textures/lacquered_cherry_wood_diff_4k.jpg",
        glm::vec3(0.42f, 0.25f, 0.14f)
    );
    float cabinetSize    = 1.9f;
    float cornerMargin   = 0.05f;
    float cabinetOffsetX = 0.55f; // زيدي أو قللي هذا الرقم لتحريكها يمين/يسار
    float cabinetX       = -roomWidth * 0.5f + cabinetSize * 0.5f + cornerMargin + cabinetOffsetX;
    float cabinetZ       = -roomDepth * 0.5f + cabinetSize * 0.5f + cornerMargin;
    SmallCabinet smallCabinet(
        glm::vec3(cabinetX, floorY, cabinetZ),
        cabinetSize,
        0.09f,
        cabinetWoodTex
    );

    // تحميل موديل الشمعدان (.glb) ووضعه على الطاولة بالطرف اليمين واليسار مع رفع بسيط لتفادي الغوص تحت السطح
    float tableCandleYOffset = 0.5f; // عدّلي هذه القيمة لرفع أو خفض الشمعدانين على الطاولة
    GltfModel candleHolder("models/golden_glow__antique_three-arm_candelabra/candelabra.glb");
    candleHolder.position = glm::vec3(1.50f, tableTopY + tableCandleYOffset, 0.0f);
    candleHolder.scale    = glm::vec3(1.2f);
    candleHolder.rotation = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 leftCandlePos = glm::vec3(-1.50f, tableTopY + tableCandleYOffset, 0.0f);

    // شمعدان فوق سطح المدفأة العلوي (مكبّر قليلاً ومدوّر على المحور X)
    float mantelTopY = floorY + fireplaceH;
    glm::vec3 fireplaceCandlePos   = glm::vec3(0.0f, mantelTopY, fireplaceZ);
    glm::vec3 fireplaceCandleScale = glm::vec3(2.7f);
    glm::vec3 fireplaceCandleRot   = glm::vec3(glm::radians(0.0f), 45.0f, 0.0f);

    // إنشاء البطاقات الخمس مع تحميل تكستشرات card1 إلى card5 ضمن حلقة واحدة
    std::vector<Card> cards;
    const int CARD_COUNT = 5;
    float cardW = 0.42f;
    float cardH = 0.58f;
    float spacing = 0.56f;
    float startX = -((CARD_COUNT - 1) * spacing) * 0.5f;

    glm::vec3 tempColors[CARD_COUNT] = {
        glm::vec3(0.88f, 0.84f, 0.78f),
        glm::vec3(0.84f, 0.80f, 0.74f),
        glm::vec3(0.88f, 0.84f, 0.78f),
        glm::vec3(0.84f, 0.80f, 0.74f),
        glm::vec3(0.88f, 0.84f, 0.78f)
    };

    for (int i = 0; i < CARD_COUNT; ++i) {
        std::string cardTexPath = "textures/card" + std::to_string(i + 1) + ".jpg";
        unsigned int cardTex = TextureLoader::loadFromFile(cardTexPath, tempColors[i]);

        float xPos = startX + i * spacing;
        glm::vec3 deckPos(xPos, tableTopY, table.deckZoneCenter.z + cardH * 0.45f);
        glm::vec3 playedPos(xPos, tableTopY + 0.002f * i, table.playedZoneCenter.z + cardH * 0.45f);

        cards.emplace_back(deckPos, playedPos, cardW, cardH, tempColors[i], cardTex);
    }

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, cards);

        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 projection = glm::perspective(
            glm::radians(55.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f,
            100.0f
        );

        glm::mat4 view(1.0f);
        if (activeCamera == 1) {
            view = glm::lookAt(
                glm::vec3(0.0f, 2.7f, 0.05f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, -1.0f)
            );
        } else {
            view = glm::lookAt(
                glm::vec3(0.0f, 1.2f, 3.3f),
                glm::vec3(0.0f, 0.75f, -1.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        }

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        room.draw(shader);
        wallPoster.draw(shader);
        fireplace.draw(shader);
        smallCabinet.draw(shader);
        candleHolder.drawCustom(shader, fireplaceCandlePos, fireplaceCandleRot, fireplaceCandleScale);
        table.draw(shader);
        candleHolder.draw(shader);
        candleHolder.drawCustom(shader, leftCandlePos, candleHolder.rotation, candleHolder.scale);

        for (auto& card : cards) {
            card.update(deltaTime, activeCamera);
            card.draw(shader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    smallCabinet.cleanup();
    candleHolder.cleanup();
    Card::cleanupSharedMesh();
    glfwTerminate();
    return 0;
}