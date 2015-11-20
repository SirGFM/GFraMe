/**
 * Fragment shader for rendering stanced sprites
 *
 * @file src/core/video/opengl3/sprites_glsl.vs
 */


"#version 330\n"

/** The current vertex */
"layout(location = 0) in vec2 vtx;\n"

/** Texture coordinate for the current vertex */
"out vec2 texCoord;\n"

"void main() {\n"
    /* Scaling is done automatically, simply maps src square to dst */
"    gl_Position = vec4(vtx, -1.0f, 1.0f);\n"

    /* Translate the origin from the center to the upper left corner */
"    texCoord = 0.5f * vtx + vec2(0.5f, 0.5f);\n"
"}"

