//网格类，储存从Scene中解析出的网格数据

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

typedef struct Vertex Vertex;
struct Vertex {
	//位置
	glm::vec3 Position;
	//法向量
	glm::vec3 Normal;
	//纹理坐标
	glm::vec2 TexCoords;
	//切线
	glm::vec3 Tangent;
	//副切线
	glm::vec3 Bitangent;
};

typedef struct Texture Texture;
struct Texture {
	unsigned int id;
	string type;
	string path;//储存纹理的路径用于与其它纹理进行比较
};

class Mesh {
public:
	//网格数据
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	//构造函数
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
	{
		//从参数中获取到数据
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		//配置vao、vbo、ebo
		setupMesh();
	}

	//渲染网格
	void Draw(Shader shader)
	{
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);//激活一个尚未使用的纹理单元
			glUniform1i(glGetUniformLocation(shader.ID, textures[i].type.c_str()), i);//将采样器设置到对应的纹理单元			
			glBindTexture(GL_TEXTURE_2D, textures[i].id);//绑定纹理
		}

		//绘制网格
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//恢复原来的配置
		glActiveTexture(GL_TEXTURE0);
	}

	//实例化渲染，需要给定渲染的数量
	void DrawInstance(Shader shader, int amount)
	{
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);//激活一个尚未使用的纹理单元
			glUniform1i(glGetUniformLocation(shader.ID, textures[i].type.c_str()), i);//将采样器设置到对应的纹理单元
			glBindTexture(GL_TEXTURE_2D, textures[i].id);//绑定纹理
		}

		//实例化绘制网格
		glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, amount);
		glBindVertexArray(0);

		//恢复原来的配置
		glActiveTexture(GL_TEXTURE0);
	}

private:
	unsigned int VBO, EBO;

	//配置vao、vbo、ebo
	void setupMesh()
	{
		//创建vao、vbo、ebo
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		//绑定vbo和ebo
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		//设置顶点属性指针
		//位置
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		//法向量
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		//纹理坐标
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		//切线
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
		//副切线
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		glBindVertexArray(0);
	}
};
#endif
