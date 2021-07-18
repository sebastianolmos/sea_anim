#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "shader/shader.h"
#include "util/performanceMonitor.h"
#include "util/camera3d.h"
#include "util/model.h"
#include "util/shipMovement.h"
#include <glm/gtx/norm.hpp >

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, bool* fill);
static void draw_vec3_widget(const std::string& label, glm::vec3& values, float columnWidth);
void createSeaMesh(float*& vertices, unsigned int*& indices, unsigned int N, glm::vec3 initPos, glm::vec3 finalPos, float sizeUV);
glm::vec3 GerstnerWave(glm::vec4 wave, glm::vec3 p, glm::vec3& tangent, glm::vec3& binormal, float gravity, float time);
glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest);
glm::vec3 GetSkyColor(float cenit);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera3D camera(glm::vec3(0.0f, 0.0f, 0.0f));
ShipMovement shipMovement(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

const double PI = 3.141592653589793238463;

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

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

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
    glBufferData(GL_ARRAY_BUFFER, (seaSize * seaSize)*5*sizeof(float), seaVertices, GL_STATIC_DRAW);

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
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    unsigned char* data = stbi_load("../assets/water2.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load("../assets/displacement1.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

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
    unsigned int texture3;
    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load("../assets/sun2.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    sunShader.use(); // don't forget to activate/use the shader before setting uniforms!
    sunShader.setInt("texture1", 0);

    // Enabling transparencies
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 410";

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    auto& colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };

    colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f };
    colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
    colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };

    colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f };
    colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
    colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };

    colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f };
    colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
    colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };

    colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };
    colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.38f, 0.38f, 1.0f };
    colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.28f, 0.28f, 1.0f };
    colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f };

    colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };
    colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

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

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);

    PerformanceMonitor pMonitor(glfwGetTime(), 0.5f);

    glm::vec3 mColor = { 1.0f, 0.0f, 0.0f };
    float mRoughness = 0.2f;
    float mMetallic = 0.1f;
    glm::vec3 mPosition = { 1.5f, 3.5f, 3.0f };
    glm::vec2 mSize = { 800, 800 };
    bool fillPolygon = true;

    float sun_cenit = 35.749f;
    float sun_azim = 34.596f;
    //float wave_amplitud = 1.0f;
    //float wave_steepness= 0.5f;
    //float wave_length = 10.0f;
    //float wave_speed = 1.0f;
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

    float disA_size = 1.531f;
    glm::vec2 disA_dir = glm::vec2(1.0f);
    float disA_speed = 0.015f;
    float disA_strenght = 3.429f;
    glm::vec3 disA_color = glm::vec3(0.358f, 0.358f, 0.358f);
    glm::vec2 disA_discard = glm::vec2(0.269f, 0.333f);
    bool disA_inside = true;

    float disB_size = 2.963f;
    glm::vec2 disB_dir = glm::vec2(-0.478f, 0.507f);
    float disB_speed = 0.021f;
    float disB_strenght = 1.282f;
    glm::vec3 disB_color = glm::vec3(0.485f, 0.485f, 0.485f);;
    glm::vec2 disB_discard = glm::vec2(0.186f, 0.269f);
    bool disB_inside = false;

    float disC_size = 4.599f;
    glm::vec2 disC_dir = glm::vec2(-1.0f, 1.0f);
    float disC_speed = 0.044f;
    float disC_strenght = 2.179f;
    glm::vec3 disC_color = glm::vec3(0.200f, 0.200f, 0.200f);
    glm::vec2 disC_discard = glm::vec2(0.071f, 0.353f);
    bool disC_inside = false;

    float ship_size = 1.0f;
    glm::vec3 ship_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 ship_tangent = glm::vec3(1.0f);
    glm::vec3 ship_binormal = glm::vec3(1.0f);
    glm::vec3 ship_normal = glm::vec3(1.0f);
    float ship_rotation = 34.1f;

    glm::vec3 sky_color = glm::vec3(1.0f);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float t1 = glfwGetTime();
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

        // prerender uicontext
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create the docking environment
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("InvisibleWindow", nullptr, windowFlags);
        ImGui::PopStyleVar(3);

        ImGuiID dockSpaceId = ImGui::GetID("InvisibleWindowDockSpace");

        ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

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
        p += GerstnerWave(wave_A, ship_pos, ship_tangent, ship_binormal, gravity, t1);
        p += GerstnerWave(wave_B, ship_pos, ship_tangent, ship_binormal, gravity, t1);
        p += GerstnerWave(wave_C, ship_pos, ship_tangent, ship_binormal, gravity, t1);
        ship_normal = glm::normalize(cross(ship_tangent, ship_binormal));
        glm::mat4 rotate = glm::mat4_cast(RotationBetweenVectors(
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

        seaShader.setFloat("disASize", disA_size);
        seaShader.setVec2("disADir", disA_dir);
        seaShader.setFloat("disASpeed", disA_speed);
        seaShader.setFloat("disAStrenght", disA_strenght);
        seaShader.setVec3("disAColor", disA_color);
        seaShader.setVec2("disADiscard", disA_discard);
        seaShader.setBool("disAInside", disA_inside);

        seaShader.setFloat("disBSize", disB_size);
        seaShader.setVec2("disBDir", disB_dir);
        seaShader.setFloat("disBSpeed", disB_speed);
        seaShader.setFloat("disBStrenght", disB_strenght);
        seaShader.setVec3("disBColor", disB_color);
        seaShader.setVec2("disBDiscard", disB_discard);
        seaShader.setBool("disBInside", disB_inside);

        seaShader.setFloat("disCSize", disC_size);
        seaShader.setVec2("disCDir", disC_dir);
        seaShader.setFloat("disCSpeed", disC_speed);
        seaShader.setFloat("disCStrenght", disC_strenght);
        seaShader.setVec3("disCColor", disC_color);
        seaShader.setVec2("disCDiscard", disC_discard);
        seaShader.setBool("disCInside", disC_inside);

        // render the sea
        glBindVertexArray(seaVAO);
        glDrawElements(GL_TRIANGLES, (seaSize - 1)* (seaSize - 1) * 2 * 3, GL_UNSIGNED_INT, 0);

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

        ImGui::Begin("Scene");
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        mSize = { viewportPanelSize.x, viewportPanelSize.y };

        // add rendered texture to ImGUI scene window
        ImGui::Image(reinterpret_cast<void*>(mTexId), ImVec2{ mSize.x, mSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        ImGui::End();

        ImGui::Begin("Properties");

        if (ImGui::CollapsingHeader("Ship"))
        {
            ImGui::PushID(30);
            ImGui::Text("Position");
            ImGui::Separator();
            draw_vec3_widget("Position", ship_pos, 80.0f);
            ImGui::Separator();
            ImGui::SliderFloat("Size", &ship_size, 0.01, 3.0f);
            ImGui::Separator();
            ImGui::SliderFloat("Rotation", &ship_rotation, 0.00, 360.0f);
            ImGui::PopID();
        }

        if (ImGui::CollapsingHeader("Water Material"))
        {
            if (ImGui::TreeNode("Water Color"))
            {
                ImGui::ColorPicker3("Color", (float*)&water_color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Water Ambient"))
            {
                ImGui::ColorPicker3("Ambient", (float*)&water_ambient, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Water Diffuse"))
            {
                ImGui::ColorPicker3("Diffuse", (float*)&water_diffuse, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Water Specular"))
            {
                ImGui::ColorPicker3("Specular", (float*)&water_specular, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                ImGui::TreePop();
            }
            ImGui::SliderFloat("shininess", &water_shininess, 1.0f, 100.0f);
        }

        if (ImGui::CollapsingHeader("Waves"))
        {
            ImGui::Separator();
            ImGui::SliderFloat("Gravity", &gravity, 0.1f, 20.0f);
            ImGui::Separator();
            if (ImGui::TreeNode("First Wave"))
            {
                ImGui::PushID(1);
                ImGui::SliderFloat2("Direction", &wave_A.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Steepness", &wave_A.z, 0.0f, 1.0f);
                ImGui::SliderFloat("WaveLength", &wave_A.w, 1.0f, 20.0f);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Second Wave"))
            {
                ImGui::PushID(2);
                ImGui::SliderFloat2("Direction", &wave_B.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Steepness", &wave_B.z, 0.0f, 1.0f);
                ImGui::SliderFloat("WaveLength", &wave_B.w, 1.0f, 20.0f);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Third Wave"))
            {
                ImGui::PushID(3);
                ImGui::SliderFloat2("Direction", &wave_C.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Steepness", &wave_C.z, 0.0f, 1.0f);
                ImGui::SliderFloat("WaveLength", &wave_C.w, 1.0f, 20.0f);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
        }
        if (ImGui::CollapsingHeader("Displace"))
        {
            if (ImGui::TreeNode("First texture"))
            {
                ImGui::PushID(4);
                ImGui::SliderFloat2("Direction", &disA_dir.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Size", &disA_size, 0.1f, 32.0f);
                ImGui::SliderFloat("Speed", &disA_speed, 0.0001f, 0.1f);
                ImGui::SliderFloat("Strenght", &disA_strenght, 0.0f, 5.0f);
                if (ImGui::TreeNode("Texture Color"))
                {
                    ImGui::ColorPicker3("Color", (float*)&disA_color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                    ImGui::TreePop();
                }
                ImGui::SliderFloat("Lmt 1", &disA_discard.x, 0.0f, 1.0f);
                ImGui::SliderFloat("Lmt 2", &disA_discard.y, 0.0f, 1.0f);
                ImGui::Checkbox("Include inside?", &disA_inside);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Second texture"))
            {
                ImGui::PushID(5);
                ImGui::SliderFloat2("Direction", &disB_dir.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Size", &disB_size, 0.1f, 32.0f);
                ImGui::SliderFloat("Speed", &disB_speed, 0.0001f, 0.1f);
                ImGui::SliderFloat("Strenght", &disB_strenght, 0.0f, 5.0f);
                if (ImGui::TreeNode("Texture Color"))
                {
                    ImGui::ColorPicker3("Color", (float*)&disB_color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                    ImGui::TreePop();
                }
                ImGui::SliderFloat("Lmt 1", &disB_discard.x, 0.0f, 1.0f);
                ImGui::SliderFloat("Lmt 2", &disB_discard.y, 0.0f, 1.0f);
                ImGui::Checkbox("Include inside?", &disB_inside);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Third texture"))
            {
                ImGui::PushID(6);
                ImGui::SliderFloat2("Direction", &disC_dir.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Size", &disC_size, 0.1f, 32.0f);
                ImGui::SliderFloat("Speed", &disC_speed, 0.0001f, 0.1f);
                ImGui::SliderFloat("Strenght", &disC_strenght, 0.0f, 5.0f);
                if (ImGui::TreeNode("Texture Color"))
                {
                    ImGui::ColorPicker3("Color", (float*)&disC_color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                    ImGui::TreePop();
                }
                ImGui::SliderFloat("Lmt 1", &disC_discard.x, 0.0f, 1.0f);
                ImGui::SliderFloat("Lmt 2", &disC_discard.y, 0.0f, 1.0f);
                ImGui::Checkbox("Include inside?", &disC_inside);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
        }

        if (ImGui::CollapsingHeader("Light"))
        {
            ImGui::Separator();
            ImGui::SliderFloat("Cenit", &sun_cenit, 0.0f, 179.9f);
            ImGui::Separator();
            ImGui::SliderFloat("Azim", &sun_azim, -179.9f, 179.9f);
            ImGui::Separator();
            ImGui::SliderFloat("Ambient", &light_ambient, 0.0, 1.0f);
            ImGui::Separator();
            ImGui::SliderFloat("Diffuse", &light_diffuse, 0.0, 1.0f);
            ImGui::Separator();
            ImGui::SliderFloat("Specular", &light_specular, 0.0, 1.0f);
            ImGui::Separator();
            //ImGui::Text("Position");
            //ImGui::Separator();
            //draw_vec3_widget("Position", mPosition, 80.0f);

        }
        viewName = globaLView ? "Ship View" : "Global View";
        if (ImGui::Button(viewName.c_str()))
        {
            globaLView = !globaLView;
            if (globaLView)
            {
                ship_pos.z = 0.0f;
            }
            else {
                ship_pos.z = 3.0f;
            }
            
        }
        ImGui::Separator();

        if (ImGui::Button("Configuration 1"))
        {
            gravity = 9.8f;
            wave_A = glm::vec4(-0.072f, 1.0f, 0.346f, 20.0f);
            wave_B = glm::vec4(0.855f, -0.536f, 0.417f, 12.814f);
            wave_C = glm::vec4(0.449f, 0.362f, 0.712f, 19.026f);

            water_color = glm::vec3(0.279f, 0.528f, 1.0f);
            water_ambient = glm::vec3(0.0f, 0.006f, 0.359f);
            water_diffuse = glm::vec3(0.05f, 0.165f, 0.65f);
            water_specular = glm::vec3(0.491f, 0.492f, 0.492f);
            water_shininess = 67.0f;;

            disA_size = 1.531f;
            disA_dir = glm::vec2(1.0f);
            disA_speed = 0.015f;
            disA_strenght = 3.429f;
            disA_color = glm::vec3(0.358f, 0.358f, 0.358f);
            disA_discard = glm::vec2(0.269f, 0.333f);
            disA_inside = true;

            disB_size = 2.963f;
            disB_dir = glm::vec2(-0.478f, 0.507f);
            disB_speed = 0.021f;
            disB_strenght = 1.282f;
            disB_color = glm::vec3(0.485f, 0.485f, 0.485f);
            disB_discard = glm::vec2(0.186f, 0.269f);
            disB_inside = false;

            disC_size = 4.599f;
            disC_dir = glm::vec2(-1.0f, 1.0f);
            disC_speed = 0.044f;
            disC_strenght = 2.179f;
            disC_color = glm::vec3(0.200f, 0.200f, 0.200f);
            disC_discard = glm::vec2(0.071f, 0.353f);
            disC_inside = false;

            sun_cenit = 35.749f;
            sun_azim = 34.596f;

            light_ambient = 0.115f;
            light_diffuse = 0.833f;
            light_specular = 0.756f;
        }
        if (ImGui::Button("Configuration 2"))
        {
            gravity = 9.8f;
            wave_A = glm::vec4(-0.571f, 0.029f, 0.272f, 7.133f);
            wave_B = glm::vec4(0.086f, -0.229f, 0.259f, 8.215f);
            wave_C = glm::vec4(-0.114f, -0.057f, 0.418f, 5.329f);

            water_color = glm::vec3(0.126f, 0.726f, 1.0f);
            water_ambient = glm::vec3(0.00f, 0.080f, 0.477f);
            water_diffuse = glm::vec3(0.0f, 0.584f, 0.846f);
            water_specular = glm::vec3(0.340f, 0.340f, 0.340f);
            water_shininess = 100.0f;;

            disA_size = 1.0f;
            disA_dir = glm::vec2(-1.0f, 1.0f);
            disA_speed = 0.006f;
            disA_strenght = 0.823f;
            disA_color = glm::vec3(0.413f, 0.562f, 1.00f);
            disA_discard = glm::vec2(0.139f, 0.333f);
            disA_inside = true;

            disB_size = 18.877f;
            disB_dir = glm::vec2(0.629f, 0.507f);
            disB_speed = 0.010f;
            disB_strenght = 1.994f;
            disB_color = glm::vec3(0.734f, 0.893f, 1.0f);
            disB_discard = glm::vec2(0.025f, 0.196f);
            disB_inside = true;

            disC_size = 11.406f;
            disC_dir = glm::vec2(-1.0f, -0.629f);
            disC_speed = 0.018f;
            disC_strenght = 0.285f;
            disC_color = glm::vec3(0.210f, 0.616f, 1.0f);
            disC_discard = glm::vec2(0.1f, 0.259f);
            disC_inside = false;

            sun_cenit = 53.515f;
            sun_azim = 9.109f;

            light_ambient = 0.115f;
            light_diffuse = 0.833f;
            light_specular = 0.756f;
        }
        if (ImGui::Button("Configuration 3"))
        {
            gravity = 9.8f;
            wave_A = glm::vec4(1.0f, 1.0f, 0.5f, 10.0f);
            wave_B = glm::vec4(0.1f, 1.0f, 0.3f, 4.0f);
            wave_C = glm::vec4(0.4f, 0.2f, 0.7f, 2.0f);

            water_color = glm::vec3(1.0f, 0.409f, 0.3f);
            water_ambient = glm::vec3(0.049f, 0.049f, 0.628f);
            water_diffuse = glm::vec3(0.688f, 0.068f, 0.0f);
            water_specular = glm::vec3(1.0f, 0.612f, 0.00f);
            water_shininess = 41.0f;;

            disA_size = 12.618f;
            disA_dir = glm::vec2(1.0f);
            disA_speed = 0.037f;
            disA_strenght = 1.0f;
            disA_color = glm::vec3(1.0f);
            disA_discard = glm::vec2(0.19f, 0.5f);
            disA_inside = true;

            disB_size = 22.107f;
            disB_dir = glm::vec2(-1.0f);
            disB_speed = 0.011f;
            disB_strenght = 1.930f;
            disB_color = glm::vec3(1.0f);
            disB_discard = glm::vec2(0.2f, 0.8f);
            disB_inside = true;

            disC_size = 6.359f;
            disC_dir = glm::vec2(-0.686f, 0.429f);
            disC_speed = 0.039f;
            disC_strenght = 0.823f;
            disC_color = glm::vec3(1.0f);
            disC_discard = glm::vec2(0.165f, 0.392f);
            disC_inside = false;

            sun_cenit = 79.703f;
            sun_azim = 0.80f;

            light_ambient = 0.310f;
            light_diffuse = 0.551f;
            light_specular = 0.842f;
        }
        if (ImGui::Button("Configuration 4"))
        {
            gravity = 1.234;
            wave_A = glm::vec4(0.571f, -0.057f, 0.076f, 12.665f);
            wave_B = glm::vec4(-0.457f, -0.314f, 0.475f, 14.100f);
            wave_C = glm::vec4(-0.371f, 0.486f, 0.215f, 15.070f);

            water_color = glm::vec3(1.0f, 0.199f, 0.0f);
            water_ambient = glm::vec3(0.921f, 0.384f, 0.165f);
            water_diffuse = glm::vec3(0.813f, 0.375f, 0.137f);
            water_specular = glm::vec3(0.159f, 0.133f, 0.006f);
            water_shininess = 100.0f;;

            disA_size = 2.523f;
            disA_dir = glm::vec2(-1.0f, 1.0f);
            disA_speed = 0.013f;
            disA_strenght = 3.07f;
            disA_color = glm::vec3(1.0f, 0.901f, 0.352f);
            disA_discard = glm::vec2(0.025f, 0.259f);
            disA_inside = true;

            disB_size = 4.946f;
            disB_dir = glm::vec2(-0.514f, 0.371f);
            disB_speed = 0.013f;
            disB_strenght = 1.962f;
            disB_color = glm::vec3(1.0f, 0.839f, 0.00f);
            disB_discard = glm::vec2(0.057f, 0.589f);
            disB_inside = true;

            disC_size = 3.532f;
            disC_dir = glm::vec2(-0.171f, 0.4f);
            disC_speed = 0.014f;
            disC_strenght = 2.722f;
            disC_color = glm::vec3(1.0f, 0.563f, 0.00f);
            disC_discard = glm::vec2(0.247f, 0.576f);
            disC_inside = false;

            sun_cenit = 53.515f;
            sun_azim = 9.189f;

            light_ambient = 0.115f;
            light_diffuse = 0.8335f;
            light_specular = 0.756f;
        }
        ImGui::End();

       

        // Render the UI 
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }


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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

static void draw_vec3_widget(const std::string& label, glm::vec3& values, float columnWidth = 100.0f)
{
    ImGuiIO & io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
    
    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
    ImGui::PushFont(boldFont);
    ImGui::Button("X", buttonSize);
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushFont(boldFont);
    ImGui::Button("Y", buttonSize);
    ImGui::PopFont();

    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushFont(boldFont);
    ImGui::Button("Z", buttonSize);
    ImGui::PopFont();
    
    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
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
    indices = new unsigned int[(N - 1)*(N - 1) * 2 * indexSize];

    for (unsigned int j = 0; j < N; j++) {
        for (unsigned int i = 0; i < N; i++) {
            float tempv[] = {
                initPos.x + i * xGap, initPos.y + j *yGap, initPos.z, 
                0.0f + ((float)i/(float)N) * sizeUV, sizeUV - ((float)j / (float)N) * sizeUV };
            copy(tempv, tempv + (1 * vertexSize), (vertices + (j*N + i) * vertexSize));

            if ( (j < N - 1) && (i < N - 1) ) {
                unsigned int tempi[] = {
                    (j * N + i), (j * N + (i + 1)), ((j+1) * N + (i+1)),
                    ((j + 1) * N + (i + 1)), ((j + 1) * N + (i + 0)), ((j + 0) * N + (i + 0)) };
                copy(tempi, tempi + (2 * indexSize), (indices + (j * (N - 1) + i) * 2 * indexSize));
            }
        }
    }
}

glm::vec3 GerstnerWave(glm::vec4 wave, glm::vec3 p, glm::vec3& tangent, glm::vec3& binormal, float gravity, float time)
{
    float steepness = wave.z;
    float waveLength = wave.w;
    float k = 2 * PI / waveLength;
    float c = glm::sqrt(gravity / k);
    glm::vec2 d = glm::normalize(glm::vec2(wave.x, wave.y));
    float f = k * (glm::dot(d, glm::vec2(p.x, p.y)) - c * time);
    float a = steepness / k;
    tangent.x = 1 - d.x * d.x * (steepness * glm::sin(f));
    tangent.y = -d.x * d.y * (steepness * glm::sin(f));
    tangent.z = d.x * (steepness * glm::cos(f));

    binormal.x = -d.x * d.y * (steepness * glm::sin(f));
    binormal.y = 1 - d.y * d.y * (steepness * glm::sin(f));
    binormal.z = d.y * (steepness * glm::cos(f));
    return glm::vec3(
        d.x * (a * glm::cos(f)),
        d.y * (a * glm::cos(f)),
        a * glm::sin(f)
    );
}

glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest) {
    start = glm::normalize(start);
    dest = glm::normalize(dest);

    float cosTheta = glm::dot(start, dest);
    glm::vec3 rotationAxis;

    if (cosTheta < -1 + 0.001f) {
        // special case when vectors in opposite directions:
        // there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
        if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
            rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

        rotationAxis = glm::normalize(rotationAxis);
        return glm::angleAxis(glm::radians(180.0f), rotationAxis);
    }

    rotationAxis = glm::cross(start, dest);

    float s = glm::sqrt((1 + cosTheta) * 2);
    float invs = 1 / s;

    return glm::quat(
        s * 0.5f,
        rotationAxis.x * invs,
        rotationAxis.y * invs,
        rotationAxis.z * invs
    );


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