#pragma once

#include <fstream>
#include "basic\TextureMaterial.h"
#include "basic\Transformation.h"
#include "basic\SceneManager.h"

#include "basic\Shader.h"

class Terrain
{
public:
	Terrain();
	virtual ~Terrain();
	float getHeight(const float x, const float z) const;

private:
	GLuint m_vao;
	GLuint m_dataBuffer;

	GLuint m_shadowDataBuffer;
	GLuint m_shadowVAO;

	int m_dataByteOffset;
	Transformation *m_transform;

private:
	glm::vec3 worldVToHeightMapUV(float x, float z) const;

	void fromMYTD(const std::string &filename);

	int m_width;
	int m_height;
	float m_heightScale;

private:
	GLuint m_indexBuffer;
	GLuint m_shadowIndexBuffer;

	glm::mat4 m_worldToHeightUVMat;
	glm::mat4 m_worldToDiffuseUVMat;

	TextureMaterial *m_elevationTex;
	TextureMaterial *m_normalTex;
	TextureMaterial *m_colorTex;
	

	float* m_elevationMap;
	float* m_normalMap;
	float* m_colorMap;

	float m_heightFactor = 50.0;

	// 4 rotation matrix for chunk
	glm::mat4 m_chunkRotMat[4];


public:
	void update(glm::mat4 shadow_sbpv_matrix);
	void updateLight(GLuint mvp_id, glm::mat4 light_vp_matrix);

public:
	void setCameraPosition(const glm::vec3 &pos);

};

