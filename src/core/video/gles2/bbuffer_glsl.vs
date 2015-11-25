/**
 * Fragment shader for rendering stanced sprites
 *
 * @file src/core/video/gles2/sprites_glsl.vs
 */


"#version 100\n"

/** Texture coordinate for the current vertex */
"varying mediump vec2 texCoord;\n"

/** The current vertex */
"attribute vec2 vtx;\n"

"void main() {\n"
    /* Scaling is done automatically, simply maps src square to dst */
"    gl_Position = vec4(vtx, -1.0f, 1.0f);\n"

    /* Translate the origin from the center to the upper left corner */
"    texCoord = 0.5f * vtx + vec2(0.5f, 0.5f);\n"
"}"

