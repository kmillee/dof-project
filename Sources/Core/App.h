#pragma once
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Light.h"
#include "../RayHit/BVH.h"
#include "../Helper/Gizmo.h"
#include "../Helper/DebugLine.h"
#include "../Helper/RaySim.h"
#include "../Helper/ThreadPool.h"

#include <vector>
#include <memory>

struct PixelStats {
	glm::vec3 color;        // accumulated mean color
	glm::vec3 variance;     // accumulated variance
	int samples = 0;        // number of samples accumulated
	bool converged = false; // convergence flag
};

// ============================================================
// Main application class
// Handles initialization, update, rendering, and shutdown
// ============================================================
class App {
public:
	// ---- Lifecycle ----
    bool init();
    void render();
    void shutdown();

	void renderRasterized();

	// ---- Depth of Field rendering -----
    void renderDoF();
	glm::vec3 traceDoFRay(const Ray& tay);
	void displayFramebuffer();
	float computeCoC(float z, float focusDist, float aperture);
	float layerAttenuation(int layer);

	// ---- Scene interaction ----
	void initScene();
    void toggleGizmo();
	void toggleDoF();
	void resetAccumulation();
	void drawFocusPlane();
	void setFocusFromScreen(float mouseX, float mouseY, int winW, int winH);

	// ------------ GETTERS & SETTERS -----------
	Camera& camera() { return m_camera; }
	Gizmo& gizmo() { return m_gizmo; }
	int& width() { return m_width; }
	int& height() { return m_height; }
	int& currentPixel() { return m_currentPixel; }
	bool& renderingDoF() { return m_useDoF; }
	bool& showFocusPlane() { return m_showFocusPlane; }
	int& numSamples() { return m_numSamples;}	
	int& currentSample() { return m_currentSample; }

private:

	// ---- Internal helpers ----
	void setFocusFromPixel(int x, int y);
	glm::vec3 visualizeLayers(const Ray& ray, int i);
	glm::vec3 debugCoC(const Ray& ray);


	// ---- GPU resources ----
	Shader m_shader;
	Shader m_screenShader;
	GLuint m_fullscreenQuadVAO = 0;
	GLuint m_colorTexture = 0;

	// ---- Scene data ----
	std::vector<std::shared_ptr<Mesh>> m_meshes;
	Camera m_camera;
	Light m_light;
	std::unique_ptr<BVH> m_bvh;

	// ---- Framebuffer / Progressive DoF buffers ----
	int m_width = 0;
	int m_height = 0;
	int m_numSamples = 1;
	int m_currentSample = 0;
	int m_currentPixel = 0;

	std::vector<glm::vec3> m_framebuffer;
	std::vector<PixelStats> m_pixels;
	std::vector<glm::vec3> m_accumBuffer;

	std::vector<int> m_activePixels;

	// ---- Flags ----
	bool m_useDoF = false;
	bool m_dofFinished = false;
	bool m_showFocusPlane = false;

	// ---- Helpers & debug ----
	Gizmo m_gizmo;
	DebugLine m_debug;

	// ---- Multithreading ----
	std::unique_ptr<ThreadPool> m_threadPool;
};
