#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <string>
#include <iostream>

class TextureLoader {
public:
    static unsigned int loadFromFile(const std::string& path, const glm::vec3& fallbackColor = glm::vec3(0.5f)) {
        stbi_set_flip_vertically_on_load(true);

        int width = 0, height = 0, nrChannels = 0;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

        unsigned int textureID = 0;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        if (data && width > 0 && height > 0) {
            GLenum format = GL_RGB;
            if (nrChannels == 1) {
                format = GL_RED;
            } else if (nrChannels == 3) {
                format = GL_RGB;
            } else if (nrChannels == 4) {
                format = GL_RGBA;
            }

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            if (nrChannels == 1) {
                GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }

            stbi_image_free(data);
        } else {
            std::cerr << "[Fallback Texture] Could not load: " << path << " -> using solid fallback color.\n";
            if (data) {
                stbi_image_free(data);
            }

            unsigned char pixel[3] = {
                static_cast<unsigned char>(glm::clamp(fallbackColor.r, 0.0f, 1.0f) * 255.0f),
                static_cast<unsigned char>(glm::clamp(fallbackColor.g, 0.0f, 1.0f) * 255.0f),
                static_cast<unsigned char>(glm::clamp(fallbackColor.b, 0.0f, 1.0f) * 255.0f)
            };

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        return textureID;
    }
};