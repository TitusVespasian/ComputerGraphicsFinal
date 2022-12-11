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
#include<fstream>

#include <irrklang/irrKlang.h>


/********************************************* 函数声明 *********************************************/
void initGL();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void renderScene(Shader& shader, Shader& modelShader, Model& Model_island, Model& Model_stage, Model& Model_castle, Model& dancer);
void RenderQuad();
void RenderStart();
unsigned int loadTexture_(char const* path);

/********************************************* 全局变量/宏定义 *********************************************/

//window
GLFWwindow* window;

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(-9.0f, 3.0f, 10.0f));
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
const float st[] = { 0.0f ,180.0f ,360.0f,540.0f,690.0f };
const float s0 = 0.0f;
const float s1 = 180.0f;
const float s2 = 360.0f;
const float s3 = 540.0f;
const float s4 = 690.0f;

//animation end time
const float et[] = { 141.0f,315.0f,509.0f,665.0f,804.0f };
const float e0 = 141.0f;
const float e1 = 315.0f;
const float e2 = 509.0f;
const float e3 = 665.0f;
const float e4 = 804.0f;

//动作提示的宏定义
int need_pose=0;//提示当前需要做的动作
const int up_pose = 1;
const int down_pose = 2;
const int left_pose = 3;
const int right_pose = 4;
const float delta_pose_time = 5.0f;//每个动作提示相差的秒数
float start_pose_time = 12.0f;//下一个动作的开始时间;
float end_pose_time = 13.0f;//下一个动作的开始时间;


//model position
const glm::vec3 castlePos = glm::vec3(0.0f, -4.3f, -0.0f);
const glm::vec3 islandPos = glm::vec3(15.0f, 0.0f, 15.0f);
const glm::vec3 smallIslandPos = glm::vec3(-25.0f, 1.0f, -15.0f);
const glm::vec3 stagePos = glm::vec3(25.0f, 1.6f, -5.0f);
const glm::vec3 ballonPos = glm::vec3(3.0f, 2.5f, 3.0f);
const glm::vec3 paperPos = glm::vec3(3.0f, 2.5f, 3.0f);
const glm::vec3 circlePos = glm::vec3(7.2f, 0.11f, -6.0f);
const glm::vec3 arrowPos = glm::vec3(3.0f, 4.5f, 3.0f);

const glm::vec3 dirLightDirection = glm::vec3(2.0f, -3.0f, 0.0f);
const glm::vec3 dirLightPos = glm::vec3(-5.0f, 7.0f, 8.0f);

//屏幕四边形的VA0\VBO(用到)
GLuint quadVAO = 0;
GLuint quadVBO;
GLuint startVBO, startVAO, startEBO;
// ocean
extern void initProgram();
extern void initWater();
extern void renderWater();
extern void deleteOcean();

//game mode
const int MODE = 1; //0:keyboard 1:readfile(CV)

// aanimation time last
const float a_time[] = { 4.74,4.5,4.97,4.17,3.8 };

struct movement {
	int id;
	float end_time;
};

// music player
irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();

