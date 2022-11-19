// @time   : 2022.11.18 
// @func   : the main function of the scene

#include "scene_head.h"
#ifdef MODELWITHBOX

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_m.h"
#include "camera.h"
#include "model.h"
#include <iostream>


/********************************************* 全局变量/宏定义 *********************************************/
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::vec3 lampPos(0.5f, 1.0f, 1.5f);


/********************************************* 函数定义 *********************************************/
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
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

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrComponents;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}



/********************************************* 主函数 *********************************************/
int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SCENE", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// =================== MODEL ===================
	// build and compile shader
	Shader modelShader_noneTexture("shader/model_loading.vs", "shader/model_loading_noneTexture.fs");
	Shader modelShader_withTexture("shader/model_loading.vs", "shader/model_loading_withTexture.fs");
	// load models
	Model Model_castle("../sceneMaterial/cartoonCastle/Cartoon castle.obj");
	Model Model_island("../sceneMaterial/Small Tropical Island/Small Tropical Island.obj");
	Model Model_smallIsland("../sceneMaterial/island/island.obj");

	//  =================== skybox ===================
	float skyboxVertices[] = {
		// positions          
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	// load textures
	string skyBoxPath = "../skyBoxMaterial/skyBox/";
	string skyBoxType = ".png";
	vector<std::string> faces
	{
		string(skyBoxPath + "right" + skyBoxType),
		string(skyBoxPath + "left" + skyBoxType),
		string(skyBoxPath + "top" + skyBoxType),
		string(skyBoxPath + "bottom" + skyBoxType),
		string(skyBoxPath + "front" + skyBoxType),
		string(skyBoxPath + "back" + skyBoxType),
	};
	unsigned int cubemapTexture = loadCubemap(faces);
	// construt a shader
	Shader skyboxShader("shader/skybox.vs", "shader/skybox.fs");
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);
	
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// ================================== MODEL parameter define ==================================
		// 含贴图
		GLint lightAmbientLoc = glGetUniformLocation(modelShader_withTexture.ID, "light.ambient");
		GLint lightDiffuseLoc = glGetUniformLocation(modelShader_withTexture.ID, "light.diffuse");
		GLint lightSpecularLoc = glGetUniformLocation(modelShader_withTexture.ID, "light.specular");
		GLint lightPosLoc = glGetUniformLocation(modelShader_withTexture.ID, "light.position");
		GLint attConstant = glGetUniformLocation(modelShader_withTexture.ID, "light.constant");
		GLint attLinear = glGetUniformLocation(modelShader_withTexture.ID, "light.linear");
		GLint attQuadratic = glGetUniformLocation(modelShader_withTexture.ID, "light.quadratic");
		GLint shininess = glGetUniformLocation(modelShader_withTexture.ID, "shininess");

		// 不含贴图
		GLint lightAmbientLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "light.ambient");
		GLint lightDiffuseLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "light.diffuse");
		GLint lightSpecularLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "light.specular");
		GLint lightPosLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "light.position");
		GLint attConstant_none = glGetUniformLocation(modelShader_noneTexture.ID, "light.constant");
		GLint attLinear_none = glGetUniformLocation(modelShader_noneTexture.ID, "light.linear");
		GLint attQuadratic_none = glGetUniformLocation(modelShader_noneTexture.ID, "light.quadratic");
		GLint shininess_none = glGetUniformLocation(modelShader_noneTexture.ID, "shininess");

		// 设置观察者位置
		GLint viewPosLoc = glGetUniformLocation(modelShader_withTexture.ID, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		// -------------------------------- MODEL island --------------------------------
		modelShader_withTexture.use();
		// 设置光源属性 点光源
		glUniform3f(lightAmbientLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosLoc, lampPos.x, lampPos.y, lampPos.z);
		// 设置衰减系数
		glUniform1f(attConstant, 1.0f);
		glUniform1f(attLinear, 0.09f);
		glUniform1f(attQuadratic, 0.032f);
		// 亮度
		glUniform1f(shininess, 64.0f);
		// view/projection transformations
		modelShader_withTexture.setMat4("projection", projection);
		modelShader_withTexture.setMat4("view", view);
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));			// site
		model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));		// scale
		modelShader_withTexture.setMat4("model", model);
		Model_island.Draw(modelShader_withTexture);

		// -------------------------------- MODEL castle --------------------------------
		modelShader_noneTexture.use();
		// 设置光源属性 点光源
		glUniform3f(lightAmbientLoc_none, 5.0f, 5.0f, 5.0f);
		glUniform3f(lightDiffuseLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(lightSpecularLoc_none, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosLoc_none, lampPos.x, lampPos.y, lampPos.z);
		// 设置衰减系数
		glUniform1f(attConstant_none, 1.0f);
		glUniform1f(attLinear_none, 0.09f);
		glUniform1f(attQuadratic_none, 0.032f);
		// 亮度
		glUniform1f(shininess_none, 64.0f);
		// view/projection transformations
		modelShader_noneTexture.setMat4("projection", projection);
		modelShader_noneTexture.setMat4("view", view);
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(25.0f, -4.3f, -25.0f));		   // site
		model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));		   // scale
		modelShader_noneTexture.setMat4("model", model);
		Model_castle.Draw(modelShader_noneTexture);

		// -------------------------------- MODEL smallIsland --------------------------------
		modelShader_noneTexture.use();
		// 设置光源属性 点光源
		glUniform3f(lightAmbientLoc_none, 5.0f, 5.0f, 5.0f);
		glUniform3f(lightDiffuseLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(lightSpecularLoc_none, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosLoc_none, lampPos.x, lampPos.y, lampPos.z);
		// 设置衰减系数
		glUniform1f(attConstant_none, 1.0f);
		glUniform1f(attLinear_none, 0.09f);
		glUniform1f(attQuadratic_none, 0.032f);
		// 亮度
		glUniform1f(shininess_none, 64.0f);
		// view/projection transformations
		modelShader_noneTexture.setMat4("projection", projection);
		modelShader_noneTexture.setMat4("view", view);
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-25.0f,1.0f, -15.0f));		   // site
		model = glm::scale(model, glm::vec3(1000.0, 1000.0, 1000.0));		   // scale
		modelShader_noneTexture.setMat4("model", model);
		Model_smallIsland.Draw(modelShader_noneTexture);


		// ================================== SKYBOX ==================================
		// draw skybox
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		glDepthFunc(GL_LEQUAL);									// change depth function so depth test passes when values are equal to depth buffer's content
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);									// set depth function back to default


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// delete the VAO/VBO
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

#endif // MODELWITHBOX