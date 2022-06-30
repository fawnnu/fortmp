#include "include.hpp"
#include "renderer.hpp"

#include "window.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

renderer::renderer()
{
    if (!glfwInit())
    {
        return;
    }
    
    // Always create window initially
    glfw_window = create_window("FortMP", 1280, 940);

    if (glfw_window == nullptr)
    {
        return;
    }
    
    while (!glfwWindowShouldClose(glfw_window))
    {
        // Render everything in the GLFW window
        render();
    }
}

GLFWwindow* renderer::create_window(std::string window_name, int window_height, int window_width)
{
    GLFWwindow* new_window = glfwCreateWindow(
        window_width,
        window_width,
        window_name.c_str(),
        nullptr,
        nullptr);

    if (new_window == nullptr)
    {
        // Window not created
        return nullptr;
    }

    glfwMakeContextCurrent(new_window);
    glfwSwapInterval(1);

    ImGui::CreateContext();

    if (!ImGui_ImplGlfw_InitForOpenGL(new_window, true) ||
        !ImGui_ImplOpenGL3_Init())
    {
        return nullptr;
    }
    
    return new_window;
}

void renderer::render()
{
    // Clear background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    // Start new frame
    ImGui::NewFrame();

    {
        // Render main window
        window::render(glfw_window);
    }

    // Render everything that has just been created
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(glfw_window);
    glfwPollEvents();
}