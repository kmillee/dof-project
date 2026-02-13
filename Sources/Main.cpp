#pragma once 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Core/App.h"
#include "Core/Window.h"
#include "External/imgui/imgui.h"
#include "External/imgui/imgui_impl_glfw.h"
#include "External/imgui/imgui_impl_opengl3.h"

#include <iostream>


bool initOpenGL()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    return true;
}

void drawImGui(App& app, Window& win)
{
    Camera& camera = app.camera();

    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // --- Stats ---
    ImGui::TextDisabled("Statistics");
    ImGui::Text("Application average %.2f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Rendered pixels: %d / %d", app.currentPixel(), app.width() * app.height());

    ImGui::ProgressBar(float(app.currentPixel()) / float(app.width() * app.height()),ImVec2(0, 0));

    ImGui::Separator();

    // --- Rendering ---
    ImGui::TextDisabled("Rendering");
    ImGui::Checkbox("Depth of Field", &app.renderingDoF());
    ImGui::Text("Accum samples: %d / %d", app.currentSample(), app.numSamples());
    ImGui::SliderInt("Samples", &app.numSamples(), 1, 32);

    ImGui::Separator();

    // --- Camera / DoF ---
    bool changed = false;

    if (ImGui::CollapsingHeader("Camera & DoF", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();
        ImGui::Checkbox("Show focus plane", &app.showFocusPlane());

        changed |= ImGui::SliderFloat("Aperture", &app.camera().getAperture(), 0.f, 0.1f);
        changed |= ImGui::SliderFloat("Focus Distance", &app.camera().focusDistance, 0.1f, 10.f);

        ImGui::Unindent();
    }

    if (changed) app.resetAccumulation(); // Reset rendering when modifying parameters

    ImGui::End();
}

int main()
{
    Window window(1200, 1200, "Lens Simulation");
    if (!window.isValid()) { return -1; }
    if (!initOpenGL()) { return -1; }

    App app; 
    if (!app.init()) return -1;

    window.setCamera(&app.camera());
    window.setApp(&app);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window.glfwWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --------------------------------------------------------
    // Main loop
    // --------------------------------------------------------
    while (!window.shouldClose())
    {
        float dt = window.deltaTime();

        window.pollEvents();

        // ---- ImGui ----
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        drawImGui(app, window);

		window.processInput(); // update input events + frame rate

        app.render();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
     
        window.swapBuffers();

    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    app.shutdown();
    return 0;
}
