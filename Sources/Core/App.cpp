#pragma once 

#include <glad/glad.h>

#include "App.h"
#include "Mesh.h"
#include "../RayHit/Hit.h"
#include "../Helper/DebugLine.h"
#include "../Helper/Utils.h"
#include "../RayHit/BVH.h"
#include "../RayHit/HitCollector.h"

#include <iostream>
#include <glm/glm.hpp>
#include <limits>
#include <thread>
#include <vector>
#include <unordered_set>
#include <random>
#include <algorithm>

#include "External/imgui/imgui.h"
#include "External/imgui/imgui_impl_glfw.h"
#include "External/imgui/imgui_impl_opengl3.h"

/**
* Main application class
* Handles initialization, update, rendering, and shutdown
* 
* This is the main rendering class. 
* Initializes Depth of Field (DoF) rendering parameters: focus distance, aperture, sample number...
* Initializes the Scene and launches BVH
* Update Scene to match camera movement
* Handle DoF rendering with multiple threads
*/


// ---------- CONSTANTS -------------
// Camera
const glm::vec3 CAM_POS = glm::vec3(0.f, 10.f, 3.f);
const float CAM_FOV = 60.0f;
const float CAM_NEARPLANE = 0.01f;
const float CAM_FARPLANE = 500.0f;
const float CAM_APERTURE = 0.05f;
const float CAM_FOCUSDIST = 1.65f;

// Depth of Field
const int RESOLUTION = 480;
const int NUM_SAMPLES = 16;  // number of rays per pixel

