#pragma once

#include <vector>
#include "Shader.h"
#include "SceneManager.h"


#include "../Terrain.h"

class SceneRenderer
{
public:
	SceneRenderer();
	virtual ~SceneRenderer();

private:
	Shader *m_shader;
	glm::mat4 m_projMat;
	glm::mat4 m_viewMat;
	glm::vec4 m_cameraPos;
	int m_frameWidth;
	int m_frameHeight;

	Terrain *m_terrain = nullptr;

public:
	void resize(const int w, const int h);
	void initialize(const int w, const int h, GLuint shadow_program);

	void setProjection(const glm::mat4 &proj);
	void setView(const glm::mat4 &view, const glm::vec3 &cameraPos);
	void setShadow(const glm::mat4 mat);
	void appendTerrain(Terrain *t);

// pipeline
public:
	void renderPass(GLuint depthMap, glm::mat4 shadow_sbpv_matrix);
	void renderLight(GLuint mvp_id, glm::mat4 light_vp_matrix);

private:
	void clear(const glm::vec4 &clearColor = glm::vec4(0.0, 0.0, 0.0, 1.0), const float depth = 1.0);
	void setUpShader(GLuint shadow_program);
};

