#version 450

// 'in' keyword denotes 'input' vectors. 'location' is what matters between shaders, names can be different.
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

// Output variable is built-in, but we can create more as required.
// No association between Input and output locations, so location = 0 is different for 'in' and 'out'.
layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec3 directionToLight;
} ubo;

// Push constants expected. Must match the order from the main file.
// Only one push constant block per shader entry, and max size is 128 Bytes.
layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

const float AMBIENT = 0.02; 

// Main function executes once for each vertex we have.
void main() {
    gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);

    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);
    fragColor = lightIntensity * color;
}