bool App::init()
{
    m_camera = Camera();
    m_light = Light();
	m_gizmo = Gizmo();
   
    m_shader.load("Resources/Shaders/RasterVert.glsl", "Resources/Shaders/RasterFrag.glsl");
    m_screenShader.load("Resources/Shaders/ScreenQuadVert.glsl", "Resources/Shaders/ScreenQuadFrag.glsl");

    // CAMERA PARAMETERS
    m_camera.fov = CAM_FOV;
    m_camera.setPosition(CAM_POS);
    m_camera.nearPlane = CAM_NEARPLANE;
    m_camera.farPlane = CAM_FARPLANE;
    m_camera.setAperture(CAM_APERTURE);
    m_camera.focusDistance = CAM_FOCUSDIST;

    // PARAMETERS INIT
    m_height = RESOLUTION;
    m_width = RESOLUTION;

    m_numSamples = NUM_SAMPLES;
    m_threadPool = std::make_unique<ThreadPool>();

    // SCENE INIT
    initScene();

    std::cout << "width: " << m_width << " height: " << m_height << "\n";
    std::cout << "Camera position: " << m_camera.getPosition().x << " "<< m_camera.getPosition().y << " "<< m_camera.getPosition().z << "\n";
	std::cout << "Camera Aperture: " << m_camera.getAperture() << "\n";
	std::cout << "Focus Distance: " << m_camera.focusDistance << "\n\n";
    Utils::printHelp();

	// DoF framebuffer
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,m_width, m_height, 0,GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// init fullscreen quad VAO
    float quadVertices[] = {
        // pos        // uv
        -1.f, -1.f,   0.f, 0.f,
         1.f, -1.f,   1.f, 0.f,
         1.f,  1.f,   1.f, 1.f,

        -1.f, -1.f,   0.f, 0.f,
         1.f,  1.f,   1.f, 1.f,
        -1.f,  1.f,   0.f, 1.f
    };

    GLuint VBO;
    glGenVertexArrays(1, &m_fullscreenQuadVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(m_fullscreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    return true;
}


// Load meshes of the scene and build BVH
void App::initScene()
{
    m_meshes.clear();

	// ------ Backwall ------
	auto backwall = std::make_shared<Mesh>();
	backwall->createQuad(20.f);
	backwall->upload();
	backwall->setRotation({glm::half_pi<float>(),0.f, 0.f });
	backwall->setTranslation({ 0.f, 10.f, -10.f });
	backwall->setScale(1.f);
	backwall->color() = { 0.2f, 0.6f, 0.9f };  // sky blue
	m_meshes.push_back(backwall);

    // ------ Background Elements -------
    auto sphere = std::make_shared<Mesh>();
    sphere->loadOFF("Resources/Models/sphere_low_res.off");
    sphere->upload();

    glm::vec3 ball_center = glm::vec3(0.0f, 7.0f, -8.0f);

    // To randomize background sphere position
    //uint32_t seed = std::random_device{}();  
    //std::cout << "Current seed for random generation: " << seed << "\n";

    uint32_t seed = 1077708736u;
    std::mt19937 rng(seed);

    // set random positions withing specific range 
    std::uniform_real_distribution<float> dx(-5.f, 5.f);
    std::uniform_real_distribution<float> dy(3.f, 8.f);
    std::uniform_real_distribution<float> dz(-12.f, -6.f);
    std::uniform_real_distribution<float> scaleDist(0.2f, 0.6f);
    const int ball_nb = 20;

    for (int i = 0; i < ball_nb; ++i)
    {
        auto inst = std::make_shared<Mesh>(*sphere);

        glm::vec3 pos(
            dx(rng),
            dy(rng),
            dz(rng)
        );

        inst->setTranslation(pos);
        inst->setScale(scaleDist(rng));

        std::uniform_real_distribution<float> hue(0.f, 1.f);        // random color hue
        glm::vec3 color = Utils::HSVtoRGB(hue(rng), 0.8f, 0.9f);
        inst->setColor(color);

        m_meshes.push_back(inst);
    }


    // ------ Floor ------
    auto floor = std::make_shared<Mesh>();

    floor->createQuad(20.f);
    floor->upload();

    floor->setRotation({ 0.0f, 0.f, 0.f });
    floor->setTranslation({ 0.f, 0.f, 0.f });
    floor->setScale(1.f);
    floor->color() = { 0.6f, 0.9f, 0.6f };  // Green
    m_meshes.push_back(floor);


    // ------ Penguins ------
    auto mesh = std::make_shared<Mesh>();
    mesh->loadOFF("Resources/Models/pingu.off");
    mesh->setRotation({ glm::half_pi<float>(), 0.f, 0.f });
    mesh->upload();

    const int penguin_nb = 5;

    glm::vec3 center = glm::vec3(-2.0f, 0.0f, 2.0f);
    float spacing = 1.0f;

    for (int i = 0; i < penguin_nb; ++i)
    {
        auto inst = std::make_shared<Mesh>(*mesh);
        glm::vec3 offset = center + glm::vec3(-spacing / 5.0f + static_cast<float>(i) * spacing / 2.0f, 0.0f, -static_cast<float>(i) * spacing);
        inst->setTranslation(offset);

		// Color based on HSV 
        float t = float(i) / (penguin_nb - 1);
		float h = 0.80f + 0.4f * t;     // hue varying
		float s = 0.8f;                 // saturation fixed
		float v = 1.0f - 0.2f * t;      // value (bright to slightly darker)
        inst->setColor(Utils::HSVtoRGB(h, s, v));

        inst->setScale(1.0f);
        m_meshes.push_back(inst);
    }

    // BVH initialization in world space
	std::cout << "Building BVH...\n";
	m_bvh = std::make_unique<BVH>(m_meshes);
    std::cout << "BVH built.\n";
}


void App::render()
{
    glEnable(GL_DEPTH_TEST);

    if (m_useDoF)
    {
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        renderDoF();
        displayFramebuffer();
        return;
    }
    renderRasterized();
 }


void App::renderRasterized() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 V = m_camera.viewMatrix();
    glm::mat4 P = m_camera.projectionMatrix(m_width, m_height);

    glm::vec3 lightPosVS = glm::vec3(V * glm::vec4(m_light.position, 1.0f));

    m_shader.use();

    // Camera uniforms
    m_shader.setMat4("uView", V);
    m_shader.setMat4("uProjection", P);

    // Light uniforms
    m_shader.setVec3("uLightPosVS", lightPosVS);
    m_shader.setVec3("uLightColor", glm::vec3(1.0f));
    m_shader.setFloat("uLightIntensity", m_light.intensity);
    m_shader.setFloat("uShininess", m_light.shininess);

    for (auto& mesh : m_meshes)
    {
        glm::mat4 M = mesh->computeTransformMatrix();

        m_shader.setMat4("uModel", M);
        m_shader.setVec3("uMeshColor", mesh->color());

        mesh->draw();
    }

    glm::mat4 VP = P * V;

    // ---------- Gizmo ----------
    if (m_gizmo.isEnabled())
    {
        m_gizmo.draw(VP);
    }

    // ---------- Debug rays (grid) ----------

    m_debug.clear();
    if (m_showFocusPlane) drawFocusPlane();
    m_debug.draw(VP);
}

void App::renderDoF()
{
    const int minSamples = 4;         // Minimum of sampels before a pixel can be considered as convergent (currently disabled due to artefacts)
    const int raysPerFrame = 20000;   // Number of rays between render frame (shared by threads)

    // Initialize accumulation buffers on first frame or after a reset
    if (m_accumBuffer.empty()) resetAccumulation();

    int totalActive = static_cast<int>(m_activePixels.size());  // number of pixels not yet converged (active list size)
    // Stop rendering if all pixels are done or max sample reached
    if (totalActive == 0 || m_currentSample >= m_numSamples) return;

    // if all pixels processed for current sample, go to next global sample
    if (m_currentPixel >= totalActive) {
        m_currentPixel = 0;
        ++m_currentSample;
        return;
    }

    // How many pixels we'll process this frame (cap with raysPerFrame)
    int toProcess = std::min(raysPerFrame, totalActive - m_currentPixel);
    if (toProcess <= 0) return;

	// Create a local copy of the active pixel indices to process to avoid concurrent modification
    // (threads will access this list instead of main active list, which may be modified by convergence)
    std::vector<int> pixelsToProcess;
    pixelsToProcess.reserve(toProcess);

    // Decide workload per thread (base + remainder for first threads)
    int start = m_currentPixel;
    int end = start + toProcess;

    for (int i = start; i < end; ++i)
        pixelsToProcess.push_back(m_activePixels[i]);

    // Process in small block for cache perf
	const int BLOCK_SIZE = 32; 

	// Account for convergence during processing: if a pixel converges while we're processing, we can skip it in the remaining threads (currently disabled)
    std::mutex convergedMutex;
    //std::vector<int> convergedPixels;

    // Dispatch blocks to the thread pool
    for (int blockStart = 0; blockStart < toProcess; blockStart += BLOCK_SIZE) {

        int blockEnd = std::min(blockStart + BLOCK_SIZE, toProcess); // end of this block (exclusive)

        m_threadPool->enqueue([this, &pixelsToProcess,blockStart, blockEnd, minSamples, /*&convergedPixels,*/ &convergedMutex]() {

            for (int i = blockStart; i < blockEnd; ++i) {
                int pixelIdx = pixelsToProcess[i];
                PixelStats& p = m_pixels[pixelIdx];

                int x = pixelIdx % m_width;
                int y = pixelIdx / m_width;
                int reversed_y = m_height - 1 - y; // to flip rendered image (pinhole effect)

                // generate ray (accounting for aperture)
                Ray pinhole = m_camera.pinholeRay(x, reversed_y, m_width, m_height);
                Ray ray = (m_camera.getAperture() > 0.f)
                    ? m_camera.thinLensRay(pinhole, p.samples, m_numSamples)
                    : pinhole;

                // Trace ray with depth-of-field and N-buffer visibility
                glm::vec3 color = traceDoFRay(ray);

                // Online mean and variance update using Welford's algorithm 
                // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
                p.samples++;

                // Update pixel color
                glm::vec3 delta = color - p.color;
                p.color += delta / static_cast<float>(p.samples);
                m_framebuffer[pixelIdx] = p.color;

                // ---- CONVERGENCE (not used, creates artefacts) ----
                //p.variance += delta * (color - p.color); // (xn - mean_n-1) * (xn - mean_n) -> Welford's method for variance 
                //if (p.samples >= minSamples) {
                //    glm::vec3 var = p.variance / static_cast<float>(std::max(1, p.samples - 1));
                //    float lumVar = glm::dot(var, glm::vec3(0.2126f, 0.7152f, 0.0722f)); // luminance variance (Y taken from Rec. 709, https://en.wikipedia.org/wiki/SRGB#Primaries)
                //
                //    // if converged (luminance variance below threshold)
                //    if (lumVar < 1e-9f) {
                //        //// collect this pixel idx for main thread to mark as converged
                //        std::lock_guard<std::mutex> lock(convergedMutex);
                //        convergedPixels.push_back(pixelIdx);
                //    }
                //}
            }
        });
    }

    m_threadPool->wait();

    // ---- CONVERGENCE (not used, creates artefacts) ----
    // merge converged lists into a set and mark pixels as converged
    //for (int pid : convergedPixels) {
    //    if (!m_pixels[pid].converged) {
    //        m_pixels[pid].converged = true;
    //    }
    //}
    // 
    // compact active list
    //int write = 0;
    //for (int i = 0; i < (int)m_activePixels.size(); ++i) {
    //    int pid = m_activePixels[i];
    //    if (!m_pixels[pid].converged) {
    //        m_activePixels[write++] = pid;
    //    }
    //}
    //m_activePixels.resize(write);

    // advance cursor for next frame
    m_currentPixel += toProcess;

    // if the active list for this sample is done, go to next global sample
    if (m_currentPixel >= static_cast<int>(m_activePixels.size())) {
        m_currentPixel = 0;
        ++m_currentSample;
    }

    displayFramebuffer();
}

// The use of the second ray hit does not show any convincing results, so i commented it 
glm::vec3 App::traceDoFRay(const Ray& ray)
{
    RayHits hits;
    HitCollector collector(ray, m_meshes, hits);
    m_bvh->traverse(ray, collector);

    if (hits.count == 0)
        return glm::vec3(0.0f);

    glm::vec3 color(0.0f);
    //float totalWeight = 0.0f;

    for (int i = 0; i < hits.count; ++i)
    {
        const Hit& h = hits.hits[i];

        // ---- CoC ----
        //float coc = computeCoC(
        //    h.t,
        //    m_camera.focusDistance,
        //    m_camera.getAperture()
        //);

        // ---- SHADING ----
        glm::vec3 N = glm::normalize(h.normal);
        if (glm::dot(N, ray.direction) > 0.0f)
            N = -N;

        glm::vec3 L = glm::normalize(m_light.position - h.position);
        float diffuse = glm::max(glm::dot(N, L), 0.0f);
        glm::vec3 shaded = h.color * diffuse;

        // ---- EARLY EXIT (perfect focus) ----
        if (i == 0 /*&& coc < 1e-2f*/)
            return shaded;

        //// ---- Contribution ----
        
        //float weight = layerAttenuation(i);

        //color += shaded * weight;

        //totalWeight += weight;
    }

    return color /*/ totalWeight*/;
}

// Computes Circle of Confusion 
float App::computeCoC(float z, float focusDist, float aperture)
{
    return aperture * abs(z - focusDist) / z;
}

// Add small contribution to additional layer (heuristic)
float App::layerAttenuation(int layer)
{
    if (layer == 0) return 0.95f;
    if (layer == 1) return 0.05f;
    return 0.0f;
}


// Reset DoF rendering
void App::resetAccumulation()
{
	m_currentPixel = 0;

    m_pixels.assign(m_width * m_height, 
        PixelStats{glm::vec3(0.f), glm::vec3(0.f), 0, false});


	// every pixel is active at the beginning
    m_activePixels.clear();
    m_activePixels.reserve(m_width * m_height);

    for (int i = 0; i < m_width * m_height; ++i)
        m_activePixels.push_back(i);

    m_accumBuffer.assign(m_width * m_height, glm::vec3(0.0f));
    m_framebuffer.assign(m_width * m_height, glm::vec3(0.0f));
    m_currentPixel = 0;
    m_currentSample = 0;
}

void App::displayFramebuffer()
{
    // Upload framebuffer to texture
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
        m_width, m_height,
        GL_RGB, GL_FLOAT,
        m_framebuffer.data());

    // Render fullscreen quad with the texture
    m_screenShader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    m_screenShader.setInt("uTexture", 0);

    glBindVertexArray(m_fullscreenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // to cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


// ------------------- USER INTERFACE ----------------------

// Handles click-based focus control
void App::setFocusFromScreen(float mouseX, float mouseY, int winW, int winH)
{
    int px = int(mouseX * m_width  / winW);
    int py = int(mouseY * m_height / winH);

    // clamp to screen bounds
    px = glm::clamp(px, 0, m_width  - 1);
    py = glm::clamp(py, 0, m_height - 1);

    setFocusFromPixel(px, py);
}

// Called by setFocusFromScreen
void App::setFocusFromPixel(int x, int y)
{
    // To set focus distance, casts a pinhole ray from the clicked piel and finds the closest hit,
    // the focus distance is updated to hit distance and DoF renderer reset
    if (!m_bvh) return;

    Ray ray = m_camera.pinholeRay(x, y, m_width, m_height);

    RayHits hits;
    HitCollector collector(ray, m_meshes, hits);
    m_bvh->traverse(ray, collector);

    if (hits.count == 0) return;

    m_camera.focusDistance = hits.hits[0].t;

    resetAccumulation();
}

// Visualize camera focus plane as a grid
void App::drawFocusPlane()
{
    const float size = 8.0f;
    const int grid = 30;

    glm::vec3 center = m_camera.getPosition() + m_camera.getForward() * m_camera.focusDistance;
    glm::vec3 right = m_camera.getRight() * size;
    glm::vec3 up = m_camera.getUp() * size;

    glm::vec3 yellow = { 1.0f, 1.0f, 0.0f }; 

    // --- grid ---
    for (int i = 0; i <= grid; ++i)
    {
        float t = float(i) / grid * 1.0f - 0.5f;

        glm::vec3 a = center + right * t - up;
        glm::vec3 b = center + right * t + up;
        glm::vec3 c = center - right + up * t;
        glm::vec3 d = center + right + up * t;

        m_debug.addLine(a, b, yellow);
        m_debug.addLine(c, d, yellow);
    }
}

// Called pressing G key
void App::toggleGizmo()
{
    if (!m_gizmo.isEnabled())
        std::cout << "Gizmo enabled.\n";
    else
        std::cout << "Gizmo disabled.\n";
    m_gizmo.toggle();
}

// Called pressing F key
void App::toggleDoF()
{
    m_useDoF = !m_useDoF;

    if (m_useDoF)
    {
        std::cout << "Depth of Field enabled.\n";
        m_framebuffer.clear();
    }
    else
    {
        std::cout << "Depth of Field disabled.\n";
    }
}

// Called pressing ESC key
void App::shutdown()
{
    std::cout << "Shutting down application.\n";
}


// --------------------- Visualization ------------------------------

 // Visualize depth peeling (check Hit::MAX_HITS values before using)
glm::vec3 App::visualizeLayers(const Ray& ray, int layer)
{
    RayHits hits;
    HitCollector collector(ray, m_meshes, hits);
    m_bvh->traverse(ray, collector);

    if (hits.count == 0 || hits.count <= layer)
        return glm::vec3(0.0f); // fond noir

    glm::vec3 color(0.0f);
    const Hit& h = hits.hits[layer];

    // different color for each layer
    switch (layer)
    {
    case 0:
        color += glm::vec3(0.0f, 0.0f, 1.0f) * 0.9f;
        break;
    case 1:
        color += glm::vec3(1.0f, 0.0f, 0.0f) * 0.9f;
        break;
    case 2:
        color += glm::vec3(0.0f, 1.0f, 0.0f) * 0.9f;
        break;
    case 3:
        color += glm::vec3(1.0f, 1.0f, 0.0f) * 0.9f;
        break;
    case 4:
        color += glm::vec3(0.0f, 1.0f, 1.0f) * 0.9f;
        break;
    default: return glm::vec3(0.0f);
    }

    return color / float(hits.count);
}

glm::vec3 App::debugCoC(const Ray& ray)
{
    RayHits hits;
    HitCollector collector(ray, m_meshes, hits);
    m_bvh->traverse(ray, collector);

    if (hits.count == 0)
        return glm::vec3(0);

    float coc = computeCoC(
        hits.hits[0].t,
        m_camera.focusDistance,
        m_camera.getAperture()
    );

    float maxCoC = 0.05f;
    float v = std::clamp(coc / maxCoC, 0.0f, 1.0f);
    return glm::vec3(v, 0.0f, 1.0f - v);
}

