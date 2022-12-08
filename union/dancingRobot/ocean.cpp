#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <general/camera.h>

#include"water/Program_w.hpp"

extern GLFWwindow* window;
extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;
extern Camera camera;
extern float deltaTime;
extern float lastFrame;

// water
const unsigned SAMPLES = 8;
const unsigned TEXTURES_AMOUNT = 13;
const unsigned TESS_LEVEL = 100; //对四边形进行更细的划分
const float DEPTH = 0.02f;
unsigned heightMap[TEXTURES_AMOUNT];
unsigned normalMap[TEXTURES_AMOUNT];
unsigned waterTex;
unsigned wavesNormalMap;
unsigned wavesHeightMap;
unsigned firstIndex = 0;
unsigned lastIndex = 1;
bool rotate = false;
unsigned VAO = 0;
float interpolateFactor = 0.0f;
Program_w program_water;



void initProgram();
void initWater();
void renderWater();
unsigned int loadTexture(const char* path);
void deleteOcean();

void deleteOcean() 
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteTextures(TEXTURES_AMOUNT, heightMap);
	glDeleteTextures(TEXTURES_AMOUNT, normalMap);
	glDeleteTextures(1, &waterTex);
	glDeleteTextures(1, &wavesHeightMap);
	glDeleteTextures(1, &wavesNormalMap);
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void initProgram() {
	program_water.create();
	program_water.attachShader(createVertexShader("shaders/water/water-vert.vs"));
	program_water.attachShader(createTessalationControlShader("shaders/water/water-tess-control.glsl"));
	program_water.attachShader(createTessalationEvaluationShader("shaders/water/water-tess-eval.glsl"));
	program_water.attachShader(createFragmentShader("shaders/water/water-frag.fs"));
	program_water.link();
}

void initWater()
{
	glGenVertexArrays(1, &VAO);
	glGenTextures(TEXTURES_AMOUNT, heightMap);
	glGenTextures(TEXTURES_AMOUNT, normalMap);
	glGenTextures(1, &waterTex);
	for (unsigned i = 0; i < TEXTURES_AMOUNT; ++i) {
		std::string num = std::to_string(i + 1);
		heightMap[i] = loadTexture(("textures/heights/" + num + ".png").c_str());
		normalMap[i] = loadTexture(("textures/normals/" + num + ".png").c_str());
	}
	waterTex = loadTexture("textures/water.jpg");
	wavesNormalMap = loadTexture("textures/wavesNormal.jpg");
	wavesHeightMap = loadTexture("textures/wavesHeight.jpg");

	program_water.use();
	program_water.setVec3("light.direction", glm::vec3(0.0, -1.0, 0.0));
	program_water.setVec3("light.ambient", glm::vec3(0.15, 0.15, 0.15));
	program_water.setVec3("light.diffuse", glm::vec3(0.75, 0.75, 0.75));
	program_water.setInt("tessLevel", TESS_LEVEL);
}

void renderWater() {
	program_water.use();
	program_water.setInt("heightMap1", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightMap[firstIndex]);

	program_water.setInt("heightMap2", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightMap[lastIndex]);

	program_water.setInt("normalMap1", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalMap[firstIndex]);

	program_water.setInt("normalMap2", 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, normalMap[lastIndex]);

	program_water.setInt("water", 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	program_water.setInt("wavesHeightMap", 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, wavesHeightMap);

	program_water.setInt("wavesNormalMap", 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, wavesNormalMap);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.55f, 0.0f));
	model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
	program_water.use();
	program_water.setMat4("model", model);
	program_water.setMat4("mvp", projection * view * model);
	program_water.setVec3("viewPos", camera.Position);
	if (interpolateFactor >= 0.7)
	{
		interpolateFactor = 0.3f;
		if (lastIndex == TEXTURES_AMOUNT - 1)
		{
			firstIndex = 0;
			lastIndex = 1;
		}
		else
		{
			++firstIndex;
			++lastIndex;
		}
	}
	else
	{
		interpolateFactor += 0.03 * deltaTime;
		program_water.setFloat("interpolateFactor", interpolateFactor);
	}

	static float offset = 0.0f;
	if (offset >= INT_MAX - 2)
		offset = 0;
	//offset += 0.2 * deltaTime;
	offset += 0.002 * deltaTime;
	program_water.setFloat("wavesOffset", offset);

	glBindVertexArray(VAO);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}


