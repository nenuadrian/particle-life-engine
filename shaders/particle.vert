#version 450

struct Particle {
    vec2 position;
    vec2 velocity;
    int particleType;
    float _pad1;
    float _pad2;
    float _pad3;
};

layout(std430, set = 0, binding = 0) readonly buffer Particles {
    Particle particles[];
};

layout(location = 0) out int outType;

void main() {
    Particle p = particles[gl_VertexIndex];

    // Map [0,1] position to [-1,1] clip space
    vec2 clipPos = p.position * 2.0 - 1.0;
    gl_Position = vec4(clipPos, 0.0, 1.0);
    gl_PointSize = 3.0;

    outType = p.particleType;
}
