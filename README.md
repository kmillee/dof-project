# Depth of Field Rendering and Focus Control

This project implements a **depth-of-field (DoF) renderer** using a thin lens camera model and ray tracing.
It simulates lens blur by sampling rays across an aperture and computing their contributions using a CPU-based rendering pipeline. This projet is based on "Real-Time Lens Blur Effects and Focus Control" by Lee et al.

It was submitted as the final project of the Advanced Computer Graphics class.

---

# Requirements

This project use the following libraries: GLFW, GLAD, GLM, stb_image, ImGui.

All external libraries are expected to be located inside the `External/` directory.

---

# Project Structure

```
LensSim/
│
├── Sources/
│   ├── Core/
│   ├── RayHit/
│   ├── Helper/
│   └── Main.cpp
│
├── External/
│   ├── glad/
│   ├── glfw/
│   ├── glm/
│   ├── stb_image/
│   └── imgui/
│
├── Resources/
└── CMakeLists.txt
```

---

# Build Instructions 

## 1. Clone the repository

```bash
git clone <repo-url>
cd LensSim
```

## 2. Make sure External dependencies exist

Ensure the following folders exist (should already be here):

```
External/glad
External/glfw
External/glm
External/stb_image
External/imgui
```

If missing, re-add them before configuring CMake.

---

# **Build Project**

```bash
cmake -B build
cmake --build build --config Release
```

Executable will be located in:

```
build/Release/
```

---

# Running the Project

After building:

* Launch the executable
* Wait for BVH to build
* The renderer progressively accumulates samples
