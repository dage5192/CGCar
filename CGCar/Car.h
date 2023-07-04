//汽车类，控制汽车相关数据

#ifndef CAR_H
#define CAR_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

using namespace std;

class Car
{
public:
	//属性
	glm::vec3 Position;//位置
	glm::vec3 Front;//朝向（相对于当前位置）
	glm::vec3 Up;//上向量
	glm::vec3 Right;//右向量
	float Yaw;//偏航角
	glm::vec3 lastPosition;//上一帧的位置

	//相机
	glm::vec3 cameraPos;
	glm::vec3 targetPos;
	float cameraYaw = -90.0f;
	float lastYaw = -90.0f;//用来追逐汽车
	//glm::vec3 lastFront;

	//按键状态
	bool isPowered = false;
	bool isBraking = false;
	bool isLefting = false;
	bool isRighting = false;
	//bool isCarCamera = true;

	//构造函数（使用向量）
	Car(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f)
	{
		Position = position;
		Front = glm::vec3(0.0f, 0.0f, -1.0f);
		Up = up;
		Yaw = yaw;
		lastPosition = Position;
		updateCoordSys(0);
	}
	//构造函数（使用标量）
	Car(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw)
	{
		Position = glm::vec3(posX, posY, posZ);
		Front = glm::vec3(0.0f, 0.0f, -1.0f);
		Up = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		lastPosition = Position;
		updateCoordSys(0);
	}

	//得到view矩阵
	glm::mat4 GetViewMatrix()
	{
		//updateCamera();
		return glm::lookAt(cameraPos, targetPos, Up);//lookAt(摄像机位置,目标位置,上向量)
	}

	//更新汽车自己的坐标系
	void updateCoordSys(float deltaTime) {
		//根据按键状态，更新偏航角
		if (isLefting) {
			Yaw += 50.0f * deltaTime;
		}
		if (isRighting) {
			Yaw -= 50.0f * deltaTime;
		}


		//根据当前的欧拉角，计算并更新坐标系（由朝向、右向量、上向量组成）
		//计算Front向量
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw));
		front.z = -1 * sin(glm::radians(Yaw));
		front.y = 0.0f;
		Front = glm::normalize(front);
		//重新计算右向量
		Right = glm::normalize(glm::cross(Front, Up));//归一化向量
		//cout << Front.x << " " << Front.y << " " << Front.z << " " << endl;


		updateCamera(deltaTime);
	}

	//更新汽车的位置
	void updatePos(float deltaTime)
	{
		lastPosition = Position;

		//没有倒车功能
		if (velocity <= 0) {
			velocity = 0;
			frictionForce = 0;
			brakingForce = 0;
		}
		else {
			frictionForce = 5;
			brakingForce = isBraking * 20.0f;
		}
		tractionForce = power * isPowered / (velocity + 1.0f);//避免除以0
		totalForce = tractionForce - frictionForce - brakingForce;
		acceleration = totalForce / 2.0f;//不忽略质量
		velocity = velocity + acceleration * deltaTime;
		Position = Position + Front * velocity * deltaTime;
		//cout <<  "isPowered:"<<isPowered<< " tractionForce:"<<tractionForce<<" velocity:"<< velocity <<" frictionForce:"<< frictionForce << endl;


		updateCamera(deltaTime);
	}

	//停车
	void stopMove() {
		tractionForce = 0;
		brakingForce = 0;
		frictionForce = 0;
		totalForce = 0;
		acceleration = 0;
		velocity = 0;
		power = 150;

		Position = lastPosition;
	}

	//检测碰撞
	bool isCollided(glm::vec3 objectPos, float objectRadius) {
		glm::vec3 carToObj = glm::vec3(objectPos - Position);
		float distanceSquare = glm::dot(carToObj, carToObj);//避免开平方计算
		float safeDistance = objectRadius + carRadius;
		if (distanceSquare <= safeDistance * safeDistance) {
			return true;
		}
		else {
			return false;
		}
	}

	//地图边界判断
	bool isAtBorder(float mapRadius) {
		glm::vec3 centerPos = glm::vec3(0.0f);
		glm::vec3 carToCenter = glm::vec3(centerPos - Position);
		float distanceSquare = glm::dot(carToCenter, carToCenter);//避免开平方计算
		float safeDistance = mapRadius - carRadius;
		if (distanceSquare >= safeDistance * safeDistance) {
			return true;
		}
		else {
			return false;
		}
	}

private:
	float tractionForce = 0;//牵引力
	float brakingForce = 0;//制动力
	float frictionForce = 0;//摩擦力
	float totalForce = 0;//合力
	float acceleration = 0;//加速度
	float velocity = 0;//速度
	float power = 150;//功率
	
	float carRadius = 1.0f;//汽车碰撞半径

	//更新相机坐标系和位置
	void updateCamera(float deltaTime) {
		cameraYaw += (Yaw - cameraYaw)*deltaTime*0.5f;//使用一个相对汽车进行了延迟的偏转角度来更新相机角度
		glm::vec3 front;
		front.x = cos(glm::radians(cameraYaw));
		front.z = -1 * sin(glm::radians(cameraYaw));
		front.y = 0.0f;

		//使用延迟的front来跟随汽车
		cameraPos = Position + Up * 10.0f - front * 10.0f;
		targetPos = Position + front * 9.0f;//看向汽车靠前的位置
	}
};

#endif