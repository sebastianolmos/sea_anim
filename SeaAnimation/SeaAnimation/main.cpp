#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "shader/shader.h"
#include "util/performanceMonitor.h"
#include "util/camera3d.h"
#include "util/model.h"
#include "util/shipMovement.h"
#include <glm/gtx/norm.hpp >

#include "menu.h"

#include <iostream>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, bool* fill);
void createSeaMesh(float*& vertices, unsigned int*& indices, unsigned int N, glm::vec3 initPos, glm::vec3 finalPos, float sizeUV);
glm::vec3 GetSkyColor(float cenit);
unsigned int loadTexture(string path, GLuint mode);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera3D camera(glm::vec3(0.0f, 0.0f, 0.0f));
ShipMovement shipMovement(glm::vec3(0.0f, 0.0f, 0.0f));

Menu guiMenu = Menu();

// timing
float deltaTime = 0.0f;	// time between current frame and last frame

bool globaLView = true;
string viewName;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    string title = "Sea animation";
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, title.c_str(), NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader shipShader("shader/shipShader.vs", "shader/shipShader.fs"); // you can name your shader files however you like
    Model shipModel("../assets/viking_ship/ship.obj");

    // Shader para el mar
    Shader seaShader("shader/seaShader.vs", "shader/seaShader.fs");

    float* seaVertices;
    unsigned int* seaIndices;
    unsigned int seaSize = 512;
    createSeaMesh(seaVertices, seaIndices, seaSize, glm::vec3(-32.0f, -32.0f, 0.0f), glm::vec3(32.0f, 32.0f, 0.0f), 1.0f);
    unsigned int seaVBO, seaVAO, seaEBO;
    glGenVertexArrays(1, &seaVAO);
    glGenBuffers(1, &seaVBO);
    glGenBuffers(1, &seaEBO);

    glBindVertexArray(seaVAO);

    glBindBuffer(GL_ARRAY_BUFFER, seaVBO);
    glBufferData(GL_ARRAY_BUFFER, (seaSize * seaSize) * 5 * sizeof(float), seaVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, seaEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (seaSize - 1) * (seaSize - 1) * 2 * 3 * sizeof(unsigned int), seaIndices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load and create a texture 
    // -------------------------
    unsigned int texture1 = loadTexture("../assets/water2.png", GL_RGBA);
    unsigned int texture2 = loadTexture("../assets/displacement1.jpg", GL_RGB);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    seaShader.use(); // don't forget to activate/use the shader before setting uniforms!
    seaShader.setInt("texture_tmp", 0);
    seaShader.setInt("texture_dist", 1);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader sunShader("shader/MVPTexShader.vs", "shader/MVPTexShader.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float sunVertices[] = {
        // positions        // texture coords
         -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, // top right
         -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom right
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // bottom left
        0.5f,  0.5f, 0.0f, 1.0f, 0.0f  // top left 
    };
    unsigned int sunIndices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int sunVBO, sunVAO, sunEBO;
    glGenVertexArrays(1, &sunVAO);
    glGenBuffers(1, &sunVBO);
    glGenBuffers(1, &sunEBO);

    glBindVertexArray(sunVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sunVertices), sunVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sunIndices), sunIndices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load and create a texture 
    // -------------------------
    unsigned int texture3 = loadTexture("../assets/sun2.png", GL_RGBA);

    sunShader.use(); // don't forget to activate/use the shader before setting uniforms!
    sunShader.setInt("texture1", 0);

    // Enabling transparencies
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    guiMenu.init(window);

    uint32_t mFBO = 0;
    uint32_t mTexId = 0;
    uint32_t mDepthId = 0;
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glCreateTextures(GL_TEXTURE_2D, 1, &mTexId);
    glBindTexture(GL_TEXTURE_2D, mTexId);

    int32_t mWidth = 800;
    int32_t mHeight = 800;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexId, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &mDepthId);
    glBindTexture(GL_TEXTURE_2D, mDepthId);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, mWidth, mHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthId, 0);

    GLenum buffers[4] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(mTexId, buffers);

    // unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    PerformanceMonitor pMonitor(glfwGetTime(), 0.5f);

    glm::vec2 mSize = { 800, 800 };
    bool fillPolygon = true;

    float sun_cenit = 35.749f;
    float sun_azim = 34.596f;
    float gravity = 9.8f;
    //glm::vec2 wave_direction = glm::vec2(1.0f);
    glm::vec4 wave_A = glm::vec4(-0.072f, 1.0f, 0.346f, 20.0f);
    glm::vec4 wave_B = glm::vec4(0.855f, -0.536f, 0.417f, 12.814f);
    glm::vec4 wave_C = glm::vec4(0.449f, 0.362f, 0.712f, 19.026f);

    float light_ambient = 0.115f;
    float light_diffuse = 0.833f;
    float light_specular = 0.756f;

    glm::vec3 water_color = glm::vec3(0.279f, 0.528f, 1.0f);
    glm::vec3 water_ambient = glm::vec3(0.0f, 0.006f, 0.359f);
    glm::vec3 water_diffuse = glm::vec3(0.05f, 0.165f, 0.65f);
    glm::vec3 water_specular = glm::vec3(0.491f, 0.492f, 0.492f);
    float water_shininess = 67.0f;

    displace disA = displace();
    disA.size = 1.531f;
    disA.direction = glm::vec2(1.0f);
    disA.speed = 0.015f;
    disA.strenght = 3.429f;
    disA.color = glm::vec3(0.358f, 0.358f, 0.358f);
    disA.discard = glm::vec2(0.269f, 0.333f);
    disA.inside = true;

    displace disB = displace();
    disB.size = 2.963f;
    disB.direction = glm::vec2(-0.478f, 0.507f);
    disB.speed = 0.021f;
    disB.strenght = 1.282f;
    disB.color = glm::vec3(0.485f, 0.485f, 0.485f);
    disB.discard = glm::vec2(0.186f, 0.269f);
    disB.inside = false;

    displace disC = displace();
    disC.size = 4.599f;
    disC.direction = glm::vec2(-1.0f, 1.0f);
    disC.speed = 0.044f;
    disC.strenght = 2.179f;
    disC.color = glm::vec3(0.200f, 0.200f, 0.200f);
    disC.discard = glm::vec2(0.071f, 0.353f);
    disC.inside = false;

    float ship_size = 1.0f;
    glm::vec3 ship_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 ship_tangent = glm::vec3(1.0f);
    glm::vec3 ship_binormal = glm::vec3(1.0f);
    glm::vec3 ship_normal = glm::vec3(1.0f);
    float ship_rotation = 34.1f;

    glm::vec3 sky_color = glm::vec3(1.0f);

    float t0 = glfwGetTime();
    float t1 = t0;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        t0 = glfwGetTime();
        deltaTime = t1 - t0;
        t1 = t0;

        pMonitor.update(t1);
        stringstream ss;
        ss << title << " " << pMonitor;
        glfwSetWindowTitle(window, ss.str().c_str());

        // input
        // -----
        processInput(window, &fillPolygon);

        // prerender openglcontext
        // ------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        sky_color = GetSkyColor(sun_cenit);
        glClearColor(sky_color.x, sky_color.y, sky_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        guiMenu.preRender();

        // render the triangle


        // bind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glViewport(0, 0, mWidth, mHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (fillPolygon)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


        float theta = glm::radians(sun_cenit);
        float phi = glm::radians(sun_azim);
        glm::vec3 lightDirection = glm::vec3(
            -glm::sin(theta) * glm::cos(phi),
            -glm::sin(theta) * glm::sin(phi),
            -glm::cos(theta));

        glm::vec3 p = ship_pos;
        p += shipMovement.GerstnerWave(wave_A, ship_pos, ship_tangent, ship_binormal, gravity, t1);
        p += shipMovement.GerstnerWave(wave_B, ship_pos, ship_tangent, ship_binormal, gravity, t1);
        p += shipMovement.GerstnerWave(wave_C, ship_pos, ship_tangent, ship_binormal, gravity, t1);
        ship_normal = glm::normalize(cross(ship_tangent, ship_binormal));
        glm::mat4 rotate = glm::mat4_cast(shipMovement.RotationBetweenVectors(
            glm::vec3(ship_tangent.x, ship_tangent.y, 0.0f),
            glm::vec3(ship_tangent.x, ship_tangent.y, ship_tangent.z / 4.0f)
        ));
        if (!globaLView)
        {
            shipMovement.setPos(p);
            shipMovement.setTransform(rotate * glm::rotate(glm::mat4(1.0f), glm::radians(ship_rotation), glm::vec3(0.0f, 0.0f, 1.0f)));
        }

        shipShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(globaLView ? camera.Fovy : shipMovement.Fovy), (float)mSize.x / (float)mSize.y, 0.1f, 100.0f);
        glm::mat4 view = globaLView ? camera.GetViewMatrix() : shipMovement.GetViewMatrix();
        shipShader.setMat4("projection", projection);
        shipShader.setMat4("view", view);

        glm::vec3 N = glm::vec3(ship_tangent.x, ship_tangent.y, 0.0f);
        //glm::vec3 RotationAxis = glm::cross(N, glm::vec3(dir_gl.x, dir_gl.y, 0));
        float RotationAngle = glm::acos(glm::dot(N, ship_tangent));
        if (RotationAngle == 0.0f) {
            RotationAngle = 0.001f;
        }
        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p);

        model = model * rotate;
        model = glm::rotate(model, glm::radians(ship_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, 0.1f * glm::vec3(ship_size, ship_size, ship_size));

        shipShader.setMat4("model", model);
        shipShader.setVec3("lightPos", -lightDirection);
        shipModel.Draw(shipShader);

        // Draw the sea
        seaShader.use();
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        seaShader.setVec3("light.direction", lightDirection);
        seaShader.setVec3("viewPos", camera.Position);

        // light properties
        glm::vec3 lightColor = glm::vec3(1.0f);
        seaShader.setVec3("light.ambient", lightColor * light_ambient);
        seaShader.setVec3("light.diffuse", lightColor * light_diffuse);
        seaShader.setVec3("light.specular", lightColor * light_specular);

        // material properties
        seaShader.setVec3("material.color", water_color);
        seaShader.setVec3("material.ambient", water_ambient);
        seaShader.setVec3("material.diffuse", water_diffuse);
        seaShader.setVec3("material.specular", water_specular); // specular lighting doesn't have full effect on this object's material
        seaShader.setFloat("material.shininess", water_shininess);

        // view/projection transformations
        seaShader.setMat4("projection", projection);
        seaShader.setMat4("view", view);

        // world transformation
        glm::mat4 seaModel = glm::mat4(1.0f);
        seaShader.setMat4("model", seaModel);

        //wave properties
        seaShader.setFloat("gravity", gravity);
        seaShader.setFloat("time", t1);
        seaShader.setVec4("waveA", wave_A);
        seaShader.setVec4("waveB", wave_B);
        seaShader.setVec4("waveC", wave_C);

        seaShader.setFloat("disASize", disA.size);
        seaShader.setVec2("disADir", disA.direction);
        seaShader.setFloat("disASpeed", disA.speed);
        seaShader.setFloat("disAStrenght", disA.strenght);
        seaShader.setVec3("disAColor", disA.color);
        seaShader.setVec2("disADiscard", disA.discard);
        seaShader.setBool("disAInside", disA.inside);

        seaShader.setFloat("disBSize", disB.size);
        seaShader.setVec2("disBDir", disB.direction);
        seaShader.setFloat("disBSpeed", disB.speed);
        seaShader.setFloat("disBStrenght", disB.strenght);
        seaShader.setVec3("disBColor", disB.color);
        seaShader.setVec2("disBDiscard", disB.discard);
        seaShader.setBool("disBInside", disB.inside);

        seaShader.setFloat("disCSize", disC.size);
        seaShader.setVec2("disCDir", disC.direction);
        seaShader.setFloat("disCSpeed", disC.speed);
        seaShader.setFloat("disCStrenght", disC.strenght);
        seaShader.setVec3("disCColor", disC.color);
        seaShader.setVec2("disCDiscard", disC.discard);
        seaShader.setBool("disCInside", disC.inside);

        // render the sea
        glBindVertexArray(seaVAO);
        glDrawElements(GL_TRIANGLES, (seaSize - 1) * (seaSize - 1) * 2 * 3, GL_UNSIGNED_INT, 0);

        // Render the sun
        // activate shader
        sunShader.use();
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture3);


        // pass projection matrix to shader (note that in this case it could change every frame)
        sunShader.setMat4("projection", projection);

        // camera/view transformation
        sunShader.setMat4("view", view);

        // render box
        glBindVertexArray(sunVAO);
        // calculate the model matrix for each object and pass it to shader before drawing
        glm::mat4 sunModel = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        //sunModel = glm::scale(sunModel, glm::vec3(10.0f, 10.0f, 10.0f));
        sunShader.setMat4("model", sunModel);
        sunShader.setVec3("pos", -lightDirection * 40.0f);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // unbind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        mSize = guiMenu.begin(mTexId);
        guiMenu.setShip(&ship_pos, &ship_size, &ship_rotation);

        guiMenu.setWaterMaterial(&water_color, &water_ambient, &water_diffuse, &water_specular, &water_shininess);

        guiMenu.setWaves(&gravity, &wave_A, &wave_B, &wave_C);

        guiMenu.setTextures(&disA, &disB, &disC);

        guiMenu.setLight(&sun_cenit, &sun_azim, &light_ambient, &light_diffuse, &light_specular);

        guiMenu.setView(&globaLView, &ship_pos);

        guiMenu.config1(&gravity, &wave_A, &wave_B, &wave_C, &water_color, &water_ambient, &water_diffuse, &water_specular, &water_shininess, &disA, &disB, &disC, &sun_cenit, &sun_azim,
            &light_ambient, &light_diffuse, &light_specular);
        guiMenu.config2(&gravity, &wave_A, &wave_B, &wave_C, &water_color, &water_ambient, &water_diffuse, &water_specular, &water_shininess, &disA, &disB, &disC, &sun_cenit, &sun_azim,
            &light_ambient, &light_diffuse, &light_specular);
        guiMenu.config3(&gravity, &wave_A, &wave_B, &wave_C, &water_color, &water_ambient, &water_diffuse, &water_specular, &water_shininess, &disA, &disB, &disC, &sun_cenit, &sun_azim,
            &light_ambient, &light_diffuse, &light_specular);
        guiMenu.config4(&gravity, &wave_A, &wave_B, &wave_C, &water_color, &water_ambient, &water_diffuse, &water_specular, &water_shininess, &disA, &disB, &disC, &sun_cenit, &sun_azim,
            &light_ambient, &light_diffuse, &light_specular);

        guiMenu.endRender();


        // Render end, swap buffers

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1, &VAO);
    //glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);

    guiMenu.destroy();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, bool* fill)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        *fill = false;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        *fill = true;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        camera.ProcessKeyboardMovement(ORIGIN, deltaTime);

    if (globaLView)
    {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.ProcessKeyboardRotation(AZIM_UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.ProcessKeyboardRotation(AZIM_DOWN, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.ProcessKeyboardRotation(ZEN_LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.ProcessKeyboardRotation(ZEN_RIGHT, deltaTime);
    }
    else {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            shipMovement.ProcessKeyboardRotation(THETA_UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            shipMovement.ProcessKeyboardRotation(THETA_UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            shipMovement.ProcessKeyboardRotation(PHI_LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            shipMovement.ProcessKeyboardRotation(PHI_RIGHT, deltaTime);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (globaLView) {
        camera.ProcessMouseScroll(yoffset);
    }
    else {
        shipMovement.ProcessMouseScroll(yoffset);
    }

}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    float posX = 2 * (xpos - SCR_WIDTH / 2) / SCR_WIDTH;
    float posY = 2 * (SCR_HEIGHT / 2 - ypos) / SCR_HEIGHT;
    if (globaLView) {
        camera.SetCurrentMousePos(posX, posY);
    }
    else {
        shipMovement.SetCurrentMousePos(posX, posY);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        camera.SetRotDrag(true);
        shipMovement.SetRotDrag(true);
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        camera.SetRotDrag(false);
        shipMovement.SetRotDrag(false);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        camera.SetCenterDrag(true);
        shipMovement.SetVerticalDrag(true);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        camera.SetCenterDrag(false);
        shipMovement.SetVerticalDrag(false);
    }
}

void createSeaMesh(float*& vertices, unsigned int*& indices, unsigned int N, glm::vec3 initPos, glm::vec3 finalPos, float sizeUV) {
    int vertexSize = 5;
    int indexSize = 3;
    float xGap = (finalPos.x - initPos.x) / ((float)N - 1.0f);
    float yGap = (finalPos.y - initPos.y) / ((float)N - 1.0f);
    vertices = new float[N * N * vertexSize];
    indices = new unsigned int[(N - 1) * (N - 1) * 2 * indexSize];

    for (unsigned int j = 0; j < N; j++) {
        for (unsigned int i = 0; i < N; i++) {
            float tempv[] = {
                initPos.x + i * xGap, initPos.y + j * yGap, initPos.z,
                0.0f + ((float)i / (float)N) * sizeUV, sizeUV - ((float)j / (float)N) * sizeUV };
            copy(tempv, tempv + (1 * vertexSize), (vertices + (j * N + i) * vertexSize));

            if ((j < N - 1) && (i < N - 1)) {
                unsigned int tempi[] = {
                    (j * N + i), (j * N + (i + 1)), ((j + 1) * N + (i + 1)),
                    ((j + 1) * N + (i + 1)), ((j + 1) * N + (i + 0)), ((j + 0) * N + (i + 0)) };
                copy(tempi, tempi + (2 * indexSize), (indices + (j * (N - 1) + i) * 2 * indexSize));
            }
        }
    }
}

glm::vec3 InterpColor(glm::vec3 color1, glm::vec3 color2, float t)
{
    return color1 * (1.0f - t) + color2 * t;
}

glm::vec3 GetSkyColor(float cenit) {
    glm::vec3 color1 = glm::vec3(0.701f, 1.0f, 0.996f);
    glm::vec3 color2 = glm::vec3(0.345f, 0.796f, 0.996f);
    glm::vec3 color3 = glm::vec3(1.0f, 0.682f, 0.180f);
    glm::vec3 color4 = glm::vec3(0.929f, 0.349f, 0.007f);
    glm::vec3 color5 = glm::vec3(0.003f, 0.031f, 0.517f);
    glm::vec3 color6 = glm::vec3(0.0f);

    glm::vec3 skyColor = glm::vec3(1.0f);
    float middleDay = 0.0f;
    float afternoon = 60.0f;
    float sunsetStart = 70.0f;
    float sunsetMiddle = 80.0f;
    float sunsetEnd = 95.0f;
    float midnight = 180.0f;

    if (cenit < afternoon) {
        skyColor = InterpColor(color1, color2, cenit / (afternoon - middleDay));
    }
    else if (cenit < sunsetStart) {
        skyColor = InterpColor(color2, color3, (cenit - afternoon) / (sunsetStart - afternoon));
    }
    else if (cenit < sunsetMiddle) {
        skyColor = InterpColor(color3, color4, (cenit - sunsetStart) / (sunsetMiddle - sunsetStart));
    }
    else if (cenit < sunsetEnd) {
        skyColor = InterpColor(color4, color5, (cenit - sunsetMiddle) / (sunsetEnd - sunsetMiddle));
    }
    else {
        skyColor = InterpColor(color5, color6, (cenit - sunsetEnd) / (midnight - sunsetEnd));
    }

    return skyColor;
}

unsigned int loadTexture(string path, GLuint mode) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}