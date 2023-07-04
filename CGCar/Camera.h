//自由漫游相机，用来控制其相关数据

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

//定义摄像机的移动方向
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

//默认的相机属性值
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 7.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 50.0f;

//Camera类用于处理输入，计算view矩阵
class Camera
{
public:
	//相机属性
	glm::vec3 Position;//位置
	glm::vec3 Front;//朝向（相对于相机当前位置）
	glm::vec3 Up;//上向量
	glm::vec3 Right;//右向量
	glm::vec3 WorldUp;//世界坐标系中的上向量
	//欧拉角，用来得到相机朝向
	float Yaw;//偏航角
	float Pitch;//俯仰角
	//其他的几个属性
	float MovementSpeed;//相机移动速度
	float MouseSensitivity;//鼠标灵敏度
	float Zoom;//视野大小

	//构造函数（使用向量）
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	//构造函数（使用标量）
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	//得到view矩阵
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);//lookAt(摄像机位置,目标位置,上向量)
	}

	//处理键盘输入，需要移动方向和上一帧的渲染时间
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		switch (direction)
		{
		case FORWARD:
			Position += Front * velocity;
			break;
		case BACKWARD:
			Position -= Front * velocity;
			break;
		case LEFT:
			Position -= Right * velocity;
			break;
		case RIGHT:
			Position += Right * velocity;
			break;
		case UP:
			Position += WorldUp * velocity;
			break;
		case DOWN:
			Position -= WorldUp * velocity;
			break;
		default:
			break;
		}

		//Position.y = 0.0f;//让相机停留在地面上
	}

	//处理鼠标移动，需要x和y方向的偏移量
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean isConstrainedPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		//避免俯仰角超出范围，使得屏幕翻转
		if (isConstrainedPitch)
		{
			if (Pitch > 89.0f) {
				Pitch = 89.0f;
			}
			if (Pitch < -89.0f) {
				Pitch = -89.0f;
			}
		}

		//更新相机的三个向量
		updateCameraVectors();
	}

	//处理鼠标滚轮，需要竖直方向偏移量
	void ProcessMouseScroll(float yOffset)
	{
		float maxFov = 90.0f;
		float minFov = 1.0f;
		if (Zoom >= minFov && Zoom <= maxFov) {
			Zoom -= yOffset;
		}
		if (Zoom <= minFov) {
			Zoom = minFov;
		}
		if (Zoom >= maxFov) {
			Zoom = maxFov;
		}
	}

private:
	//根据相机当前的欧拉角，计算并更新相机坐标系（由朝向、右向量、上向量组成）
	void updateCameraVectors()
	{
		//计算Front向量
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		//重新计算右向量和上向量
		Right = glm::normalize(glm::cross(Front, WorldUp));//归一化向量
		Up = glm::normalize(glm::cross(Right, Front));
	}
};

#endif