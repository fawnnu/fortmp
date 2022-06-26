#pragma once
#include <GLFW/glfw3.h>

class renderer
{
public:
    renderer();
    
    // Creates a new OpenGL window
    GLFWwindow* create_window(std::string window_name, int window_height, int window_width);

    // Called every frame to render
    void render();

private:
    GLFWwindow* glfw_window;
};
