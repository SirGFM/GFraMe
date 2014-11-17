
static char sprVs[] = 
  "#version 330\n"
  "layout(location = 0) in vec2 vtx;\n"
  "out vec2 texCoord;\n"
  "uniform mat4 locToGL;\n"
  "uniform vec2 texDimensions;\n"
  "uniform vec2 translation;\n"
  "uniform float dimension;\n"
  "uniform vec2 texOffset;\n"
  "void main() {\n"
  "  vec2 pos = vtx;\n"
  "  pos *= dimension;\n"
  "  pos += dimension*vec2(0.5f,"
  "                            0.5f);\n"
  "  pos += translation;\n"
  "  vec4 position = vec4(pos.x, pos.y,"
  "                     -1.0f, 1.0f);\n"
  "  gl_Position = position*locToGL;\n"
  
  "  vec2 _texCoord = vtx + vec2(0.5f,"
                             "0.5f);\n"
  "  _texCoord *= texDimensions;\n"
  "  _texCoord *= dimension;\n"
  "  texCoord = texDimensions*texOffset"
  "                +_texCoord;\n"
  "}\n";

static char sprFs[] = 
  "#version 330\n"
  "in vec2 texCoord;\n"
  "uniform sampler2D gSampler;\n"
  "void main() {\n"
  "  gl_FragColor = texture2D(gSampler, texCoord.st);\n"
  "}\n";

static char bbVs[] = 
  "#version 330\n"
  "layout(location = 0) in vec2 vtx;\n"
  "out vec2 texCoord;\n"
  "void main() {\n"
  "  gl_Position = vec4(vtx, -1.0f, 1.0f);\n"
  "  texCoord = 0.5f * vtx + vec2(0.5f, 0.5f);\n"
  "}\n";

static char bbFs[] = 
  "#version 330\n"
  "in vec2 texCoord;\n"
  "uniform sampler2D gSampler;\n"
//  "uniform vec2 windowDimensions;\n"
//  "uniform vec2 ppTexDimensions;\n"
  "void main() {\n"
  "  vec4 pixel = texture2D(gSampler, texCoord.st);\n"
  "  gl_FragColor = pixel;\n"
  "}\n";

