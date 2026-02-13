#version 410 core

in vec3 fPosition;
in vec3 fNormal;

uniform vec3 uMeshColor;

// Light
uniform vec3 uLightPosVS;
uniform vec3 uLightColor;
uniform float uLightIntensity;
uniform float uShininess;

out vec4 FragColor;

void main()
{
    // Blinn Phong model
    vec3 N = normalize(fNormal);
    vec3 L = normalize(uLightPosVS - fPosition);
    vec3 V = normalize(-fPosition);      // camera at (0,0,0) in view space

    float diff = max(dot(N, L), 0.0);  // diffuse component

    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), uShininess);

    // removed specular to match dof rendering (which is already heavy enough)
    //vec3 color = uMeshColor * diff + spec * uLightColor;
    vec3 color = uMeshColor * diff;

    //color *= uLightIntensity;

    FragColor = vec4(color, 1.0);
}
