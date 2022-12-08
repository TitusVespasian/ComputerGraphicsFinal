#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "water/Program_w.hpp"
#include <general/camera.h>

#include <iostream>
#include <stdexcept>
#include <cmath>
#include <climits>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

//window
GLFWwindow* window;

// settings
long SCR_WIDTH = 800;
long SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 2.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//water
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
void initGL();
void renderWater();

int main()
{
    initGL();
    initProgram();
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

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        processInput(window);
        // render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view/projection transformations
        renderWater();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(TEXTURES_AMOUNT, heightMap);
    glDeleteTextures(TEXTURES_AMOUNT, normalMap);
    glDeleteTextures(1, &waterTex);
    glDeleteTextures(1, &wavesHeightMap);
    glDeleteTextures(1, &wavesNormalMap);
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
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
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
void initGL() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_SAMPLES, SAMPLES);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
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

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
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
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
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
    offset += 0.02 * deltaTime;
    program_water.setFloat("wavesOffset", offset);

    glBindVertexArray(VAO);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
