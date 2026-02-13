#pragma once

#include "Camera.h"
#include "App.h"

#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window(int w, int h, const std::string& title);
    ~Window();

    // Swap the front and back buffers (double buffering)
    void swapBuffers() { glfwSwapBuffers(m_window); }
    void pollEvents() { glfwPollEvents(); }

    void processInput();

	// Set a user pointer associated with the window (camera, app, etc.)
    void setUserPointer(void* ptr);
    void toggleCameraControl(bool enabled);

    // ---------- FLAGS --------------
    bool cameraControlEnabled() const;
    bool isValid() const { return m_window != nullptr; }
    bool shouldClose() const { return glfwWindowShouldClose(m_window); }

	// ------------ GETTERS & SETTERS -----------
    int width() const { return m_width; }
	int height() const { return m_height; }
    float deltaTime() const { return m_deltaTime; }
	GLFWwindow* glfwWindow() const { return m_window; }
    GLFWwindow* handle() const { return m_window; }

    void Window::setCamera(Camera* camera) { 
        if (!camera) {
            std::cerr << "Warning: trying to set a null camera to the window.\n";
        }
        else {
            m_camera = camera;
        }
    }
    void setApp(App* app) { m_app = app; }
   
private:
    // ----- FLAGS ------
    bool m_cameraControl = false;
    bool imguiWantsInput() const;

    // ----- CALLBACKS -----
    static void framebufferCallback(GLFWwindow*, int, int);
    static void mouseCallback(GLFWwindow*, double, double);
    static void scrollCallback(GLFWwindow*, double, double);
	static void mouseButtonCallback(GLFWwindow*, int, int, int);

    GLFWwindow* m_window = nullptr;
	Camera* m_camera = nullptr;
    App* m_app = nullptr;

    int m_width, m_height;
    float m_lastTime = 0.f;
    float m_deltaTime = 0.f;
    
    // ----- MOUSE CONTROLS ------
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_firstMouse = true;
    bool m_leftMouse = false;
    bool m_rightMouse = false;
    bool m_focusControl = false;

};
