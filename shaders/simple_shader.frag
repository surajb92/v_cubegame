#version 450

layout(location = 0) in vec3 fragColor;
// no built-in output variable, so we have to define one outselves
layout(location = 0) out vec4 outColor;

// Push constants
layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    // RGB + Alpha value.
    outColor = vec4(fragColor, 1.0);
}