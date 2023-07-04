//使用Assimp加载模型到Mesh中

#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model
{
public:
	//模型数据
	vector<Mesh> meshes;//模型中的所有网格
	string directory;
	vector<Texture> textures_loaded;//存储到目前为止已加载的所有纹理，以确保不会多次加载同一个纹理
	bool gammaCorrection;//是否进行gamma校正

	//重载“ = ”符号
	Model &operator = (Model rhs) {
		this->textures_loaded = rhs.textures_loaded;
		this->meshes = rhs.meshes;
		this->directory = rhs.directory;
		this->gammaCorrection = rhs.gammaCorrection;
		return *this;
	}

	//默认构造函数
	Model() = default;

	//构造函数，从给定路径加载模型
	Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
	{
		loadModel(path);
	}

	//绘制模型中的所有网格
	void Draw(Shader shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++) {
			meshes[i].Draw(shader);
		}
	}

	//实例化绘制
	void DrawInstance(Shader shader, int amount)
	{
		for (unsigned int i = 0; i < meshes.size(); i++) {
			meshes[i].DrawInstance(shader, amount);
		}
	}

private:
	//使用ASSIMP从指定路径中加载模型到这个类（的meshes中）
	void loadModel(string const &path)
	{
		//使用ASSIMP读取文件到scene中
		//importer.ReadFile（文件路径，一些后期处理的选项）
		//aiProcess_Triangulate：将图元形状变换为三角形；aiProcess_FlipUVs：翻转y轴纹理坐标；aiProcess_CalcTangentSpace：计算切线和副切线
		Assimp::Importer importer;		
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);//importer.ReadFile（文件路径，一些后期处理的选项）
		//检查errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		//获取文件路径的目录路径
		directory = path.substr(0, path.find_last_of('/'));

		//递归地处理scene的根节点，
		processNode(scene->mRootNode, scene);
	}

	//递归地处理节点
	void processNode(aiNode *node, const aiScene *scene)
	{
		//处理当前节点的所有网格
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {			
			//注意：节点中存储的是索引，真正的数据在scene中
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));//将scene中的mesh转换成我们定义的Mesh，并放到meshes中
		}
		//处理完当前节点后，需要递归地对它的子节点进行处理
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}

	}

	//访问网格的相关属性并将它们储存到我们定义的对象中
	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		// data to fill
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		//遍历网格中的所有顶点，获取数据填充到vertices中
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;//assimp使用了自己的vector类，它不能直接转换为glm的vec3类，所以需要一个vector作为中介
			//位置
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			//法向量
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			//纹理坐标
			if (mesh->mTextureCoords[0]) {//如果mesh中有纹理坐标
				glm::vec2 vec;
				//Assimp允许一个模型在一个顶点上有最多8个不同的纹理坐标，但我们只使用第一组纹理坐标
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}else{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}
			//切线
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
			//副切线
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
			vertices.push_back(vertex);
		}

		//遍历所有的face，并储存face的索引到indices中	//由于使用了aiProcess_Triangulate选项，face总是三角形
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mMaterialIndex >= 0) {//如果网格中包含有材质
			//处理materials，从中获取材质信息，将纹理放到textures中
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			//漫反射贴图
			vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			//镜面反射贴图
			vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			//光泽度贴图
			std::vector<Texture> shininessMaps = loadMaterialTextures(material, aiTextureType_SHININESS, "texture_shininess");
			textures.insert(textures.end(), shininessMaps.begin(), shininessMaps.end());
			//法向量贴图
			std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		}

		//将从mesh中提取的数据封装成我们定义的Mesh
		return Mesh(vertices, indices, textures);
	}

	//遍历给定纹理类型的所有纹理位置，获取纹理的文件位置，并加载和生成纹理，将信息存到一个vector中
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);//获取纹理的文件位置
			//检查纹理是否在之前加载过，如果是就跳过它
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {//如果具有相同文件路径的纹理已被加载
					textures.push_back(textures_loaded[j]);
					skip = true;//那就跳过它
					break;
				}
			}

			if (!skip) {//如果纹理还没有被加载，那就加载它
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);//存储到textures_loaded中，让我们可以检查一个纹理是否被加载过
			}
		}
		return textures;
	}
};

//加载纹理
unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		//判断图片格式
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		if (format == GL_RGBA) {
			//避免使用透明值时，纹理图像的顶部与底部边缘的纯色值进行插值
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//避免采样时对应的采样点产生细微偏移，产生透明边框
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
#endif