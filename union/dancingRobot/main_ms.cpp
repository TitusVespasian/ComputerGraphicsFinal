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

#include <iostream>
#include <wtypes.h>


/********************************************* 函数声明 *********************************************/
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

/********************************************* 全局变量/宏定义 *********************************************/
// settings
long SCR_WIDTH = 1422;
long SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(20.0f, 3.0f, -20.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//animation id
const int a0 = 0;
const int a1 = 1;
const int a2 = 2;
const int a3 = 3;
const int a4 = 4;

//animation start time
const float s0 = 0.0f;
const float s1 = 5667.0f;
const float s2 = 9500.0f;
const float s3 = 14500.0f;
const float s4 = 23233.0f;

//animation end time
const float e0 = 1433.0f;
const float e1 = 8000.0f;
const float e2 = 10867.0f;
const float e3 = 18933.0f;
const float e4 = 24533.0f;


//
const glm::vec3 lampPos(0.0f, 20.0f, 0.0f);

//model position
const glm::vec3 castlePos = glm::vec3(25.0f, -4.3f, -25.0f);
const glm::vec3 islandPos = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 smallIslandPos = glm::vec3(-25.0f, 1.0f, -15.0f);
const glm::vec3 dirLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);


#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	//GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "JustDance", NULL, NULL);
	int monitorCount = 0;
	GLFWmonitor** pMonitor = glfwGetMonitors(&monitorCount);
	int holographic_screen = -1;
	GLFWwindow* window = NULL;
	int w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN);
	for (int i = 0; i < monitorCount; i++)
	{
		GLFWvidmode* mode = (GLFWvidmode*)glfwGetVideoMode(pMonitor[i]);
		if (mode->width == w && h == mode->height)
		{
			holographic_screen = i;
			window = glfwCreateWindow(w, h, "", pMonitor[holographic_screen], NULL);
		}
	}
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
	glEnable(GL_MULTISAMPLE);
	// build and compile shaders
	// -------------------------
	Shader ourShader("shaders/model/anim_model.vs", "shaders/model/anim_model.fs");


	// load models
	// -----------
	Model ourModel("resources/objects/dance/Swing Dancing.dae");
	Animation danceAnimation("resources/objects/dance/Swing Dancing.dae", &ourModel);
	Animator animator(&danceAnimation);//start of the animator
	//Model backgroundModel("resources/backpack/backpack.obj");
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

	//animation id
	int act = a0;

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

		//change the animation
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			animator.setCurrentTime(s1);
			act = a1;
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			animator.setCurrentTime(s2);
			act = a2;
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			animator.setCurrentTime(s3);
			act = a3;
		}
		else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			animator.setCurrentTime(s4);
			act = a4;
		}

		//keep the animation
		if (act == a0 && animator.getCurrentTime() >= e0) {
			animator.setCurrentTime(s0);
		}
		else if (act == a1 && animator.getCurrentTime() >= e1) {
			animator.setCurrentTime(s1);
		}
		else if (act == a2 && animator.getCurrentTime() >= e2) {
			animator.setCurrentTime(s2);
		}
		else if (act == a3 && animator.getCurrentTime() >= e3) {
			animator.setCurrentTime(s3);
		}
		else if (act == a4 && animator.getCurrentTime() >= e4) {
			animator.setCurrentTime(s4);
		}

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
		model = glm::translate(model, glm::vec3(22.7f, 1.2f, -22.7f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.f));
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
		GLint pointLightAmbientLoc = glGetUniformLocation(modelShader_withTexture.ID, "pointLights[0].ambient");
		GLint pointLightDiffuseLoc = glGetUniformLocation(modelShader_withTexture.ID, "pointLights[0].diffuse");
		GLint pointLightSpecularLoc = glGetUniformLocation(modelShader_withTexture.ID, "pointLights[0].specular");
		GLint pointLightPosLoc = glGetUniformLocation(modelShader_withTexture.ID, "pointLights[0].position");
		GLint attConstant = glGetUniformLocation(modelShader_withTexture.ID, "pointLights[0].constant");
		GLint attLinear = glGetUniformLocation(modelShader_withTexture.ID, "pointLights[0].linear");
		GLint attQuadratic = glGetUniformLocation(modelShader_withTexture.ID, "pointLights[0].quadratic");
		GLint shininess = glGetUniformLocation(modelShader_withTexture.ID, "shininess");
		GLint dirLightAmbientLoc = glGetUniformLocation(modelShader_withTexture.ID, "dirLight.ambient");
		GLint dirLightDiffuseLoc = glGetUniformLocation(modelShader_withTexture.ID, "dirLight.diffuse");
		GLint dirLightSpecularLoc = glGetUniformLocation(modelShader_withTexture.ID, "dirLight.specular");
		GLint dirLightDirectionLoc = glGetUniformLocation(modelShader_withTexture.ID, "dirLight.position");

		// 不含贴图
		GLint pointLightAmbientLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "pointLights[0].ambient");
		GLint pointLightDiffuseLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "pointLights[0].diffuse");
		GLint pointLightSpecularLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "pointLights[0].specular");
		GLint pointLightPosLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "pointLights[0].position");
		GLint attConstant_none = glGetUniformLocation(modelShader_noneTexture.ID, "pointLights[0].constant");
		GLint attLinear_none = glGetUniformLocation(modelShader_noneTexture.ID, "pointLights[0].linear");
		GLint attQuadratic_none = glGetUniformLocation(modelShader_noneTexture.ID, "pointLights[0].quadratic");
		GLint shininess_none = glGetUniformLocation(modelShader_noneTexture.ID, "shininess");
		GLint dirLightAmbientLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "dirLight.ambient");
		GLint dirLightDiffuseLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "dirLight.diffuse");
		GLint dirLightSpecularLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "dirLight.specular");
		GLint dirLightDirectionLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "dirLight.direction");
		// 设置观察者位置
		GLint viewPosLoc = glGetUniformLocation(modelShader_withTexture.ID, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		model = glm::mat4(1.0f);


		// -------------------------------- MODEL island --------------------------------
		modelShader_withTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc, 0.8f, 0.8f, 0.9f);
		glUniform3f(dirLightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 landLightPos = glm::vec3(islandPos.x, islandPos.y + 5, islandPos.z);
		glUniform3f(pointLightAmbientLoc, 0.7f, 0.7f, 0.8f);
		glUniform3f(pointLightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc, landLightPos.x, landLightPos.y, landLightPos.z);
		// 设置衰减系数
		glUniform1f(attConstant, 1.0f);
		glUniform1f(attLinear, 0.009f);
		glUniform1f(attQuadratic, 0.032f);
		// 亮度
		glUniform1f(shininess, 64.0f);
		// view/projection transformations
		modelShader_withTexture.setMat4("projection", projection);
		modelShader_withTexture.setMat4("view", view);
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::translate(model, islandPos);			// site

		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));		// scale
		modelShader_withTexture.setMat4("model", model);
		Model_island.Draw(modelShader_withTexture);

		// -------------------------------- MODEL castle --------------------------------
		modelShader_noneTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc_none, 0.1f, 0.1f, 0.1f);
		glUniform3f(dirLightDiffuseLoc_none, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc_none, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 castleLightPos = glm::vec3(castlePos.x, castlePos.y + 6, castlePos.z);
		glUniform3f(pointLightAmbientLoc_none, 0.95f, 0.8f, 0.85f);
		glUniform3f(pointLightDiffuseLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc_none, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc_none, castleLightPos.x, castleLightPos.y, castleLightPos.z);
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
		model = glm::translate(model, castlePos);		   // site
		//model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));		   // scale
		modelShader_noneTexture.setMat4("model", model);
		Model_castle.Draw(modelShader_noneTexture);

		// -------------------------------- MODEL smallIsland --------------------------------
		modelShader_noneTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc_none, 0.40f, 0.40f, 0.40f);
		glUniform3f(dirLightDiffuseLoc_none, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc_none, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 SLandLightPos = glm::vec3(smallIslandPos.x, smallIslandPos.y + 2, smallIslandPos.z);
		glUniform3f(pointLightAmbientLoc_none, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightDiffuseLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc_none, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc_none, SLandLightPos.x, SLandLightPos.y, SLandLightPos.z);
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
		model = glm::translate(model, smallIslandPos);		   // site
		model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));		   // scale
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
