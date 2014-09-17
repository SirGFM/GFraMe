/**
 * @src/gframe_opengl.c
 */

GLuint vbo;
GLuint ibo;

GFraMe_opengl_ret GFraMe_opengl_init() {
	// Create a vertex buffer object (i.e., geometry vertexes)
	const float vbo_data[] = {-0.5f,-0.5f, -0.5f,0.5f, 0.5f,0.5f, 0.5f,-0.5f};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_data), vbo_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Create a index buffer object (i.e., map for vertexes)
	const GLshort ibo_data[] = {0,1,2, 2,3,0};
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo_data), ibo_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GFraMe_opengl_clear() {
	glDeleteBuffer(1, &vbo);
	glDeleteBuffer(1, &ibo);
}

