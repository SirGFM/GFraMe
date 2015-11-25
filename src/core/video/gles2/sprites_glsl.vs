/**
 * Vertex shader for rendering stanced sprites
 *
 * @file src/core/video/gles2/sprites_glsl.vs
 */

"#version 100\n"

/** The current vertex */
"attribute vec2 vtx;\n"

/** Screen to OpenGL transformation matrix */
"uniform mat4 locToGL;\n"

/** Current texture dimensions */
"uniform vec2 texDimensions;\n"

/** Position and whether the sprite is flipped */
"uniform ivec3 sprData;\n"

/** Tile's width, height and index */
"uniform vec3 tileData;\n"

/** Texture coordinate for the current vertex */
"varying mediump vec2 texCoord;\n"

"void main() {\n"
    /* -- Output the vertex position ------------------------------------ */

"    vec2 pos = vtx;\n"

    /* Flip the X axis, if necessary */
"    pos.x *= 1.0f - float(2 * sprData.z);\n"

    /* Expand the postion to the sprite's dimensions (i.e., convert a (0,0)
     * centered, 1 unit wide square to a rectangle of width dimensions.x and
     * height dimensions.y dimensions */
"    pos *= tileData.xy;\n"
"    pos += tileData.xy * vec2(0.5f, 0.5f);\n"

    /* Translate the sprite to its in-screen position */
"    pos += float(sprData.xy);\n"

    /* Convert from screen-space to opengl-space */
"    vec4 position = vec4(pos.x, pos.y, -1.0f, 1.0f);\n"
"    gl_Position = position * locToGL;\n"

    /* -- Output the texture coordinate --------------------------------- */

"    vec2 texOffset;\n"
"    float columns = texDimensions.x / tileData.x;\n"

    /* Calculate the tile position into the texture */
"    texOffset.x = mod(tileData.z, columns) * tileData.x;\n"
"    texOffset.y = floor(tileData.z / columns) * tileData.y;\n"
"    texOffset /= texDimensions;\n"

    /* Again, start with the default square and convert it to a rectangle */
"    vec2 _texCoord = vtx + vec2(0.5f, 0.5f);\n"
"    _texCoord *= tileData.xy / texDimensions;\n"

    /* Offset it by the tile position and output it to the fragment shader */
"    texCoord = _texCoord + texOffset;\n"
"}"

