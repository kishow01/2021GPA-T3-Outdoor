#pragma once

#include "src\basic\Shader.h"
#include "SceneSetting.h"

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
		GLuint m_normaltex;
	} Shape;

	int index_count;
	int m_numPlantInstance;

	Shape m_shape;
	Shape m_shape2;
	Shader *m_shader;
	GLuint um4mv_id;
	GLuint um4p_id;
	GLuint tex_id;
	GLuint normal_tex_id;
	GLuint shadow_mat_id;
	GLuint shadow_tex_id;

	GLuint render_type_id;
	GLuint nm_mapping_id;

	GLuint Ka_id;
	GLuint Kd_id;
	GLuint Ks_id;

	int type;
	// 0 for rendering plane,
	// 1 for rendering house
	// 2 for rendering tree0's trunk
	// 3 for rendering tree1's trunk
	// 4 for rendering grass0
	// 5 for rendering grass1
public:
	bool enable_normal_mapping;
public:
	glm::mat4 um4m;
	glm::mat4 um4v;
	glm::mat4 um4p;
	glm::mat4 shadow_matrix;

	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
public:
	Object(int t);
	virtual ~Object();
	void initialize(PlantManager *m_plantManager);
	void renderPass(GLuint depthMap);
	void renderLight(GLuint mvp_id, glm::mat4 light_vp_matrix);
private:
	void loadModule(PlantManager *m_plantManager);
};