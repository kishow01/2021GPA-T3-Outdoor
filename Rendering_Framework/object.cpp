#include "object.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "stb_image.h"

typedef struct _texture_data
{
	_texture_data() : width(0), height(0), data(0) {}
	int width;
	int height;
	unsigned char* data;
} texture_data;

texture_data loadImg(const char* path)
{
	texture_data texture;
	int n;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc *data = stbi_load(path, &texture.width, &texture.height, &n, 4);
	if (data != NULL) {
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

void Object::loadModule() {
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
	for (int s = 0; s < shapes.size(); ++s) {    // for 'ladybug.obj', there is only one object
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
			}
			index_offset += fv;
			m_shape.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &m_shape.vao);
	glBindVertexArray(m_shape.vao);

	glGenBuffers(1, &m_shape.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
	glEnableVertexAttribArray(2);

	shapes.clear();
	shapes.shrink_to_fit();
	materials.clear();
	materials.shrink_to_fit();
	vertices.clear();
	vertices.shrink_to_fit();
	texcoords.clear();
	texcoords.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();

	cout << "Load " << m_shape.vertexCount << " vertices" << endl;

	texture_data tdata;
	if (type == 0)
		tdata = loadImg("models\\textures\\Airplane_smooth_DefaultMaterial_BaseMap.png");
	else if (type == 1)
		tdata = loadImg("models\\textures\\Medieval_house_Diffuse.png");

	glGenTextures(1, &m_shape.m_texture);
	glBindTexture(GL_TEXTURE_2D, m_shape.m_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete tdata.data;
}

void Object::initialize() {
	loadModule();

	// Set up Shader for plane
	this->m_shader = new Shader("src\\shader\\object.vs.glsl", "src\\shader\\object.fs.glsl");

	const GLuint programId = this->m_shader->getProgramID();
	um4mv_id = glGetUniformLocation(programId, "um4mv");
	um4p_id = glGetUniformLocation(programId, "um4p");
	tex_id = glGetUniformLocation(programId, "tex");
	shadow_mat_id = glGetUniformLocation(programId, "shadow_matrix");
	shadow_tex_id = glGetUniformLocation(programId, "shadow_tex");
}

void Object::renderPass(GLuint depthMap) {
	this->m_shader->useShader();

	glUniformMatrix4fv(um4mv_id, 1, GL_FALSE, value_ptr(um4v * um4m));
	glUniformMatrix4fv(um4p_id, 1, GL_FALSE, value_ptr(um4p));
	glUniformMatrix4fv(shadow_mat_id, 1, GL_FALSE, value_ptr(shadow_matrix));

	glBindVertexArray(m_shape.vao);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_shape.m_texture);
	glUniform1i(tex_id, 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(shadow_tex_id, 4);

	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	this->m_shader->disableShader();
}

void Object::renderLight(GLuint mvp_id, glm::mat4 light_vp_matrix) {
	glBindVertexArray(m_shape.vao);
	glUniformMatrix4fv(mvp_id, 1, GL_FALSE, value_ptr(light_vp_matrix * um4m));
	glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);
}