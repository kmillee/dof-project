#version 410 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
//uniform mat4 projectionMat, modelViewMat, normalMat; // Uniform variables, set from the CPU-side main program

out vec3 fPosition;
out vec3 fNormal;

void main()
{
    mat4 MV = uView * uModel;
    vec4 posVS = MV * vec4(vPosition, 1.0);

    fPosition = posVS.xyz;
    fNormal = mat3(transpose(inverse(MV))) * vNormal;

    gl_Position = uProjection * posVS;
}

