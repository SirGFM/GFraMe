/**
 * Fragment shader for rendering the backbuffer
 *
 * @file src/core/video/opengl3/bbuffer_glsl.vs
 */

"#version 330\n"

/** Texture coordinate, retrieved from vertex shader */
"in vec2 texCoord;\n"

/** The current texture */
"uniform sampler2D gSampler;\n"

"void main() {\n"
    /* Simply retrieve the color from the texture coordinate */
"    gl_FragColor = texture2D(gSampler, texCoord.st);\n"
"}"

