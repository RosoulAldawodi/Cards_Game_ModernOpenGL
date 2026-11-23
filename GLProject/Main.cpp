#include "libs/glad_loader_docs.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "PolygonWithTexture.cpp"
#include "Polygon.cpp"
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include "stb_image.h"

using namespace glm;

int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;

Camera camera(vec3(0.0f, 0.0f, 4.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

int main()
{
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lecture 4", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glEnable(GL_DEPTH_TEST);

	stbi_set_flip_vertically_on_load(true);

	Shader ourShader("./shaders/vs/L4.vs", "./shaders/fs/L4.fs");

	// Model testModel("./models/backpack/backpack.obj");

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// float borderColor[] = { 0.44f, 0.37f, 0.02f, 1.0f };
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nrChannels;
	unsigned char *data = stbi_load("./textures/crate.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	stbi_image_free(data);

	std::vector<vec3> vertices1 = {};
	vertices1.push_back(vec3(-0.5f, -0.5f, -0.5f));
	vertices1.push_back(vec3(-0.5f, 0.5f, -0.5f));
	vertices1.push_back(vec3(0.5f, 0.5f, -0.5f));
	vertices1.push_back(vec3(0.5f, -0.5f, -0.5f));

	std::vector<vec2> texcoords1 = {};
	texcoords1.push_back(vec2(0.0f, 0.0f));
	texcoords1.push_back(vec2(0.0f, 1.0f));
	texcoords1.push_back(vec2(1.0f, 1.0f));
	texcoords1.push_back(vec2(1.0f, 0.0f));
	// Polygon Face1 = Polygon(vertices1, vec3(1.0f, 0.0f, 0.0f)); // Red
	PolygonWithTexture Face1 = PolygonWithTexture(vertices1, texcoords1);
	Face1.setTexture(texture);

	std::vector<vec3> vertices2 = {};
	vertices2 = vertices1;
	texcoords1 = {};

	texcoords1.push_back(vec2(1.0f, 0.0f));
	texcoords1.push_back(vec2(1.0f, 1.0f));
	texcoords1.push_back(vec2(0.0f, 1.0f));
	texcoords1.push_back(vec2(0.0f, 0.0f));
	// Polygon Face2 = Polygon(vertices2, vec3(1.0f, 1.0f, 0.0f)); // Yellow
	PolygonWithTexture Face2 = PolygonWithTexture(vertices2, texcoords1);
	Face2.setTexture(texture);

	std::vector<vec3> vertices3 = {};
	vertices3.push_back(vec3(-0.5f, -0.5f, -0.5f));
	vertices3.push_back(vec3(-0.5f, -0.5f, 0.5f));
	vertices3.push_back(vec3(0.5f, -0.5f, 0.5f));
	vertices3.push_back(vec3(0.5f, -0.5f, -0.5f));
	Polygon Face3 = Polygon(vertices3, vec3(0.0f, 0.0f, 1.0f)); // Blue

	std::vector<vec3> vertices4 = {};
	vertices4 = vertices3;
	Polygon Face4 = Polygon(vertices4, vec3(1.0f, 0.0f, 1.0f)); // Purple

	std::vector<vec3> vertices5 = {};
	vertices5.push_back(vec3(-0.5f, -0.5f, 0.5f));
	vertices5.push_back(vec3(-0.5f, 0.5f, 0.5f));
	vertices5.push_back(vec3(-0.5f, 0.5f, -0.5f));
	vertices5.push_back(vec3(-0.5f, -0.5f, -0.5f));

	std::vector<vec2> texcoords2 = {};
	texcoords2.push_back(vec2(0.0f, 0.0f));
	texcoords2.push_back(vec2(0.0f, 1.0f));
	texcoords2.push_back(vec2(1.0f, 1.0f));
	texcoords2.push_back(vec2(1.0f, 0.0f));
	// Polygon Face5 = Polygon(vertices5, vec3(0.0f, 1.0f, 0.0f)); // Green
	PolygonWithTexture Face5 = PolygonWithTexture(vertices5, texcoords2);
	Face5.setTexture(texture);

	std::vector<vec3> vertices6 = {};
	vertices6 = vertices5;
	texcoords2 = {};
	texcoords2.push_back(vec2(1.0f, 0.0f));
	texcoords2.push_back(vec2(1.0f, 1.0f));
	texcoords2.push_back(vec2(0.0f, 1.0f));
	texcoords2.push_back(vec2(0.0f, 0.0f));
	// Polygon Face6 = Polygon(vertices6, vec3(0.0f, 1.0f, 1.0f)); // Cyan
	PolygonWithTexture Face6 = PolygonWithTexture(vertices6, texcoords2);
	Face6.setTexture(texture);

	ourShader.use();

	mat4 projection = mat4(1.0f);
	projection = perspective(radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	// float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
	// projection = ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, 0.1f, 100.0f);
	ourShader.setMat4("projection", projection);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 view = mat4(1.0f);
		view = view = camera.GetViewMatrix();
		ourShader.setMat4("view", view);

		float time = (float)glfwGetTime();
		vec3 axis = vec3(0.0f, 1.0f, 0.0f);

		// Faces with rotation only
		// mat4 baseRotation = mat4(1.0f);
		mat4 baseRotation = rotate(mat4(1.0f), time, axis);
		Face1.transformation(baseRotation);
		Face3.transformation(baseRotation);
		Face5.transformation(baseRotation);

		// Faces with rotation + translation
		Face2.transformation(translate(baseRotation, vec3(0.0f, 0.0f, 1.0f)));
		Face4.transformation(translate(baseRotation, vec3(0.0f, 1.0f, 0.0f)));
		Face6.transformation(translate(baseRotation, vec3(1.0f, 0.0f, 0.0f)));

		// ourShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));
		// ourShader.setMat4("model", mat4(1.0f));
		// testModel.Draw(ourShader);

		Face1.draw(ourShader);
		Face2.draw(ourShader);
		Face3.draw(ourShader);
		Face4.draw(ourShader);
		Face5.draw(ourShader);
		Face6.draw(ourShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}
