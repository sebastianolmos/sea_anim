#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct displace {
    float size;
    glm::vec2 direction;
    float speed;
    float strenght;
    glm::vec3 color;
    glm::vec2 discard;
    bool inside;
};

class Menu {
public:
    void init(GLFWwindow* window) {
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
    }

    void preRender() {
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
    }
    void draw_vec3_widget(const std::string& label, glm::vec3& values, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
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

    glm::vec2 begin(uint32_t id) {
        ImGui::Begin("Scene");
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        glm::vec2 mSize = { viewportPanelSize.x, viewportPanelSize.y };

        // add rendered texture to ImGUI scene window
        ImGui::Image(reinterpret_cast<void*>(id), ImVec2{ mSize.x, mSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        ImGui::End();

        ImGui::Begin("Properties");
        return mSize;
    }

    void setShip(glm::vec3* pos, float* size, float* rotation) {
        if (ImGui::CollapsingHeader("Ship"))
        {
            ImGui::PushID(30);
            ImGui::Text("Position");
            ImGui::Separator();
            draw_vec3_widget("Position", *pos, 80.0f);
            ImGui::Separator();
            ImGui::SliderFloat("Size", size, 0.01, 3.0f);
            ImGui::Separator();
            ImGui::SliderFloat("Rotation", rotation, 0.00, 360.0f);
            ImGui::PopID();
        }
    }

    void setWaterMaterial(glm::vec3* color, glm::vec3* ambient, glm::vec3* diffuse, glm::vec3* specular, float* shininess) {
        if (ImGui::CollapsingHeader("Water Material"))
        {
            if (ImGui::TreeNode("Water Color"))
            {
                ImGui::ColorPicker3("Color", (float*)color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Water Ambient"))
            {
                ImGui::ColorPicker3("Ambient", (float*)ambient, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Water Diffuse"))
            {
                ImGui::ColorPicker3("Diffuse", (float*)diffuse, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Water Specular"))
            {
                ImGui::ColorPicker3("Specular", (float*)specular, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                ImGui::TreePop();
            }
            ImGui::SliderFloat("shininess", shininess, 1.0f, 100.0f);
        }
    }

    void setWaves(float* gravity, glm::vec4* wave_A, glm::vec4* wave_B, glm::vec4* wave_C) {
        if (ImGui::CollapsingHeader("Waves"))
        {
            ImGui::Separator();
            ImGui::SliderFloat("Gravity", gravity, 0.1f, 20.0f);
            ImGui::Separator();
            if (ImGui::TreeNode("First Wave"))
            {
                ImGui::PushID(1);
                ImGui::SliderFloat2("Direction", &wave_A->x, -1.0f, 1.0f);
                ImGui::SliderFloat("Steepness", &wave_A->z, 0.0f, 1.0f);
                ImGui::SliderFloat("WaveLength", &wave_A->w, 1.0f, 20.0f);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Second Wave"))
            {
                ImGui::PushID(2);
                ImGui::SliderFloat2("Direction", &wave_B->x, -1.0f, 1.0f);
                ImGui::SliderFloat("Steepness", &wave_B->z, 0.0f, 1.0f);
                ImGui::SliderFloat("WaveLength", &wave_B->w, 1.0f, 20.0f);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Third Wave"))
            {
                ImGui::PushID(3);
                ImGui::SliderFloat2("Direction", &wave_C->x, -1.0f, 1.0f);
                ImGui::SliderFloat("Steepness", &wave_C->z, 0.0f, 1.0f);
                ImGui::SliderFloat("WaveLength", &wave_C->w, 1.0f, 20.0f);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
        }
    }

    void setTextures(displace* disA, displace* disB, displace* disC) {
        if (ImGui::CollapsingHeader("Displace"))
        {
            if (ImGui::TreeNode("First texture"))
            {
                ImGui::PushID(4);
                ImGui::SliderFloat2("Direction", &disA->direction.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Size", &disA->size, 0.1f, 32.0f);
                ImGui::SliderFloat("Speed", &disA->speed, 0.0001f, 0.1f);
                ImGui::SliderFloat("Strenght", &disA->strenght, 0.0f, 5.0f);
                if (ImGui::TreeNode("Texture Color"))
                {
                    ImGui::ColorPicker3("Color", (float*)&disA->color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                    ImGui::TreePop();
                }
                ImGui::SliderFloat("Lmt 1", &disA->discard.x, 0.0f, 1.0f);
                ImGui::SliderFloat("Lmt 2", &disA->discard.y, 0.0f, 1.0f);
                ImGui::Checkbox("Include inside?", &disA->inside);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Second texture"))
            {
                ImGui::PushID(5);
                ImGui::SliderFloat2("Direction", &disB->direction.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Size", &disB->size, 0.1f, 32.0f);
                ImGui::SliderFloat("Speed", &disB->speed, 0.0001f, 0.1f);
                ImGui::SliderFloat("Strenght", &disB->strenght, 0.0f, 5.0f);
                if (ImGui::TreeNode("Texture Color"))
                {
                    ImGui::ColorPicker3("Color", (float*)&disB->color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                    ImGui::TreePop();
                }
                ImGui::SliderFloat("Lmt 1", &disB->discard.x, 0.0f, 1.0f);
                ImGui::SliderFloat("Lmt 2", &disB->discard.y, 0.0f, 1.0f);
                ImGui::Checkbox("Include inside?", &disB->inside);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Third texture"))
            {
                ImGui::PushID(6);
                ImGui::SliderFloat2("Direction", &disC->direction.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Size", &disC->size, 0.1f, 32.0f);
                ImGui::SliderFloat("Speed", &disC->speed, 0.0001f, 0.1f);
                ImGui::SliderFloat("Strenght", &disC->strenght, 0.0f, 5.0f);
                if (ImGui::TreeNode("Texture Color"))
                {
                    ImGui::ColorPicker3("Color", (float*)&disC->color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
                    ImGui::TreePop();
                }
                ImGui::SliderFloat("Lmt 1", &disC->discard.x, 0.0f, 1.0f);
                ImGui::SliderFloat("Lmt 2", &disC->discard.y, 0.0f, 1.0f);
                ImGui::Checkbox("Include inside?", &disC->inside);
                ImGui::Separator();
                ImGui::PopID();
                ImGui::TreePop();
            }
        }
    }

    void setLight(float* cenit, float* azim, float* ambient, float *diffuse, float* specular) {
        if (ImGui::CollapsingHeader("Light"))
        {
            ImGui::Separator();
            ImGui::SliderFloat("Cenit", cenit, 0.0f, 179.9f);
            ImGui::Separator();
            ImGui::SliderFloat("Azim", azim, -179.9f, 179.9f);
            ImGui::Separator();
            ImGui::SliderFloat("Ambient", ambient, 0.0, 1.0f);
            ImGui::Separator();
            ImGui::SliderFloat("Diffuse", diffuse, 0.0, 1.0f);
            ImGui::Separator();
            ImGui::SliderFloat("Specular", specular, 0.0, 1.0f);
            ImGui::Separator();

        }
    }

    void setView(bool* globalView, glm::vec3* pos) {
        string viewName = *globalView ? "Ship View" : "Global View";
        if (ImGui::Button(viewName.c_str()))
        {
            *globalView = !(*globalView);
            if (*globalView)
            {
                pos->z = 0.0f;
            }
            else {
                pos->z = 2.0f;
            }

        }
        ImGui::Separator();
    }

    void config1(float* gr, glm::vec4* wA, glm::vec4* wB, glm::vec4* wC, glm::vec3* wtClr, glm::vec3* wtAm, glm::vec3* wtDf, glm::vec3* wtSp, float* wtSh,
        displace* dA, displace* dB, displace* dC, float* cn, float* az, float* la, float* ld, float *ls) {
        if (ImGui::Button("Configuration 1"))
        {
            *gr = 9.8f;
            *wA = glm::vec4(-0.072f, 1.0f, 0.346f, 20.0f);
            *wB = glm::vec4(0.855f, -0.536f, 0.417f, 12.814f);
            *wC = glm::vec4(0.449f, 0.362f, 0.712f, 19.026f);

            *wtClr = glm::vec3(0.279f, 0.528f, 1.0f);
            *wtAm = glm::vec3(0.0f, 0.006f, 0.359f);
            *wtDf = glm::vec3(0.05f, 0.165f, 0.65f);
            *wtSp = glm::vec3(0.491f, 0.492f, 0.492f);
            *wtSh = 67.0f;;

            dA->size = 1.531f;
            dA->direction = glm::vec2(1.0f);
            dA->speed = 0.015f;
            dA->strenght = 3.429f;
            dA->color = glm::vec3(0.358f, 0.358f, 0.358f);
            dA->discard = glm::vec2(0.269f, 0.333f);
            dA->inside = true;

            dB->size = 2.963f;
            dB->direction = glm::vec2(-0.478f, 0.507f);
            dB->speed = 0.021f;
            dB->strenght = 1.282f;
            dB->color = glm::vec3(0.485f, 0.485f, 0.485f);
            dB->discard = glm::vec2(0.186f, 0.269f);
            dB->inside = false;

            dC->size = 4.599f;
            dC->direction = glm::vec2(-1.0f, 1.0f);
            dC->speed = 0.044f;
            dC->strenght = 2.179f;
            dC->color = glm::vec3(0.200f, 0.200f, 0.200f);
            dC->discard = glm::vec2(0.071f, 0.353f);
            dC->inside = false;

            *cn = 35.749f;
            *az = 34.596f;

            *la = 0.115f;
            *ld = 0.833f;
            *ls = 0.756f;
        }
    }

    void config2(float* gr, glm::vec4* wA, glm::vec4* wB, glm::vec4* wC, glm::vec3* wtClr, glm::vec3* wtAm, glm::vec3* wtDf, glm::vec3* wtSp, float* wtSh,
        displace* dA, displace* dB, displace* dC, float* cn, float* az, float* la, float* ld, float* ls) {
        if (ImGui::Button("Configuration 2"))
        {
            *gr = 9.8f;
            *wA = glm::vec4(-0.571f, 0.029f, 0.272f, 7.133f);
            *wB = glm::vec4(0.086f, -0.229f, 0.259f, 8.215f);
            *wC = glm::vec4(-0.114f, -0.057f, 0.418f, 5.329f);

            *wtClr = glm::vec3(0.126f, 0.726f, 1.0f);;
            *wtAm = glm::vec3(0.00f, 0.080f, 0.477f);
            *wtDf = glm::vec3(0.0f, 0.584f, 0.846f);
            *wtSp = glm::vec3(0.340f, 0.340f, 0.340f);
            *wtSh = 100.0f;;;

            dA->size = 1.0f;
            dA->direction = glm::vec2(-1.0f, 1.0f);
            dA->speed = 0.006f;
            dA->strenght = 0.823f;
            dA->color = glm::vec3(0.413f, 0.562f, 1.00f);
            dA->discard = glm::vec2(0.139f, 0.333f);
            dA->inside = true;

            dB->size = 18.877f;
            dB->direction = glm::vec2(0.629f, 0.507f);
            dB->speed = 0.010f;
            dB->strenght = 1.994f;
            dB->color = glm::vec3(0.734f, 0.893f, 1.0f);
            dB->discard = glm::vec2(0.025f, 0.196f);
            dB->inside = true;

            dC->size = 11.406f;
            dC->direction = glm::vec2(-1.0f, -0.629f);
            dC->speed = 0.018f;;
            dC->strenght = 0.285f;
            dC->color = glm::vec3(0.210f, 0.616f, 1.0f);
            dC->discard = glm::vec2(0.1f, 0.259f);
            dC->inside = false;

            *cn = 53.515f;
            *az = 9.109f;

            *la = 0.115f;
            *ld = 0.833f;
            *ls = 0.756f;
        }
    }

    void config3(float* gr, glm::vec4* wA, glm::vec4* wB, glm::vec4* wC, glm::vec3* wtClr, glm::vec3* wtAm, glm::vec3* wtDf, glm::vec3* wtSp, float* wtSh,
        displace* dA, displace* dB, displace* dC, float* cn, float* az, float* la, float* ld, float* ls) {
        if (ImGui::Button("Configuration 3"))
        {
            *gr = 9.8f;
            *wA = glm::vec4(1.0f, 1.0f, 0.5f, 10.0f);
            *wB = glm::vec4(0.1f, 1.0f, 0.3f, 4.0f);
            *wC = glm::vec4(0.4f, 0.2f, 0.7f, 2.0f);

            *wtClr = glm::vec3(1.0f, 0.409f, 0.3f);
            *wtAm = glm::vec3(0.049f, 0.049f, 0.628f);
            *wtDf = glm::vec3(0.688f, 0.068f, 0.0f);
            *wtSp = glm::vec3(1.0f, 0.612f, 0.00f);
            *wtSh = 41.0f;

            dA->size = 12.618f;
            dA->direction = glm::vec2(1.0f);
            dA->speed = 0.037f;
            dA->strenght = 1.0f;
            dA->color = glm::vec3(1.0f);
            dA->discard = glm::vec2(0.19f, 0.5f);
            dA->inside = true;

            dB->size = 22.107f;
            dB->direction = glm::vec2(-1.0f);
            dB->speed = 0.011f;
            dB->strenght = 1.930f;
            dB->color = glm::vec3(1.0f);
            dB->discard = glm::vec2(0.2f, 0.8f);
            dB->inside = true;

            dC->size = 6.359f;
            dC->direction = glm::vec2(-0.686f, 0.429f);
            dC->speed = 0.039f;
            dC->strenght = 0.823f;
            dC->color = glm::vec3(1.0f);
            dC->discard = glm::vec2(0.165f, 0.392f);
            dC->inside = false;

            *cn = 79.703f;
            *az = 0.80f;

            *la = 0.310f;
            *ld = 0.551f;
            *ls = 0.842f;;
        }
    }

    void config4(float* gr, glm::vec4* wA, glm::vec4* wB, glm::vec4* wC, glm::vec3* wtClr, glm::vec3* wtAm, glm::vec3* wtDf, glm::vec3* wtSp, float* wtSh,
        displace* dA, displace* dB, displace* dC, float* cn, float* az, float* la, float* ld, float* ls) {
        if (ImGui::Button("Configuration 4"))
        {
            *gr = 1.234;
            *wA = glm::vec4(0.571f, -0.057f, 0.076f, 12.665f);
            *wB = glm::vec4(-0.457f, -0.314f, 0.475f, 14.100f);
            *wC = glm::vec4(-0.371f, 0.486f, 0.215f, 15.070f);

            *wtClr = glm::vec3(1.0f, 0.199f, 0.0f);
            *wtAm = glm::vec3(0.921f, 0.384f, 0.165f);
            *wtDf = glm::vec3(0.813f, 0.375f, 0.137f);
            *wtSp = glm::vec3(0.159f, 0.133f, 0.006f);
            *wtSh = 100.0f;

            dA->size = 2.523f;
            dA->direction = glm::vec2(-1.0f, 1.0f);
            dA->speed = 0.013f;
            dA->strenght = 3.07f;
            dA->color = glm::vec3(1.0f, 0.901f, 0.352f);
            dA->discard = glm::vec2(0.025f, 0.259f);
            dA->inside = true;

            dB->size = 4.946f;
            dB->direction = glm::vec2(-0.514f, 0.371f);
            dB->speed = 0.013f;
            dB->strenght = 1.962f;
            dB->color = glm::vec3(1.0f, 0.839f, 0.00f);
            dB->discard = glm::vec2(0.057f, 0.589f);
            dB->inside = true;

            dC->size = 3.532f;
            dC->direction = glm::vec2(-0.171f, 0.4f);
            dC->speed = 0.014f;
            dC->strenght = 2.722f;
            dC->color = glm::vec3(1.0f, 0.563f, 0.00f);
            dC->discard = glm::vec2(0.247f, 0.576f);
            dC->inside = false;

            *cn = 53.515f;
            *az = 9.189f;

            *la = 0.115f;
            *ld = 0.833f;
            *ls = 0.756f;
        }
    }

    void endRender() {
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
    }

    void destroy() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

};