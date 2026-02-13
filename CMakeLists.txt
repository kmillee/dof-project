cmake_minimum_required(VERSION 3.9)
project(LensSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP REQUIRED)

add_subdirectory(External)

add_executable(LensSim
    Sources/RayHit/AABB.h
    Sources/Core/Camera.h
    Sources/Core/Camera.cpp
    "Sources/Helper/Lens.h"
    "Sources/Helper/Lens.cpp"
    Sources/Core/Light.h
    Sources/Core/Mesh.h
    Sources/Core/Mesh.cpp
    Sources/Core/Shader.h
    Sources/Core/Shader.cpp
    Sources/Core/Transform.h
    Sources/Core/Window.h
    Sources/Core/Window.cpp

    Sources/Main.cpp
    Sources/Core/App.cpp
    Sources/Core/App.h
    Sources/RayHit/BVH.cpp
    Sources/RayHit/BVH.h
    Sources/RayHit/Ray.h
    Sources/RayHit/Hit.h
    Sources/Helper/Utils.h
    Sources/Helper/DebugLine.h
    Sources/Helper/DebugLine.cpp
    Sources/Helper/Gizmo.h
    Sources/Helper/Gizmo.cpp
    Sources/Helper/RaySim.h
    Sources/Helper/RaySim.cpp
    Sources/Helper/ThreadPool.h
    Sources/RayHit/TriangleVisitor.h
    Sources/RayHit/HitCollector.h
    Sources/RayHit/HitCollector.cpp

     # ---- IMGUI ----
    External/imgui/imgui.cpp
    External/imgui/imgui_draw.cpp
    External/imgui/imgui_widgets.cpp
    External/imgui/imgui_tables.cpp
    External/imgui/imgui_demo.cpp
    External/imgui/imgui_impl_glfw.cpp
    External/imgui/imgui_impl_opengl3.cpp
)

target_include_directories(LensSim PRIVATE
    Sources
    External/glm
    External/stb_image
)

target_link_libraries(LensSim PRIVATE
    glfw
    glad
    OpenMP::OpenMP_CXX
)


# Copy the shader files in the binary location. 
include_directories(${CMAKE_CURRENT_SOURCE_DIR} External/stb_image/) 
# include_directories(${CMAKE_CURRENT_SOURCE_DIR} External/imgui/) 
target_link_libraries(LensSim LINK_PRIVATE glad) 
target_link_libraries(LensSim LINK_PRIVATE glfw) 
target_link_libraries(LensSim LINK_PRIVATE glm) 
target_link_libraries(LensSim PRIVATE OpenMP::OpenMP_CXX) 
# Copy resources after build (otherswise the working directory needs to be changed) 
add_custom_command(TARGET LensSim POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Resources" "$<TARGET_FILE_DIR:LensSim>/Resources")