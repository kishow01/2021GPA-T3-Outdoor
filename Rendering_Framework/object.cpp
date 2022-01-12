#include "object.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "stb_image.h"

// define a simple data structure for storing texture image raw data
typedef struct _texture_data
{
	_texture_data() : width(0), height(0), data(0) {}
	int width;
	int height;
	unsigned char* data;
} texture_data;

// load a png image and return a TextureData structure with raw data
// not limited to png format. works with any image format that is RGBA-32bit
texture_data loadImg(const char* path)
{
	texture_data texture;
	int n;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc *data = stbi_load(path, &texture.width, &texture.height, &n, 4);
	if (data != NULL)
	{
		texture.data = new unsigned char[texture.width * texture.height * 4 * sizeof(unsigned char)];
		memcpy(texture.data, data, texture.width * texture.height * 4 * sizeof(unsigned char));
		stbi_image_free(data);
	}
	return texture;
}

Object::Object(int t) { 
	type = t;
}

Object::~Object() { }

void Object::loadModule(PlantManager *m_plantManager) {
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;

	bool ret;
	if(type == 0)
		ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models\\airplane.obj", "models");
	else if(type == 1)
		ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models\\medievalHouse.obj", "models");
	else if(type == 2)
		ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models\\trees\\baum_hd_pine_trunk.obj", "models\\trees");
	else if(type == 3)
		ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models\\trees\\baum_hd_trunk.obj", "models\\trees");
	else if(type == 4)
		ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models\\trees\\grass.obj", "models\\trees");
	else if(type == 5)
		ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models\\trees\\grass2.obj", "models\\trees");
	// type == 6 2's leaf
	else if(type == 6)
		ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models\\trees\\baum_hd_pine_leaves.obj", "models\\trees");
	else if(type == 7)
		ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models\\trees\\baum_hd_leaves.obj", "models\\trees");

	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
	}
	if (!ret) {
		exit(1);
	}

	vector<float> vertices, texcoords, normals;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	std::vector<unsigned int> indices;

	cout << "Type: " << type << " loading ... " << endl;

	// 大多數的 shapes.size() = 1
	// for (int s = 0; s < shapes.size(); ++s) {    // for 'ladybug.obj', there is only one object
	for (int s = 0; s < 1; ++s) {    // for 'ladybug.obj', there is only one object
		int index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 2]);

				indices.push_back(index_offset + v);
			}
			index_offset += fv;
			m_shape.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &m_shape.vao);
	glBindVertexArray(m_shape.vao);

	glGenBuffers(1, &m_shape.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbo);

	GLuint position_buffer;
	GLuint normal_buffer;
	GLuint tangent_buffer;
	GLuint texcoord_buffer;
	GLuint instance_position_buffer;
	GLuint index_buffer;

	glGenBuffers(1, &position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	// Compute tangent
	float *tangents = new float[normals.size()];
	for (unsigned int i = 0; i < indices.size(); i += 3)
	{
		int idx0 = indices[i];
		int idx1 = indices[i + 1];
		int idx2 = indices[i + 2];

		glm::vec3 p1(vertices[idx0 * 3 + 0], vertices[idx0 * 3 + 1], vertices[idx0 * 3 + 2]);
		glm::vec3 p2(vertices[idx1 * 3 + 0], vertices[idx1 * 3 + 1], vertices[idx1 * 3 + 2]);
		glm::vec3 p3(vertices[idx2 * 3 + 0], vertices[idx2 * 3 + 1], vertices[idx2 * 3 + 2]);

		glm::vec3 n1(normals[idx0 * 3 + 0], normals[idx0 * 3 + 1], normals[idx0 * 3 + 2]);
		glm::vec3 n2(normals[idx1 * 3 + 0], normals[idx1 * 3 + 1], normals[idx1 * 3 + 2]);
		glm::vec3 n3(normals[idx2 * 3 + 0], normals[idx2 * 3 + 1], normals[idx2 * 3 + 2]);

		glm::vec2 uv1(texcoords[idx0 * 2 + 0], texcoords[idx0 * 2 + 1]);
		glm::vec2 uv2(texcoords[idx1 * 2 + 0], texcoords[idx1 * 2 + 1]);
		glm::vec2 uv3(texcoords[idx2 * 2 + 0], texcoords[idx2 * 2 + 1]);

		glm::vec3 dp1 = p2 - p1;
		glm::vec3 dp2 = p3 - p1;

		glm::vec2 duv1 = uv2 - uv1;
		glm::vec2 duv2 = uv3 - uv1;

		float r = 1.0f / (duv1[0] * duv2[1] - duv1[1] * duv2[0]);

		glm::vec3 t;
		t[0] = (dp1[0] * duv2[1] - dp2[0] * duv1[1]) * r;
		t[1] = (dp1[1] * duv2[1] - dp2[1] * duv1[1]) * r;
		t[2] = (dp1[2] * duv2[1] - dp2[2] * duv1[1]) * r;

		glm::vec3 t1 = glm::cross(n1, t);
		glm::vec3 t2 = glm::cross(n2, t);
		glm::vec3 t3 = glm::cross(n3, t);

		tangents[idx0 * 3 + 0] += t1[0];
		tangents[idx0 * 3 + 1] += t1[1];
		tangents[idx0 * 3 + 2] += t1[2];

		tangents[idx1 * 3 + 0] += t2[0];
		tangents[idx1 * 3 + 1] += t2[1];
		tangents[idx1 * 3 + 2] += t2[2];

		tangents[idx2 * 3 + 0] += t3[0];
		tangents[idx2 * 3 + 1] += t3[1];
		tangents[idx2 * 3 + 2] += t3[2];
	}
	glGenBuffers(1, &tangent_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), tangents, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	delete tangents;

	glGenBuffers(1, &texcoord_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	/* init instance_position buffer */
	if (type == 2 || type == 6) {
		m_numPlantInstance = m_plantManager->m_numPlantInstance[0];
		// instance_position_buffer
		glGenBuffers(1, &instance_position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, instance_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, m_plantManager->m_numPlantInstance[0] * 3 * sizeof(float), m_plantManager->m_plantInstancePositions[0], GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(4);

		glVertexAttribDivisor(4, 1);
	} else if (type == 3 || type == 7) {
		m_numPlantInstance = m_plantManager->m_numPlantInstance[1];
		// instance_position_buffer
		glGenBuffers(1, &instance_position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, instance_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, m_plantManager->m_numPlantInstance[1] * 3 * sizeof(float), m_plantManager->m_plantInstancePositions[1], GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(4);

		glVertexAttribDivisor(4, 1);
	} else if (type == 4) {
		m_numPlantInstance = m_plantManager->m_numPlantInstance[2];
		// instance_position_buffer
		glGenBuffers(1, &instance_position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, instance_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, m_plantManager->m_numPlantInstance[2] * 3 * sizeof(float), m_plantManager->m_plantInstancePositions[2], GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(4);

		glVertexAttribDivisor(4, 1);
	} else if(type == 5) {
		m_numPlantInstance = m_plantManager->m_numPlantInstance[3];
		// instance_position_buffer
		glGenBuffers(1, &instance_position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, instance_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, m_plantManager->m_numPlantInstance[3] * 3 * sizeof(float), m_plantManager->m_plantInstancePositions[3], GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(4);

		glVertexAttribDivisor(4, 1);
	}

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	index_count = indices.size();

	cout << "Type: " << type << ", Load " << m_shape.vertexCount << " vertices" << endl;

	texture_data tdata;
	if (type == 0)
		tdata = loadImg("models\\textures\\Airplane_smooth_DefaultMaterial_BaseMap.png");
	else if (type == 1)
		tdata = loadImg("models\\textures\\Medieval_house_Diffuse.png");
	else if (type == 2)
		tdata = loadImg("models\\trees\\textures\\bark_pine_wh.png");
	else if (type == 3)
		tdata = loadImg("models\\trees\\textures\\bark_wh.png");
	else if (type == 4)
		tdata = loadImg("models\\trees\\textures\\GrassE_mt1_D.jpg");
	else if (type == 5)
		tdata = loadImg("models\\trees\\textures\\grass2_1.png");
	else if (type == 6)
		tdata = loadImg("models\\trees\\textures\\ast5.png");
	else if (type == 7)
		tdata = loadImg("models\\trees\\textures\\ast3.png");

	glGenTextures(1, &m_shape.m_texture);
	glBindTexture(GL_TEXTURE_2D, m_shape.m_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete tdata.data;

	if(type == 1) {
		tdata = loadImg("models\\textures\\Medieval_house_Normal.png");

		glGenTextures(1, &m_shape.m_normaltex);
		glBindTexture(GL_TEXTURE_2D, m_shape.m_normaltex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		delete tdata.data;
	}

	if(type == 5) {
		vertices.clear();
		texcoords.clear();
		normals.clear();
		// LOADING SHAPE 2 

		for (int s = 1; s < 2; ++s) {    // for 'ladybug.obj', there is only one object
			int index_offset = 0;
			for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
				int fv = shapes[s].mesh.num_face_vertices[f];
				for (int v = 0; v < fv; ++v) {
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
					vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
					vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
					texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
					texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
					normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
					normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
					normals.push_back(attrib.normals[3 * idx.normal_index + 2]);

					indices.push_back(index_offset + v);
				}
				index_offset += fv;
				m_shape2.vertexCount += fv;
			}
		}

		glGenVertexArrays(1, &m_shape2.vao);
		glBindVertexArray(m_shape2.vao);

		glGenBuffers(1, &m_shape2.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_shape2.vbo);

		GLuint position_buffer;
		GLuint normal_buffer;
		GLuint texcoord_buffer;
		GLuint instance_position_buffer;
		GLuint index_buffer;
	
		glGenBuffers(1, &position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &normal_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &texcoord_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);
		glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);

		m_numPlantInstance = m_plantManager->m_numPlantInstance[3];
		// instance_position_buffer
		glGenBuffers(1, &instance_position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, instance_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, m_plantManager->m_numPlantInstance[3] * 3 * sizeof(float), m_plantManager->m_plantInstancePositions[3], GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(4);

		glVertexAttribDivisor(4, 1);

		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
		index_count = indices.size();

		tdata = loadImg("models\\trees\\textures\\grass2_2.png");

		glGenTextures(1, &m_shape2.m_texture);
		glBindTexture(GL_TEXTURE_2D, m_shape2.m_texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		delete tdata.data;
	}

	cout << "Type: " << type << " loading finish" << endl;
}

void Object::initialize(PlantManager *m_plantManager) {
	loadModule(m_plantManager);

	// Set up Shader for plane
	this->m_shader = new Shader("src\\shader\\object.vs.glsl", "src\\shader\\object.fs.glsl");

	const GLuint programId = this->m_shader->getProgramID();
	um4mv_id = glGetUniformLocation(programId, "um4mv");
	um4p_id = glGetUniformLocation(programId, "um4p");
	tex_id = glGetUniformLocation(programId, "tex");
	normal_tex_id = glGetUniformLocation(programId, "normal_tex");
	shadow_mat_id = glGetUniformLocation(programId, "shadow_matrix");
	shadow_tex_id = glGetUniformLocation(programId, "shadow_tex");

	render_type_id = glGetUniformLocation(programId, "render_type");
	nm_mapping_id = glGetUniformLocation(programId, "normal_mapping_enable");
}

void Object::renderPass(GLuint depthMap) {
	this->m_shader->useShader();

	glUniformMatrix4fv(um4mv_id, 1, GL_FALSE, value_ptr(um4v * um4m));
	glUniformMatrix4fv(um4p_id, 1, GL_FALSE, value_ptr(um4p));
	glUniformMatrix4fv(shadow_mat_id, 1, GL_FALSE, value_ptr(shadow_matrix));

	glUniform1i(render_type_id, type);
	glUniform1i(nm_mapping_id, enable_normal_mapping);

	glBindVertexArray(m_shape.vao);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_shape.m_texture);
	glUniform1i(tex_id, 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_shape.m_normaltex);
	glUniform1i(normal_tex_id, 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(shadow_tex_id, 5);

	if (type == 2 || type == 3 || type == 4 || type == 5 || type == 6 ||type == 7)
		glDrawElementsInstanced(GL_TRIANGLES, m_shape.vertexCount, GL_UNSIGNED_INT, 0, m_numPlantInstance);
	else
		glDrawElements(GL_TRIANGLES, m_shape.vertexCount, GL_UNSIGNED_INT, 0);

	if(type == 5) {
		glBindVertexArray(m_shape2.vao);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_shape2.m_texture);
		glUniform1i(tex_id, 3);
	
		glDrawElementsInstanced(GL_TRIANGLES, m_shape2.vertexCount, GL_UNSIGNED_INT, 0, m_numPlantInstance);
	}

	this->m_shader->disableShader();
}

void Object::renderLight(GLuint mvp_id, glm::mat4 light_vp_matrix) {
	glBindVertexArray(m_shape.vao);
	glUniformMatrix4fv(mvp_id, 1, GL_FALSE, value_ptr(light_vp_matrix * um4m));

	if (type == 2 || type == 3 || type == 4 || type == 5 || type == 6 || type == 7)
		glDrawElementsInstanced(GL_TRIANGLES, m_shape.vertexCount, GL_UNSIGNED_INT, 0, m_numPlantInstance);
	else 
		glDrawElements(GL_TRIANGLES, m_shape.vertexCount, GL_UNSIGNED_INT, 0);
}