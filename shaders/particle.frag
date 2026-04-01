#version 450

layout(location = 0) flat in int inType;
layout(location = 0) out vec4 outColor;

// Distinct colors for up to 8 particle types
const vec3 TYPE_COLORS[8] = vec3[8](
    vec3(1.0, 0.2, 0.2),  // Red
    vec3(0.2, 1.0, 0.2),  // Green
    vec3(0.3, 0.5, 1.0),  // Blue
    vec3(1.0, 1.0, 0.2),  // Yellow
    vec3(1.0, 0.2, 1.0),  // Magenta
    vec3(0.2, 1.0, 1.0),  // Cyan
    vec3(1.0, 0.6, 0.2),  // Orange
    vec3(0.8, 0.8, 0.8)   // White
);

void main() {
    int idx = clamp(inType, 0, 7);
    outColor = vec4(TYPE_COLORS[idx], 1.0);
}
