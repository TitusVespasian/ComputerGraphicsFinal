#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <general/shader_m.h>
#include <general/camera.h>
#include <animation/animator.h>
#include <animation/model_animation.h>
#include <skybox.hpp>
#


#include <iostream>

/********************************************* 函数声明 *********************************************/
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

/********************************************* 全局变量/宏定义 *********************************************/
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 30.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("shaders/model/anim_model.vs", "shaders/model/anim_model.fs");


	// load models
	// -----------
	Model ourModel("resources/objects/girl/Hip Hop Dancing.dae");
	Animation danceAnimation("resources/objects/girl/Hip Hop Dancing.dae", &ourModel);
	Animator animator(&danceAnimation);//start of the animator
	Model backgroundModel("resources/backpack/backpack.obj");
	//scene
	//-----------
	Model Model_castle("resources/sceneMaterial/cartoonCastle/Cartoon castle.obj",true);
	Model Model_island("resources/sceneMaterial/Small Tropical Island/Small Tropical Island.obj",true);
	Model Model_smallIsland("resources/sceneMaterial/island/island.obj", true);

	//load skybox
	//-----------
	skybox skyBoxI;

	// build and compile shaders
	// -------------------------
	Shader backgroundShader("shaders/model/1.model_loading.vs", "shaders/model/1.model_loading.fs");
	Shader skyboxShader("shaders/scene/skybox.vs", "shaders/scene/skybox.fs");
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);
	//scene
	Shader modelShader_noneTexture("shaders/scene/model_loading.vs", "shaders/scene/model_loading_noneTexture.fs");
	Shader modelShader_withTexture("shaders/scene/model_loading.vs", "shaders/scene/model_loading_withTexture.fs");

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		animator.UpdateAnimation(deltaTime);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);



		auto transforms = animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);


		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);

		//backgroundShader.use();
		//backgroundShader.setMat4("projection", projection);
		//backgroundShader.setMat4("view", view);
		//model = glm::translate(model, glm::vec3(-1.0f, -1.0f, -1.0f)); // translate it down so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));	// it's a bit too big for our scene, so scale it down
		//backgroundShader.setMat4("model", model);
		//backgroundModel.Draw(backgroundShader);

		//skybox
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		skyBoxI.Draw();

		//scene
		glm::vec3 lampPos(0.5f, 1.0f, 1.5f);
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
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		model = glm::mat4(1.0f);

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
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));		// scale
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
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));		   // scale
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
		model = glm::translate(model, glm::vec3(-25.0f, 1.0f, -15.0f));		   // site
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));		   // scale
		modelShader_noneTexture.setMat4("model", model);
		Model_smallIsland.Draw(modelShader_noneTexture);



		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

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
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}
