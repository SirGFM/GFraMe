/**
 * Vertex shader for rendering stanced sprites
 *
 * @file src/core/video/gles2/sprites_glsl.vs
 */

"#version 330\n"

/** The current vertex */
"layout(location = 0) in vec2 vtx;\n"

/** Texture coordinate for the current vertex */
"out vec2 texCoord;\n"

/** Screen to OpenGL transformation matrix */
"uniform mat4 locToGL;\n"

/** Current texture dimensions */
"uniform vec2 texDimensions;\n"

/** Texture used to pass all data */
"uniform isamplerBuffer instanceData;\n"

"void main() {\n"
    /** .xy = sprite's position; .z = isFlipped */
"    ivec3 translation;\n"

    /** .xy = tile's dimensions, .z = tile's index */
"    ivec3 tile;\n"

    /** Index on the instances array */
"    int index;\n"

    /* -- Retrieve the instance data ------------------------------------ */

    /* Calculate the per-texel offset and the base index */
"    index = 2 * gl_InstanceID;\n"

    /* Retrieve the position (and horizontal flipping) */
"    translation.xyz = texelFetch(instanceData, index).rgb;\n"

    /* Retrieve the tile data */
"    tile.xyz = texelFetch(instanceData, index + 1).rgb;\n"

    /* -- Output the vertex position ------------------------------------ */

"    vec2 pos = vtx;\n"

    /* Flip the X axis, if necessary */
"    pos.x *= 1.0f - (2.0f * translation.z);\n"

    /* Expand the postion to the sprite's dimensions (i.e., convert a (0,0)
     * centered, 1 unit wide square to a rectangle of width dimensions.x and
     * height dimensions.y dimensions */
"    pos *= tile.xy;\n"
"    pos += tile.xy * vec2(0.5f, 0.5f);\n"

    /* Translate the sprite to its in-screen position */
"    pos += translation.xy;\n"

    /* Convert from screen-space to opengl-space */
"    vec4 position = vec4(pos.x, pos.y, -1.0f, 1.0f);\n"
"    gl_Position = position * locToGL;\n"

    /* -- Output the texture coordinate --------------------------------- */

"    vec2 texOffset;\n"
"    float columns = floor(texDimensions.x / tile.x);\n"

    /* Calculate the tile position into the texture */
"    texOffset.x = mod(tile.z, columns) * tile.x;\n"
"    texOffset.y = floor(tile.z / columns) * tile.y;\n"
"    texOffset /= texDimensions;\n"

    /* Again, start with the default square and convert it to a rectangle */
"    vec2 _texCoord = vtx + vec2(0.5f, 0.5f);\n"
"    _texCoord *= tile.xy / texDimensions;\n"

    /* Offset it by the tile position and output it to the fragment shader */
"    texCoord = _texCoord + texOffset;\n"
"}"