int main()
{
	initGL();

	//water
	initProgram();
	initWater();

	// build and compile shaders
	// -------------------------
	// "shaders/model/anim_model.vs", "shaders/model/anim_model.fs"
	//"shaders/scene/model_loading.vs", "shaders/scene/model_loading_withTexture.fs"
	Shader ourShader("shaders/model/anim_model.vs", "shaders/model/anim_model.fs");

	// load models
	// -----------
	Model ourModel("resources/objects/dance/3.fbx");
	Animation danceAnimation("resources/objects/dance/3.fbx", &ourModel);
	Animator animator(&danceAnimation);//start of the animator

	//scene
	//-----------
	Model Model_castle("resources/sceneMaterial/cartoonCastle/Cartoon castle.obj", true);
	Model Model_stage("resources/sceneMaterial/Stage/stage.obj", true);
	Model Model_island("resources/sceneMaterial/Small Tropical Island/Small Tropical Island.obj", true);
	Model Model_ballon("resources/sceneMaterial/Ballon/ballon.obj", true);
	Model Model_paper("resources/sceneMaterial/Paper/paper.obj", true);
	Model Model_circle("resources/sceneMaterial/Circle/circle.obj", true);
	Model Model_arrow_up("resources/sceneMaterial/Arrow/arrow_up.obj", true);
	Model Model_arrow_down("resources/sceneMaterial/Arrow/arrow_down.obj", true);
	Model Model_arrow_right("resources/sceneMaterial/Arrow/arrow_right.obj", true);
	Model Model_arrow_left("resources/sceneMaterial/Arrow/arrow_left.obj", true);
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
	Shader modelDepthShader("shaders/model/dirShadowModel.vs", "shaders/model/dirShadowModel.fs");
	Shader testShader("shaders/scene/shadowTest.vs", "shaders/scene/shadowTest.fs");

	//start
	Shader startShader("shaders/front/start.vs", "shaders/front/start.fs");
	Shader fadingShader("shaders/front/fading.vs", "shaders/front/fading.fs");

	//animation id
	int act = a0;
	int tip = a0;

	//beat read
	ifstream fin;
	fin.open("resources/sound/act.bt");
	if (!fin.is_open()) {
		std::cout << "Could not open beat file" << endl;
		return -1;
	}

	ifstream fin_tip;
	fin_tip.open("resources/sound/beat.bt");
	if (!fin_tip.is_open()) {
		std::cout << "Could not open beat file" << endl;
		return -1;
	}

	// texture
	// ------
	unsigned int TextureS = loadTexture_("resources/start/start.png");

	//**************SHADOW TEST************//
	/*创建阴影贴图*/
	unsigned int depthMapFBO, depthMap;
	glGenFramebuffers(1, &depthMapFBO); //生成一个帧缓冲对象
	const GLuint SHADOW_WIDTH = SCR_WIDTH*2, SHADOW_HEIGHT = SCR_HEIGHT*2;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);//绑定深度图
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//生成深度图
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// 把生成的深度纹理作为帧缓冲的深度缓冲
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);//激活深度图的帧缓冲
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	// 设置不用任何颜色
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		cout << "somthing wrong with frame buffer!";
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	

	/*****************入场动画渲染************************/

	RenderStart();
	glEnable(GL_DEPTH_TEST);
	SoundEngine->play2D("resources/sound/front.mp3", GL_FALSE);
	double t_start = glfwGetTime();
	bool is_end = false;
	double t_end = 0;
	bool not_set = true;
	while (!glfwWindowShouldClose(window))
	{

		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		startShader.use();

		double t_last = glfwGetTime() - t_start;
		if (not_set && is_end) {
			t_end = t_last;
			not_set = false;
		}
		startShader.setFloat("Time", (float)t_last);
		startShader.setFloat("ETime", (float)(t_last - t_end));
		startShader.setBool("End", is_end);
		glBindVertexArray(startVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		if (!is_end && t_last >= 13) {
			is_end = true;
		}
		if (t_last >= 19)
			break;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	t_start = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{

		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		fadingShader.use();


		double t_last = glfwGetTime() - t_start;

		if (t_last > 6)
			break;
		glBindTexture(GL_TEXTURE_2D, TextureS);

		fadingShader.setFloat("Time", (float)t_last+6.0f);
		glBindVertexArray(startVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();

	}



	// 时间管理和动作管理
	movement cur_dancer, cur_tip;
	float start_time = (float)glfwGetTime();
	float mark_time = start_time + 7.50f;
	float tip_time = mark_time - 1;

	cur_dancer.id = 0;
	cur_dancer.end_time = mark_time + 5;

	cur_tip.id = 0;
	cur_tip.end_time = tip_time + 1;
	int key = -1;
	SoundEngine->play2D("resources/sound/play.mp3", GL_FALSE);
	bool end = false;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		// music control
		if (end)
			;
		else if (lastFrame - start_time >= 85) {
			end = true;
			SoundEngine->play2D("resources/sound/applause.mp3", GL_FALSE);
		}
		else if (lastFrame - start_time >= 82.5) {
			act = -1;
			animator.setCurrentTime(et[1]);//站立
		}
		else {
			if (lastFrame - mark_time >= 5) {	// 动作读取
				if (MODE == 1) {
					act = fin.get() - '0';//从文件中读取
					cur_dancer.id = act;
					cur_dancer.end_time = lastFrame + a_time[act];//标记结束时间
					animator.setCurrentTime(st[act]);//设置模型
					mark_time = lastFrame;//记录时间
				}
				else if (key!=-1 && MODE == 0) {
					act = key;	//从键盘读取
					cur_dancer.id = act;
					cur_dancer.end_time = lastFrame + a_time[act];//标记结束时间
					animator.setCurrentTime(st[act]);//设置模型
					mark_time = lastFrame;//记录时间
				}
					
			}

			if (lastFrame - tip_time >= 5) {
				tip = fin_tip.get() - '0';
				cur_tip.id = tip;
				cur_tip.end_time = lastFrame + 1;
				tip_time = lastFrame;//记录时间
				key = -1;
			}
		}
		

		if (lastFrame >= cur_dancer.end_time) {
			act = -1;
			animator.setCurrentTime(et[1]);//站立
		}

		// input
		// -----
		processInput(window);
		// render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//START TO RENDER EVERYTHING
		/* STEP1---渲染深度贴图 */
		//光源空间的变换
		GLfloat near_plane = 0.1f, far_plane = 30.0f;
		glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(dirLightPos, dirLightPos + dirLightDirection, glm::vec3(0.0, 1.0, 0.0));//camera.Position
		//glm::vec3 delt = glm::vec3(0.0, 5.0, 0.0);
		//glm::mat4 lightView = glm::lookAt(camera.Position + delt, camera.Position + delt + dirLightDirection, glm::vec3(0.0, 1.0, 0.0));//camera.Position
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		DepthShader.use();
		GLint lightSpaceMatrixLocation = glGetUniformLocation(DepthShader.ID, "lightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		modelDepthShader.use();
		lightSpaceMatrixLocation = glGetUniformLocation(modelDepthShader.ID, "lightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderScene(DepthShader, modelDepthShader, Model_island, Model_stage, Model_castle, ourModel);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/* STEP2---渲染depthMap到场景 可视化 */
		// Reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//to debug only，RenderQuad显示当前的深度图
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
		//动画切换判断
		if (MODE == 0) {
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			//animator.setCurrentTime(s1);
			//act = a1;
			if (key < 0)
				key = a1;
			
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			//animator.setCurrentTime(s2);
			//act = a2;
			if (key < 0)
				key = a2;
			
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			//animator.setCurrentTime(s3);
			//act = a3;
			if (key < 0)
				key = a3;
			
		}
		else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			//animator.setCurrentTime(s4);
			//act = a4;
			if (key < 0)
				key = a4;
			}
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
		{
			ourShader.use();
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
			modelDepthShader.use();
			modelDepthShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
		}
		
		ourShader.use();
		//人物的shader参数设置
		ourShader.setVec3("dirLight.direction", dirLightDirection);
		ourShader.setVec3("dirLight.ambient", 0.8f, 0.8f, 0.8f);
		ourShader.setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
		ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		ourShader.setVec3("eyePos", camera.Position);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-25.0f, 0.0f, 25.0f));
		model = glm::translate(model, glm::vec3(22.7f, 1.2f, -22.7f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down
		model = glm::translate(model, glm::vec3(-300.0f, -10.0f, 400.5f)); // translate it down so it's at the center of the scene
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
		GLint dirLightDirectionLoc = glGetUniformLocation(modelShader_withTexture.ID, "dirLight.direction");
		GLint isPointLightLoc = glGetUniformLocation(modelShader_withTexture.ID, "is_pointlight");
		GLuint shadowLoc = glGetUniformLocation(modelShader_withTexture.ID, "shadowMap");
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
		GLint isPointLightLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "is_pointlight");
		GLuint shadowLoc_none = glGetUniformLocation(modelShader_noneTexture.ID, "shadowMap");
		// 设置观察者位置
		GLint viewPosLoc = glGetUniformLocation(modelShader_withTexture.ID, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		model = glm::mat4(1.0f);

		// -------------------------------- MODEL island --------------------------------
		modelShader_withTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc, 0.3f, 0.3f, 0.3f);
		glUniform3f(dirLightDiffuseLoc, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 landLightPos = glm::vec3(islandPos.x, islandPos.y + 5, islandPos.z);
		glUniform1i(isPointLightLoc, true);//点光源
		glUniform3f(pointLightAmbientLoc, 0.7f, 0.7f, 0.8f);
		glUniform3f(pointLightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc, landLightPos.x, landLightPos.y, landLightPos.z);
		// 设置衰减系数
		glUniform1f(attConstant, 1.0f);
		glUniform1f(attLinear, 0.09f);
		glUniform1f(attQuadratic, 0.032f);
		// 亮度
		glUniform1f(shininess, 64.0f);
		//光空间视角变换矩阵
		glUniformMatrix4fv(glGetUniformLocation(modelShader_withTexture.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		// view/projection transformations
		modelShader_withTexture.setMat4("projection", projection);
		modelShader_withTexture.setMat4("view", view);
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::translate(model, islandPos);			// site

		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));		// scale
		modelShader_withTexture.setMat4("model", model);
		glUniform1i(shadowLoc, 9);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		Model_island.Draw(modelShader_withTexture);

		// -------------------------------- MODEL stage --------------------------------
		modelShader_withTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDiffuseLoc, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 stageLightPos = glm::vec3(stagePos.x, stagePos.y + 5, stagePos.z);
		glUniform1i(isPointLightLoc, true);//点光源
		glUniform3f(pointLightAmbientLoc, 0.9f, 0.3f, 0.9f);
		glUniform3f(pointLightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc, stageLightPos.x, stageLightPos.y, stageLightPos.z);
		// 设置衰减系数
		glUniform1f(attConstant, 1.0f);
		glUniform1f(attLinear, 0.09f);
		glUniform1f(attQuadratic, 0.32f);
		// 亮度
		glUniform1f(shininess, 64.0f);
		// view/projection transformations
		modelShader_withTexture.setMat4("projection", projection);
		modelShader_withTexture.setMat4("view", view);
		//光空间视角变换矩阵
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
		//Model_stage.Draw(modelShader_withTexture);
		// -------------------------------- MODEL ballon --------------------------------
		modelShader_withTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDiffuseLoc, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 ballonLightPos = glm::vec3(ballonPos.x, ballonPos.y + 5, ballonPos.z);
		glUniform1i(isPointLightLoc, true);//点光源
		glUniform3f(pointLightAmbientLoc, 0.9f, 0.3f, 0.9f);
		glUniform3f(pointLightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc, ballonLightPos.x, ballonLightPos.y, ballonLightPos.z);
		// 设置衰减系数
		glUniform1f(attConstant, 1.0f);
		glUniform1f(attLinear, 0.09f);
		glUniform1f(attQuadratic, 0.32f);
		// 亮度
		glUniform1f(shininess, 64.0f);
		// view/projection transformations
		modelShader_withTexture.setMat4("projection", projection);
		modelShader_withTexture.setMat4("view", view);
		//光空间视角变换矩阵
		glUniformMatrix4fv(glGetUniformLocation(modelShader_withTexture.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(160.0f), glm::vec3(0.0, 1.0, 0.0));
		model = glm::translate(model, ballonPos);			// site
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));		// scale
		modelShader_withTexture.setMat4("model", model);
		//glUniform1i(shadowLoc, 9);
		//glActiveTexture(GL_TEXTURE9);
		//glBindTexture(GL_TEXTURE_2D, depthMap);
		Model_ballon.Draw(modelShader_withTexture);
		// -------------------------------- MODEL paper --------------------------------
		modelShader_withTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDiffuseLoc, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 paperLightPos = glm::vec3(paperPos.x, paperPos.y + 5, paperPos.z);
		glUniform1i(isPointLightLoc, true);//点光源
		glUniform3f(pointLightAmbientLoc, 0.9f, 0.3f, 0.9f);
		glUniform3f(pointLightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc, paperLightPos.x, paperLightPos.y, paperLightPos.z);
		// 设置衰减系数
		glUniform1f(attConstant, 1.0f);
		glUniform1f(attLinear, 0.09f);
		glUniform1f(attQuadratic, 0.32f);
		// 亮度
		glUniform1f(shininess, 64.0f);
		// view/projection transformations
		modelShader_withTexture.setMat4("projection", projection);
		modelShader_withTexture.setMat4("view", view);
		//光空间视角变换矩阵
		glUniformMatrix4fv(glGetUniformLocation(modelShader_withTexture.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::translate(model, paperPos);			// site
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));		// scale
		modelShader_withTexture.setMat4("model", model);
		glUniform1i(shadowLoc, 9);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		//Model_paper.Draw(modelShader_withTexture);
		// -------------------------------- MODEL castle --------------------------------
		modelShader_noneTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc_none, 0.3f, 0.3f, 0.3f);
		glUniform3f(dirLightDiffuseLoc_none, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc_none, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 castleLightPos = glm::vec3(castlePos.x, castlePos.y + 6, castlePos.z);
		glUniform1i(isPointLightLoc_none, true);//点光源
		glUniform3f(pointLightAmbientLoc_none, 0.95f, 0.8f, 0.85f);
		glUniform3f(pointLightDiffuseLoc_none, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc_none, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc_none, castleLightPos.x, castleLightPos.y, castleLightPos.z);
		// 设置衰减系数
		glUniform1f(attConstant_none, 1.0f);
		glUniform1f(attLinear_none, 0.29f);
		glUniform1f(attQuadratic_none, 0.32f);
		// 亮度
		glUniform1f(shininess_none, 64.0f);
		// view/projection transformations
		modelShader_noneTexture.setMat4("projection", projection);
		modelShader_noneTexture.setMat4("view", view);
		//光空间视角变换矩阵
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
		// -------------------------------- MODEL circle --------------------------------
		modelShader_noneTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(dirLightDiffuseLoc, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 circleLightPos = glm::vec3(circlePos.x, circlePos.y + 5, circlePos.z);
		glUniform1i(isPointLightLoc, true);//点光源
		glUniform3f(pointLightAmbientLoc, 0.9f, 0.3f, 0.9f);
		glUniform3f(pointLightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc, circleLightPos.x, circleLightPos.y, circleLightPos.z);
		// 设置衰减系数
		glUniform1f(attConstant, 1.0f);
		glUniform1f(attLinear, 0.09f);
		glUniform1f(attQuadratic, 0.32f);
		// 亮度
		glUniform1f(shininess, 64.0f);
		// view/projection transformations
		modelShader_noneTexture.setMat4("projection", projection);
		modelShader_noneTexture.setMat4("view", view);
		//光空间视角变换矩阵
		glUniformMatrix4fv(glGetUniformLocation(modelShader_noneTexture.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		// render the loaded model
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
		model = glm::translate(model, circlePos);			// site
		model = glm::scale(model, glm::vec3(0.11f, 0.11f, 0.11f));		// scale
		modelShader_noneTexture.setMat4("model", model);
		glUniform1i(shadowLoc, 9);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		Model_circle.Draw(modelShader_noneTexture);
		// -------------------------------- MODEL arrow --------------------------------
		modelShader_noneTexture.use();
		// 设置光源属性 平行光源
		glUniform3f(dirLightAmbientLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(dirLightDiffuseLoc, 0.8f, 0.8f, 0.8f);
		glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(dirLightDirectionLoc, dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
		// 设置光源属性 点光源
		glm::vec3 arrowLightPos = glm::vec3(arrowPos.x, arrowPos.y + 5, arrowPos.z);
		glUniform1i(isPointLightLoc, true);//点光源
		glUniform3f(pointLightAmbientLoc, 0.9f, 0.3f, 0.9f);
		glUniform3f(pointLightDiffuseLoc, 0.5f, 0.5f, 0.5f);
		glUniform3f(pointLightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(pointLightPosLoc, arrowLightPos.x, arrowLightPos.y, arrowLightPos.z);
		// 设置衰减系数
		glUniform1f(attConstant, 1.0f);
		glUniform1f(attLinear, 0.09f);
		glUniform1f(attQuadratic, 0.32f);
		// 亮度
		glUniform1f(shininess, 64.0f);
		// view/projection transformations
		modelShader_noneTexture.setMat4("projection", projection);
		modelShader_noneTexture.setMat4("view", view);
		//光空间视角变换矩阵
		glUniformMatrix4fv(glGetUniformLocation(modelShader_noneTexture.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		// render the loaded model
		model = glm::mat4(1.0f);
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::translate(model, arrowPos);			// site
		//model = glm::rotate(model, glm::radians(20.0f), glm::vec3(arrowPos.x, 0.0, arrowPos.z));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));		// scale
		modelShader_noneTexture.setMat4("model", model);
		glUniform1i(shadowLoc, 9);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		if (currentFrame <= cur_tip.end_time)
		{
			if (cur_tip.id == up_pose)
				Model_arrow_up.Draw(modelShader_noneTexture);
			else if (cur_tip.id == down_pose)
				Model_arrow_down.Draw(modelShader_noneTexture);
			else if (cur_tip.id == left_pose)
				Model_arrow_left.Draw(modelShader_noneTexture);
			else if (cur_tip.id == right_pose)
				Model_arrow_right.Draw(modelShader_noneTexture);
		}
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		//static bool flag = false;
		//if(!flag)
		//	save_img(window);
		//flag = true;
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


// 将深度图渲染到屏幕的四边形上
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

void RenderStart()
{
	float startVertices[] = {
		 1.0f,  1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,  0.0f,
		-1.0f, -1.0f,  0.0f,  0.0f,
		-1.0f,  1.0f,  0.0f,  1.0f
	};

	unsigned int startIndices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, &startVAO);
	glGenBuffers(1, &startVBO);
	glGenBuffers(1, &startEBO);

	glBindVertexArray(startVAO);

	glBindBuffer(GL_ARRAY_BUFFER, startVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(startVertices), startVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, startEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(startIndices), startIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void renderScene(Shader& shader, Shader& modelShader, Model& Model_island, Model& Model_stage, Model& Model_castle, Model& dancer)
{
	glm::mat4 model = glm::mat4(1.0f);
	// -------------------------------- MODEL dancer --------------------------------
	model = glm::translate(model, glm::vec3(-25.0f, 0.0f, 25.0f));
	model = glm::translate(model, glm::vec3(22.7f, 1.2f, -22.7f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down
	model = glm::translate(model, glm::vec3(-300.0f, -10.0f, 400.5f)); // translate it down so it's at the center of the scene
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.f));
	modelShader.use();
	modelShader.setMat4("model", model);
	dancer.Draw(shader);
	// -------------------------------- MODEL island --------------------------------
	model = glm::mat4(1.0f);
	model = glm::translate(model, islandPos);			// site
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));		// scale
	shader.use();
	shader.setMat4("model", model);
	Model_island.Draw(shader);
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

unsigned int loadTexture_(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = GL_RGBA;
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