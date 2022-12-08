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



/********************************************* �������� *********************************************/
void initGL();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void renderScene(Shader& shader, Model& Model_stage,Model& Model_castle, Model& dancer);
void RenderQuad();
/********************************************* ȫ�ֱ���/�궨�� *********************************************/

//window
GLFWwindow* window;

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(-5.0f, 3.0f, 8.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//animation id
const int a0 = 0;
const int a1 = 1;//up
const int a2 = 2;//down
const int a3 = 3;//left
const int a4 = 4;//right

//30fps
//animation start time
const float s0 = 0.0f;
const float s1 = 180.0f;
const float s2 = 360.0f;
const float s3 = 450.0f;
const float s4 = 570.0f;

//animation end time
const float e0 = 141.0f;
const float e1 = 315.0f;
const float e2 = 418.0f;
const float e3 = 521.0f;
const float e4 = 685.0f;

//model position
const glm::vec3 castlePos = glm::vec3(0.0f, -4.3f, -0.0f);
const glm::vec3 islandPos = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 smallIslandPos = glm::vec3(-25.0f, 1.0f, -15.0f);
const glm::vec3 stagePos = glm::vec3(25.0f, 1.6f, -5.0f);

const glm::vec3 dirLightDirection = glm::vec3(2.0f, -3.0f, 0.0f);
const glm::vec3 dirLightPos = glm::vec3(20.0f, 20.0f, -20.0f);

//��Ļ�ı��ε�VA0\VBO(�õ�)
GLuint quadVAO = 0;
GLuint quadVBO;

// ocean
extern void initProgram();
extern void initWater();
extern void renderWater();
extern void deleteOcean();

int main()
{
	initGL();

	//water
	initProgram();
	initWater();

	// build and compile shaders
    // -------------------------
	Shader ourShader("shaders/model/anim_model.vs", "shaders/model/anim_model.fs");

	// load models
	// -----------
	Model ourModel("resources/objects/dance/2.fbx");
	Animation danceAnimation("resources/objects/dance/2.fbx", &ourModel);
	Animator animator(&danceAnimation);//start of the animator

	//scene
	//-----------
	Model Model_castle("resources/sceneMaterial/cartoonCastle/Cartoon castle.obj", true);
	Model Model_stage("resources/sceneMaterial/Stage/stage.obj", true);

	//load skybox
	//-----------
	skybox skyBoxI;

	// build and compile shaders
	// -------------------------
	Shader skyboxShader("shaders/scene/skybox.vs", "shaders/scene/skybox.fs");
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	//scene
	Shader modelShader_noneTexture("shaders/scene/model_loading.vs", "shaders/scene/model_loading_noneTexture.fs");
	Shader modelShader_withTexture("shaders/scene/model_loading.vs", "shaders/scene/model_loading_withTexture.fs");

	//shadow 
	Shader DepthShader("shaders/scene/dirShadow.vs", "shaders/scene/dirShadow.fs");
	Shader testShader("shaders/scene/shadowTest.vs", "shaders/scene/shadowTest.fs");

	//animation id
	int act = a0;

	//**************SHADOW TEST************//
	/*������Ӱ��ͼ*/
	unsigned int depthMapFBO, depthMap;
	glGenFramebuffers(1, &depthMapFBO); //����һ��֡�������
	const GLuint SHADOW_WIDTH = SCR_WIDTH, SHADOW_HEIGHT = SCR_HEIGHT;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);//�����ͼ
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//�������ͼ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	//�����ɵ����������Ϊ֡�������Ȼ���
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);//�������ͼ��֡����
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//���ò����κ���ɫ
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		cout << "somthing wrong with frame buffer!";
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
		// render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//START TO RENDER EVERYTHING
		/* STEP1---��Ⱦ�����ͼ */
		//glCullFace(GL_FRONT);
		//��Դ�ռ�ı任
		GLfloat near_plane = 0.1f, far_plane = 30.0f;
		glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, near_plane, far_plane);
		//glm::mat4 lightView = glm::lookAt(dirLightPos, dirLightPos + dirLightDirection, glm::vec3(0.0, 1.0, 0.0));//camera.Position
		glm::vec3 delt = glm::vec3(0.0, 5.0, 0.0);
		glm::mat4 lightView = glm::lookAt(camera.Position + delt, camera.Position + delt + dirLightDirection, glm::vec3(0.0, 1.0, 0.0));//camera.Position
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		DepthShader.use();
		GLint lightSpaceMatrixLocation = glGetUniformLocation(DepthShader.ID, "lightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderScene(DepthShader, Model_stage, Model_castle, ourModel);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glCullFace(GL_BACK);

		/* STEP2---��ȾdepthMap������ ���ӻ� */
		// Reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//to debug only��RenderQuad��ʾ��ǰ�����ͼ
		if (0) {
			testShader.use();
			GLuint shadow_loc = glGetUniformLocation(testShader.ID, "shadowMap");
			glUniform1i(shadow_loc, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			RenderQuad();
			std::cout << "testing..." << std::endl;
		}

		/* STEP3----render the scene&model */
		//�����л��ж�
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
		if (act == a0 && animator.getCurrentTime(deltaTime) >= e0) {
			animator.setCurrentTime(s0);
		}
		else if (act == a1 && animator.getCurrentTime(deltaTime) >= e1) {
			animator.setCurrentTime(s1);
		}
		else if (act == a2 && animator.getCurrentTime(deltaTime) >= e2) {
			animator.setCurrentTime(s2);
		}
		else if (act == a3 && animator.getCurrentTime(deltaTime) >= e3) {
			animator.setCurrentTime(s3);
		}
		else if (act == a4) {
			float t = animator.getCurrentTime(deltaTime);
			if (t >= e4 || t < s4)
				animator.setCurrentTime(s4);
		}

		animator.UpdateAnimation();
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
		model = glm::translate(model, glm::vec3(-25.0f, 0.0f, 25.0f));
		model = glm::translate(model, glm::vec3(22.7f, 1.2f, -22.7f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.f));
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);

		//skybox
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		skyBoxI.Draw();

		//water
		renderWater();

		//scene
		// ================================== MODEL parameter define ==================================
		// ����ͼ
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
		GLint dirLightDirectionLoc = glGetUniformLocation(modelShader_withTexture.ID, "dirLight.direction");
		GLint isPointLightLoc = glGetUniformLocation(modelShader_withTexture.ID, "is_pointlight");
		GLuint shadowLoc = glGetUniformLocation(modelShader_withTexture.ID, "shadowMap");
		// ������ͼ
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
		GLint isPointLightLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "is_pointlight");
		GLuint shadowLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "shadowMap");
		// ���ù۲���λ��
		GLint viewPosLoc = glGetUniformLocation(modelShader_withTexture.ID, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		model = glm::mat4(1.0f);

		// -------------------------------- MODEL stage --------------------------------

		modelShader_withTexture.use();
		// ���ù�Դ���� ƽ�й�Դ
		glUniform3f(dirLightAmbientLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDiffuseLoc, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// ���ù�Դ���� ���Դ
		glm::vec3 stageLightPos = glm::vec3(stagePos.x, stagePos.y + 5, stagePos.z);
		glUniform1i(isPointLightLoc, true);//���Դ
		glUniform3f(pointLightAmbientLoc, 0.9f, 0.3f, 0.9f);
		glUniform3f(pointLightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc, stageLightPos.x, stageLightPos.y, stageLightPos.z);
		// ����˥��ϵ��
		glUniform1f(attConstant, 1.0f);
		glUniform1f(attLinear, 0.09f);
		glUniform1f(attQuadratic, 0.32f);
		// ����
		glUniform1f(shininess, 64.0f);
		// view/projection transformations
		modelShader_withTexture.setMat4("projection", projection);
		modelShader_withTexture.setMat4("view", view);
		//��ռ��ӽǱ任����
		glUniformMatrix4fv(glGetUniformLocation(modelShader_withTexture.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
		model = glm::translate(model, stagePos);			// site
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));		// scale
		modelShader_withTexture.setMat4("model", model);
		glUniform1i(shadowLoc, 9);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		Model_stage.Draw(modelShader_withTexture);

		// -------------------------------- MODEL castle --------------------------------
		modelShader_noneTexture.use();
		// ���ù�Դ���� ƽ�й�Դ
		glUniform3f(dirLightAmbientLoc_none, 0.1f, 0.1f, 0.1f);
		glUniform3f(dirLightDiffuseLoc_none, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc_none, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// ���ù�Դ���� ���Դ
		glm::vec3 castleLightPos = glm::vec3(castlePos.x, castlePos.y + 6, castlePos.z);
		glUniform1i(isPointLightLoc_none, true);//���Դ
		glUniform3f(pointLightAmbientLoc_none, 0.95f, 0.8f, 0.85f);
		glUniform3f(pointLightDiffuseLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc_none, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc_none, castleLightPos.x, castleLightPos.y, castleLightPos.z);
		// ����˥��ϵ��
		glUniform1f(attConstant_none, 1.0f);
		glUniform1f(attLinear_none, 0.29f);
		glUniform1f(attQuadratic_none, 0.32f);
		// ����
		glUniform1f(shininess_none, 64.0f);
		// view/projection transformations
		modelShader_noneTexture.setMat4("projection", projection);
		modelShader_noneTexture.setMat4("view", view);
		//��ռ��ӽǱ任����
		glUniformMatrix4fv(glGetUniformLocation(modelShader_noneTexture.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::translate(model, castlePos);		   // site
		//model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));		   // scale
		modelShader_noneTexture.setMat4("model", model);

		glUniform1i(shadowLoc_none, 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		Model_castle.Draw(modelShader_noneTexture);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	deleteOcean();
	glfwTerminate();
	return 0;
}

void initGL()
{
	// glfw: initialize and configure
	 // ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	int monitorCount = 0;
	GLFWmonitor** pMonitor = glfwGetMonitors(&monitorCount);
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);
	SCR_WIDTH = w;
	SCR_HEIGHT = h;

	int holographic_screen = -1;
	for (int i = 0; i < monitorCount; i++) {
		GLFWvidmode* mode = (GLFWvidmode*)glfwGetVideoMode(pMonitor[i]);
		if (mode->width == w && h == mode->height) {
			holographic_screen = i;
			window = glfwCreateWindow(w, h, "", pMonitor[holographic_screen], NULL);
		}
	}
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
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
		return;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
}


// �����ͼ��Ⱦ����Ļ���ı�����
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

//��Ⱦ���������ͼʱ ʹ��
void renderScene(Shader& shader,  Model& Model_stage, Model& Model_castle,  Model& dancer)
{
	glm::mat4 model = glm::mat4(1.0f);
	// -------------------------------- MODEL dancer --------------------------------
	model = glm::translate(model, glm::vec3(22.7f, 1.2f, -22.7f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f)); // translate it down so it's at the center of the scene
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.f));
	shader.setMat4("model", model);
	dancer.Draw(shader);

	// -------------------------------- MODEL stage --------------------------------
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
	model = glm::translate(model, stagePos);			// site
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));		// scale
	shader.setMat4("model", model);
	Model_stage.Draw(shader);
	// -------------------------------- MODEL castle --------------------------------
	// render the loaded model
	model = glm::mat4(1.0f);
	model = glm::translate(model, castlePos);		   // site
	//model = glm::rotate(model, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));		   // scale
	shader.setMat4("model", model);
	Model_castle.Draw(shader);

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