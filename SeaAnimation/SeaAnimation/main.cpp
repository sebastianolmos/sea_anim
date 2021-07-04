#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader/shader.h"
#include "util/performanceMonitor.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, bool* fill);
static void draw_vec3_widget(const std::string& label, glm::vec3& values, float columnWidth);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 720;

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

    string title = "Simple Quad";
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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }


    // build and compile our shader program
    // ------------------------------------
    Shader basicShader("shader/basicShader.vs", "shader/basicShader.fs"); // you can name your shader files however you like

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    float vertices[] = {
        // positions         // colors
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // bottom left
        -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f   // top left 
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // glEnable(GL_DEPTH_TEST);

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
    int32_t mHeight = 600;
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

    bool fillPolygon = true;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        pMonitor.update(glfwGetTime());
        stringstream ss;
        ss << title << " " << pMonitor;
        glfwSetWindowTitle(window, ss.str().c_str());

        // input
        // -----
        processInput(window, &fillPolygon);

        // prerender openglcontext
        // ------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

        // render scene to framebuffer and add it to scene view
        if (fillPolygon)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // render the triangle
        basicShader.use();

        // bind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glViewport(0, 0, mWidth, mHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // unbind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ImGui::Begin("Scene");
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        glm::vec2 mSize = { viewportPanelSize.x, viewportPanelSize.y };

        // add rendered texture to ImGUI scene window
        ImGui::Image(reinterpret_cast<void*>(mTexId), ImVec2{ mSize.x, mSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        ImGui::End();

        ImGui::Begin("Properties");

        if (ImGui::CollapsingHeader("Material"))
        {
            ImGui::ColorPicker3("Color", (float*)&mColor, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
            ImGui::SliderFloat("Roughness", &mRoughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallic", &mMetallic, 0.0f, 1.0f);
        }

        if (ImGui::CollapsingHeader("Light"))
        {

            ImGui::Separator();
            ImGui::Text("Position");
            ImGui::Separator();
            draw_vec3_widget("Position", mPosition, 80.0f);
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
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
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