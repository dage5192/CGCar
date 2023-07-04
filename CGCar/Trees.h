//树类，用来生成树的数据

#ifndef TREES_H
#define TREES_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <GLFW/glfw3.h>

using namespace std;

enum Tree_Type {
	BIG_TREE,//只是因为第二种模型比较大
	SMALL_TREE
};

class Trees
{
public:
	//树的位置 和 模型矩阵
	vector<glm::vec3> treePositions_1;
	vector<glm::vec3> treePositions_2;
	glm::mat4 *treeModelMatrices_1;
	glm::mat4 *treeModelMatrices_2;

	//vector<glm::vec3> treePositions;
	//glm::mat4 *treeModelMatrices;

	//构造函数（使用向量）
	Trees() = default;

	void setup(Model treeModel_1, Model treeModel_2) {
		srand(glfwGetTime()); //初始化随机种子
		setTreePositions();
		setTreeModelMatrices();
		setArrayObject(treeModel_1, treeModel_2);
	}

private:
	void setTreePositions() {
		int column = 5, row = 5;//行数和列数
		float gap = 6.0f;//行列间的间距
		glm::vec3 treePos = glm::vec3(1.0f);
		//注意ij初始值
		for (int i = 2; i <= column; i++) {
			for (int j = 2; j <= row; j++) {
				if ( i>=4 && j>=4 )continue;

				//添加四个角落的位置
				treePos = glm::vec3(i*gap, 0.0f, j*gap);
				if (rand() % 2) {
					treePositions_1.push_back(treePos);
				}
				else {
					treePositions_2.push_back(treePos);
				}

				treePos = glm::vec3(i*gap*(-1), 0.0f, j*gap);
				if (rand() % 2) {
					treePositions_1.push_back(treePos);
				}
				else {
					treePositions_2.push_back(treePos);
				}

				treePos = glm::vec3(i*gap, 0.0f, j*gap*(-1));
				if (rand() % 2) {
					treePositions_1.push_back(treePos);
				}
				else {
					treePositions_2.push_back(treePos);
				}

				////treePos = glm::vec3(i*gap*(-1), 0.0f, j*gap*(-1));
				////if (rand() % 2) {
				////	treePositions_1.push_back(treePos);
				////}
				////else {
				////	treePositions_2.push_back(treePos);
				////}
			}
		}
	}

	void setTreeModelMatrices() {
		//int amount = treePositions.size();
		//treeModelMatrices = new glm::mat4[amount];

		//glm::mat4 model;
		//for (int i = 0; i < amount; i++) {
		//	model = glm::mat4(1.0f);//初始化为单位矩阵
		//	//移动到指定位置
		//	model = glm::translate(model, treePositions.at(i));
		//	//在0.75到1.25倍之间进行缩放
		//	float scale = (rand() % 50) / 100.0f + 0.75;
		//	model = glm::scale(model, glm::vec3(scale));
		//	//绕着Y轴进行随机的旋转
		//	float rotAngle = (rand() % 360);
		//	model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		//	//添加到矩阵的数组中
		//	treeModelMatrices[i] = model;
		//}

		int amount_1 = treePositions_1.size();
		treeModelMatrices_1 = new glm::mat4[amount_1];

		glm::mat4 model;
		for (int i = 0; i < amount_1; i++) {
			model = glm::mat4(1.0f);//初始化为单位矩阵
			//移动到指定位置
			model = glm::translate(model, treePositions_1.at(i));
			//在0.75到1.25倍之间进行缩放
			float scale = (rand() % 50) / 100.0f + 0.75;
			model = glm::scale(model, glm::vec3(scale*2));//模型太小了
			//绕着Y轴进行随机的旋转
			float rotAngle = (rand() % 360);
			model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			//添加到矩阵的数组中
			treeModelMatrices_1[i] = model;
		}


		int amount_2 = treePositions_2.size();
		treeModelMatrices_2 = new glm::mat4[amount_2];

		for (int i = 0; i < amount_2; i++) {
			model = glm::mat4(1.0f);//初始化为单位矩阵
			//移动到指定位置
			model = glm::translate(model, treePositions_2.at(i));
			//在0.75到1.25倍之间进行缩放
			float scale = (rand() % 50) / 100.0f + 0.75;
			model = glm::scale(model, glm::vec3(scale*1));
			//绕着Y轴进行随机的旋转
			float rotAngle = (rand() % 360);
			model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			//添加到矩阵的数组中
			treeModelMatrices_2[i] = model;
		}
	}

	void setArrayObject(Model treeModel_1, Model treeModel_2) {
		int amount_1 = treePositions_1.size();
		//将数据绑定到vbo
		unsigned int modelMaticesVBO_1;
		glGenBuffers(1, &modelMaticesVBO_1);
		glBindBuffer(GL_ARRAY_BUFFER, modelMaticesVBO_1);
		glBufferData(GL_ARRAY_BUFFER, amount_1 * sizeof(glm::mat4), &treeModelMatrices_1[0], GL_STATIC_DRAW);

		//为4个顶点属性设置属性指针，并将它们设置为实例化数组
		// -----------------------------------------------------------------------------------------------------------------------------------
		for (unsigned int i = 0; i < treeModel_1.meshes.size(); i++)
		{
			unsigned int VAO = treeModel_1.meshes[i].VAO;
			glBindVertexArray(VAO);
			//为4个顶点属性设置属性指针，并将它们设置为实例化数组
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);
			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);

			glBindVertexArray(0);
		}


		int amount_2 = treePositions_2.size();
		//将数据绑定到vbo
		unsigned int modelMaticesVBO_2;
		glGenBuffers(1, &modelMaticesVBO_2);
		glBindBuffer(GL_ARRAY_BUFFER, modelMaticesVBO_2);
		glBufferData(GL_ARRAY_BUFFER, amount_2 * sizeof(glm::mat4), &treeModelMatrices_2[0], GL_STATIC_DRAW);

		//为4个顶点属性设置属性指针，并将它们设置为实例化数组
		// -----------------------------------------------------------------------------------------------------------------------------------
		for (unsigned int i = 0; i < treeModel_2.meshes.size(); i++)
		{
			unsigned int VAO = treeModel_2.meshes[i].VAO;
			glBindVertexArray(VAO);
			//为4个顶点属性设置属性指针，并将它们设置为实例化数组
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);
			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);

			glBindVertexArray(0);
		}

	}
};

#endif