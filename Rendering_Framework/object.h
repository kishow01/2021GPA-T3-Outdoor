#pragma once

#include "src\basic\Shader.h"

#include <GLFW\glfw3.h>
#include <glad\glad.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Object {
private:
	typedef struct
	{
		GLuint vao;			// vertex array object
		GLuint vbo;			// vertex buffer object

		int materialId;
		int vertexCount;
		GLuint m_texture;
	} Shape;

	Shape m_shape;
	Shader *m_shader;
	GLuint um4mv_id;
	GLuint um4p_id;
	GLuint tex_id;
	GLuint shadow_mat_id;
	GLuint shadow_tex_id;

	int type;
	// 0 for rendering plane,
	// 1 for rendering house
public:
	glm::mat4 um4m;
	glm::mat4 um4v;
	glm::mat4 um4p;
	glm::mat4 shadow_matrix;
public:
	Object(int t);
	virtual ~Object();
	void initialize();
	void renderPass(GLuint depthMap);
	void renderLight(GLuint mvp_id, glm::mat4 light_vp_matrix);
private:
	void loadModule();
};