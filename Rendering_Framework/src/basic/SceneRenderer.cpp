#include "SceneRenderer.h"


SceneRenderer::SceneRenderer()
{}


SceneRenderer::~SceneRenderer()
{
}

void SceneRenderer::renderPass(GLuint depthMap, glm::mat4 shadow_sbpv_matrix){
	this->m_shader->useShader();

	this->clear(glm::vec4(1.0, 1.0, 1.0, 1.0), 1.0);
	glViewport(0, 0, this->m_frameWidth, this->m_frameHeight);

	SceneManager *manager = SceneManager::Instance();

	glUniformMatrix4fv(manager->m_projMatHandle, 1, false, glm::value_ptr(this->m_projMat));
	glUniformMatrix4fv(manager->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(manager->m_shadowTextureHandle, 3);

	//////////////////////////////////////////////////////////////////////////////////	
	this->m_terrain->update(shadow_sbpv_matrix);

	this->m_shader->disableShader();
}

void SceneRenderer::renderLight(GLuint mvp_id, glm::mat4 light_vp_matrix) {
	this->m_terrain->updateLight(mvp_id, light_vp_matrix);
}

// =======================================
void SceneRenderer::resize(const int w, const int h){
	this->m_frameWidth = w;
	this->m_frameHeight = h;
}
void SceneRenderer::initialize(const int w, const int h, GLuint shadow_program){
	this->resize(w, h);
	this->setUpShader(shadow_program);

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void SceneRenderer::setProjection(const glm::mat4 &proj){
	this->m_projMat = proj;
}
void SceneRenderer::setView(const glm::mat4 &view, const glm::vec3 &cameraPos){
	this->m_viewMat = view;
	this->m_cameraPos = glm::vec4(cameraPos, 1.0);
	this->m_terrain->setCameraPosition(cameraPos);
}


void SceneRenderer::clear(const glm::vec4 &clearColor, const float depth){
	float d[] = { depth };
	glClearBufferfv(GL_COLOR, 0, glm::value_ptr(clearColor));
	glClearBufferfv(GL_DEPTH, 0, d);
	//glClearDepth(depth);
}
void SceneRenderer::appendTerrain(Terrain *t) {
	this->m_terrain = t;
}
void SceneRenderer::setUpShader(GLuint shadow_program){
	this->m_shader = new Shader("src\\shader\\oglVertexShader.glsl", "src\\shader\\oglFragmentShader.glsl");
	this->m_shader->useShader();

	const GLuint programId = this->m_shader->getProgramID();

	SceneManager *manager = SceneManager::Instance();
	manager->m_vertexHandle = glGetAttribLocation(programId, "v_vertex");
	manager->m_normalHandle = glGetAttribLocation(programId, "v_normal");
	manager->m_uvHandle = glGetAttribLocation(programId, "v_uv");

	manager->m_modelMatHandle = glGetUniformLocation(programId, "modelMat");
	manager->m_viewMatHandle = glGetUniformLocation(programId, "viewMat");
	manager->m_projMatHandle = glGetUniformLocation(programId, "projMat");
	manager->m_shadowMatHandle = glGetUniformLocation(programId, "shadow_matrix");

	manager->m_texture0Handle = glGetUniformLocation(programId, "texture0");
	manager->m_shadowTextureHandle = glGetUniformLocation(programId, "shadow_tex");

	// terrain
	manager->m_vToHeightUVMatHandle = glGetUniformLocation(m_shader->getProgramID(), "vToHeightUVMat");
	manager->m_normalMapHandle = glGetUniformLocation(m_shader->getProgramID(), "normalMap");
	manager->m_elevationMapHandle = glGetUniformLocation(m_shader->getProgramID(), "elevationMap");

	// Shadow shader 
	manager->m_shadowVertexHandle = glGetAttribLocation(shadow_program, "position");

	manager->m_diffuseTexUnit = GL_TEXTURE0;
	glUniform1i(manager->m_texture0Handle, 0);
	manager->m_elevationMapTexUnit = GL_TEXTURE1;
	glUniform1i(manager->m_elevationMapHandle, 1);
	manager->m_normalMapTexUnit = GL_TEXTURE2;
	glUniform1i(manager->m_normalMapHandle, 2);
}