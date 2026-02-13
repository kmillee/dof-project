#pragma once

#include <glad/glad.h>
#include <iostream>

#include "Window.h"
#include "App.h"
#include "../Helper/Utils.h"
#include "External/imgui/imgui.h"


Window::Window(int w, int h, const std::string& title)
    : m_width(w), m_height(h)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    m_window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);

	glfwSetFramebufferSizeCallback(m_window, framebufferCallback);  // to handle window resizing
	glfwSetCursorPosCallback(m_window, mouseCallback);              // to handle camera orbit
	glfwSetMouseButtonCallback(m_window, mouseButtonCallback);      // to handle camera mode (later focus control)
	glfwSetScrollCallback(m_window, scrollCallback);                // to handle zooming

	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	setUserPointer(this);
}



Window::~Window()
{
    glfwTerminate();
}



// Process input events (e.g., keyboard)
void Window::processInput() {
    if (!m_camera) return;

    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        return;

    float speed = 1.0f;

    if (glfwGetKey(m_window, GLFW_KEY_H) == GLFW_PRESS)
    {
        Utils::printHelp();
    }

    if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        m_camera->orbit(-speed * deltaTime(), 0.f);
        //std::cout << "Left key pressed\n";
    }
    if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        m_camera->orbit(speed * deltaTime(), 0.f);
    }
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) {
        m_camera->orbit(0.f, speed * deltaTime());
    }
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        m_camera->orbit(0.f, -speed * deltaTime());
    }

    // zoom avec W/S
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
        m_camera->zoom(-speed * deltaTime());
		//std::cout << "zoom in\n";
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
        m_camera->zoom(speed * deltaTime());
		//std::cout << "zoom out\n";
    }

	// gizmo toggle with G
    static bool gWasPressed = false;
    bool gPressed = (glfwGetKey(m_window, GLFW_KEY_G) == GLFW_PRESS);

    if (gPressed && !gWasPressed)
    {
		m_app->toggleGizmo();
    }

    gWasPressed = gPressed;

	// DoF rendering toggle with F
	static bool fWasPressed = false;
	bool fPressed = (glfwGetKey(m_window, GLFW_KEY_F) == GLFW_PRESS);
	if (fPressed && !fWasPressed)
	{
		m_app->toggleDoF();
	}
	fWasPressed = fPressed;

    // Focus Control holding shift
	m_focusControl = (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    // escape
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);
	// Update delta time
    float now = (float)glfwGetTime();
    m_deltaTime = now - m_lastTime;
    m_lastTime = now;
    
}


void Window::setUserPointer(void* ptr)
{
    glfwSetWindowUserPointer(m_window, ptr);
}

// ------------ CALLBACKS -----------

bool Window::imguiWantsInput() const
{
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

void Window::framebufferCallback(GLFWwindow*, int w, int h)
{
    glViewport(0, 0, w, h);
}

void Window::mouseCallback(GLFWwindow* win, double x, double y)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
    if (!window) return;

    // Check first if ImGui wants to capture the mouse
    if (window->imguiWantsInput())
        return;

    if (!window->m_leftMouse && !window->m_rightMouse) return;

    if (window->m_firstMouse)
    {
        window->m_lastMouseX = x;
        window->m_lastMouseY = y;
        window->m_firstMouse = false;
        return;
    }


    float dx = float(x - window->m_lastMouseX);
    float dy = float(y - window->m_lastMouseY);

    window->m_lastMouseX = x;
    window->m_lastMouseY = y;

    if (window->m_rightMouse || !window->m_focusControl)
        window->m_camera->orbit(dx, dy);

    if (window->m_leftMouse)
        window->m_camera->translate(dx, dy);
}

void Window::scrollCallback(GLFWwindow* win, double, double yoffset)
{

    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
    if (!window) return;

    // Check first if ImGui wants to capture the mouse
    if (window->imguiWantsInput())
        return;

	auto* camera = window->m_camera;
    if (!camera) {
		std::cerr << "No camera set for window scroll callback.\n";
        return;
    }
    window->m_camera->zoom((float)yoffset * 2.f);
}

void Window::mouseButtonCallback(GLFWwindow* win, int button, int action, int)
{
    auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
    if (!window) return;

    // Check first if ImGui wants to capture the mouse
    if (window->imguiWantsInput())
        return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        window->m_rightMouse = (action == GLFW_PRESS);
        window->toggleCameraControl(window->m_rightMouse);
        window->m_firstMouse = true;

        if (window->m_focusControl)
        {
            // ---- DOF focus picking ----
            if (window->m_app)
            {
				std::cout << "Setting focus from screen coordinates...\n";
                double x, y;
                glfwGetCursorPos(win, &x, &y);

                int w, h;
                glfwGetFramebufferSize(win, &w, &h);

                window->m_app->setFocusFromScreen(
                    float(x), float(y),
                    w, h
                );
            }
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        window->m_leftMouse = (action == GLFW_PRESS);
        window->m_firstMouse = true;
    }
}


// ------------ FLAGS ---------------
void Window::toggleCameraControl(bool enabled)
{
    m_cameraControl = enabled;

    if (enabled)
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool Window::cameraControlEnabled() const
{
    return m_cameraControl;
}



