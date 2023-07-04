//主程序

#include "pch.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Car.h"
#include "Trees.h"
#include "Solid.h"


#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION //千万别忘了这行。害我找了半天才搞清楚状况
#endif
#include <stb_image.h>

using namespace std;
///================================================================================
//函数声明
///================================================================================
void initGlfwWindow();//初始化并配置GLFW
void createGlfwWindow();//生成一个GLFW窗口，并设置一些事件回调函数
void loadGLFuncPtr();//加载所有的OpenGL函数指针

void framebuffer_size_callback(GLFWwindow *window, int width, int height);//改变窗口大小时，修改视口大小
void processInput(GLFWwindow *window);//处理键盘输入（改变纹理可见度，移动相机位置）
void mouse_callback(GLFWwindow *window, double xPos, double yPos);//移动鼠标时，改变相机朝向
void scroll_callback(GLFWwindow *window, double xOffset, double yOffset);//滚动滚轮时，改变相机视野大小

unsigned int loadTexture(char const * path);//加载纹理
unsigned int loadCubemap(vector<std::string> faces);//从六个纹理面中加载天空盒纹理


void renderDepthMap(glm::vec3 lightPos, const Shader &depthMapShader);//为光源渲染深度立方体贴图
void renderScene(const Shader &shader);//渲染场景
void renderLamps();//在光源位置渲染物体
void renderBorderPoints();//渲染边界标志球
void renderSkybox(const Shader &skyboxShader);//渲染天空盒

void loadResource();//加载纹理、模型等资源

void checkCollision();//检测碰撞

void getBorderSignPositions();//获取边界标志点


///================================================================================
//全局变量
///================================================================================
//窗口
///--------------------
GLFWwindow *window;//glfw窗口
int SCR_WIDTH = 1600;//窗口宽度
int SCR_HEIGHT = 1000;//窗口高度

//模型矩阵、观察矩阵、透视投影矩阵
///--------------------
glm::mat4 model = glm::mat4(1.0f);//初始化为单位矩阵
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

//顶点和位置
///--------------------
//立方体在世界空间中的位置
vector<glm::vec3> boxPositions = {
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(6.0f, 5.0f, -15.0f),
	glm::vec3(-1.5f, 8.2f, -2.5f),
	glm::vec3(-3.8f, 2.0f, -12.3f),
	glm::vec3(2.4f, 0.4f, -3.5f),
	glm::vec3(-5.7f, 3.0f, -7.5f),
	glm::vec3(1.3f, 4.0f, -4.5f)
	//glm::vec3(1.5f, 2.0f, -2.5f),
	//glm::vec3(1.5f, 0.2f, -1.5f),
	//glm::vec3(-1.3f, 1.0f, -1.5f),
	//glm::vec3(4.0f, 3.5f, 0.0),
	//glm::vec3(2.0f, 3.0f, 1.0),
	//glm::vec3(-3.0f, 1.0f, 0.0),
	//glm::vec3(-1.5f, 1.0f, 1.5),
	//glm::vec3(-1.5f, 2.0f, -3.0)
};
float borderRadius = 80.0f;//汽车可活动的边界的半径
vector<glm::vec3> borderSignPositions;//边界上的标志点
glm::vec3 lightPos = glm::vec3(0.0f);//点光源位置

//物体对象
///--------------------
Car car = Car(glm::vec3(0.0f, 0.0f, 4.0f));
Trees trees = Trees();

//渲染必需信息
///--------------------
//移动鼠标，改变相机方向
bool isFirstMouse = true;//鼠标是否是第一次移动
float lastXPos = SCR_WIDTH / 2;//鼠标在上一帧的位置
float lastYPos = SCR_HEIGHT / 2;
//调整相机移动速度时使用
float deltaTime = 0.0f; //当前帧与上一帧开始渲染的时间差
float lastFrameTime = 0.0f; //上一帧开始渲染的时间
//相机视野
float fieldOfView = 50.0f;

//键盘控制的数值
///--------------------
bool isKeyBPressed = false;
bool shadowsKeyPressed = false;
bool normalMapKeyPressed = false;
bool shininessMapKeyPressed = false;
bool isShadowsShowing = true;//是否显示阴影
bool isNormalMap = true;//是否使用法向量贴图
bool isShininessMap = true;//是否使用光泽度贴图
glm::vec3 lightAmbient = glm::vec3(0.2f);

//相机选择
///--------------------
enum Camera_Type {
	CAR_CAMERA,//跟随车移动位置
	FIXED_CAMERA,//固定位置
	FREE_CAMERA//自由移动位置
};
Camera_Type cameraType = FREE_CAMERA;//相机类型
Camera freeCamera = Camera(glm::vec3(0.0f, 4.0f, 10.0f));//自由漫游相机
glm::vec3 viewPos = glm::vec3(1.0f);//观察者位置（在着色器中用来计算反射相关内容）

//场景资源
///--------------------
Model carModel;
Model treeModel1;
Model treeModel2;
unsigned int skyboxTexture;//天空盒
unsigned int sidewalkColTex;//漫反射贴图
unsigned int sidewalkReflTex;//镜面反射贴图
unsigned int sidewalkGlossTex;//光泽度贴图
unsigned int sidewalkNrmTex;//法线贴图
unsigned int sidewalk2ColTex;
unsigned int sidewalk2ReflTex;
unsigned int sidewalk2GlossTex;
unsigned int sidewalk2NrmTex;

//着色器
///-----------------------------------------
Shader basicShader;//最简单的着色器，直接将物体输出为指定颜色
Shader skyboxShader;//渲染天空盒的着色器
Shader reflectShader;//反射天空盒立方体贴图的着色器
Shader lightDepthMapShader;//生成点光源深度立方体贴图的着色器
Shader blinnPhongShader;//用来渲染场景的着色器，使用Blinn-Phong光照模型

///================================================================================
//main()函数
///================================================================================
int main() {
	//初始配置
	///-----------------------------------------
	initGlfwWindow();//初始化并配置GLFW
	createGlfwWindow();//生成一个GLFW窗口，并设置一些事件回调函数
	loadGLFuncPtr();//GLAD：加载所有的OpenGL函数指针
	//配置OpenGL全局状态
	glEnable(GL_DEPTH_TEST);//启用深度测试
	glEnable(GL_CULL_FACE);//启用面剔除
	glEnable(GL_BLEND);//启用混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);///设定混合函数

	//加载资源（包括着色器、纹理图片、模型）
	///-----------------------------------------
	loadResource();

	//获取初始数据
	///-----------------------------------------
	srand(glfwGetTime()); // 初始化随机种子
	trees.setup(treeModel1, treeModel2);
	getBorderSignPositions();

	//渲染循环。在GLFW退出前会一直保持运行
	///-----------------------------------------
	while (!glfwWindowShouldClose(window)) {///检查GLFW是否被要求退出
		//计算上一帧的渲染时间
		///-----------------------------------------
		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		//处理输入
		///-----------------------------------------
		processInput(window);//检查键盘输入


		//更新数据
		///-----------------------------------------
		//更新汽车数据
		car.updatePos(deltaTime);
		car.updateCoordSys(deltaTime);
		checkCollision();
		//更新投影矩阵
		projection = glm::perspective(glm::radians(fieldOfView), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 200.0f);//200基本足够覆盖全场了
		//更新观察矩阵和观察者位置（view和viewPos）
		switch (cameraType)
		{
		case CAR_CAMERA:
			view = car.GetViewMatrix();
			viewPos = car.cameraPos;
			break;
		case FIXED_CAMERA:
			//使用汽车的Front和固定原点位置来跟随汽车
			glm::vec3 cameraPos = glm::vec3(0.0f, 50.0f, 0.0f);
			glm::vec3 targetPos = car.Position + car.Front * 9.0f;
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			view = glm::lookAt(cameraPos, targetPos, up);//lookAt(摄像机位置,目标位置,上向量)
			
			viewPos = cameraPos;
			break;
		case FREE_CAMERA:
			view = freeCamera.GetViewMatrix();
			viewPos = freeCamera.Position;
			break;
		default:
			break;
		}
		//移动点光源
		lightPos.x = sin(glfwGetTime() * 0.1f) * 10.0f;
		lightPos.z = cos(glfwGetTime() * 0.1f) * 10.0f;
		lightPos.y = 15.0f;


		//渲染场景
		///-----------------------------------------
		//每次新的渲染开始时清屏，否则仍然能看到上次迭代的渲染结果
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);//设置清空屏幕所用的颜色（这是一个状态设置函数）
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空屏幕的颜色缓冲//清除深度缓冲//（否则前一帧的信息仍然保存在缓冲中）//（这是一个状态使用函数）
		//渲染场景到深度贴图
		///--------------------------
		lightDepthMapShader.use();
		renderDepthMap(lightPos, lightDepthMapShader);
		//正常地渲染场景
		///--------------------------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);//改回视口大小
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blinnPhongShader.use();
		renderScene(blinnPhongShader);
		//渲染剩下的物体
		///--------------------------		
		renderLamps();//在光源位置渲染代表光源的物体		
		renderBorderPoints();//渲染边界标志球		
		skyboxShader.use();//最后画天空盒
		renderSkybox(skyboxShader);


		//检查并调用事件，交换缓冲
		///-----------------------------------------
		glfwPollEvents();//检查有没有触发事件（键盘输入、鼠标移动等），更新窗口状态，调用对应的回调函数
		glfwSwapBuffers(window);//交换颜色缓冲
	}

	//渲染结束，释放资源
	///-----------------------------------------
	glfwTerminate();

	return 0;
}


///================================================================================
//函数定义
///================================================================================
//初始化并配置GLFW
void initGlfwWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//告诉GLFW，当前使用的GL版本是3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心模式，不向后兼容
}

//生成一个GLFW窗口，并设置一些事件回调函数
void createGlfwWindow() {
	//创建一个窗口对象（这个对象存放了所有和窗口相关的数据）
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CG-CAR", NULL, NULL);//（宽，高，标题，暂时忽略，暂时忽略）
	if (window == NULL) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		exit(-1);//return -1;
	}

	glfwMakeContextCurrent(window);//将这个窗口设置为当前的上下文
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//设置一个回调函数，当窗口大小变换时调用。当窗口第一次显示时也会调用
	glfwSetCursorPosCallback(window, mouse_callback);//鼠标位置回调函数
	glfwSetScrollCallback(window, scroll_callback);//鼠标滚轮回调函数
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//隐藏光标并捕捉鼠标
}

//加载所有的OpenGL函数指针
void loadGLFuncPtr() {
	//GLAD在这里的作用是将函数地址保存到函数指针中
	//这样做因为OpenGl驱动版本众多，大多数函数的位置无法在编译时确定，需要在运行时查询
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {//glfwGetProcAddress是用来获得OpenGL函数指针地址的函数
		cout << "Failed to initialize GLAD" << endl;
		exit(-1);//return -1;
	}
}


//改变窗口大小时，修改视口大小
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = width * 5 / 8;
	glViewport(0, 0, width, width * 5 / 8);//设置OpenGL渲染窗口尺寸大小（即视口viewport）//等比例缩放
}

//处理键盘输入
void processInput(GLFWwindow *window) {
	//关闭窗口
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {//检查按键是否被按下，如果没有按下，glfwGetKey会返回GLFW_RELEASE
		glfwSetWindowShouldClose(window, true);
	}

	//控制汽车移动
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ) {
		car.isPowered = true;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) {
		car.isPowered = false;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		car.isBraking = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
		car.isBraking = false;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		car.isLefting = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) {
		car.isLefting = false;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		car.isRighting = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) {
		car.isRighting = false;
	}

	//平移自由漫游相机
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		freeCamera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		freeCamera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		freeCamera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		freeCamera.ProcessKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		freeCamera.ProcessKeyboard(UP, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		freeCamera.ProcessKeyboard(DOWN, deltaTime);
	}

	//lightAmbient
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (lightAmbient.x < 0.8f) {
			lightAmbient += glm::vec3(0.01f);
			cout << "lightAmbient.x: "<< lightAmbient.x << endl;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (lightAmbient.x > 0.05f) {
			lightAmbient -= glm::vec3(0.01f);
			cout << "lightAmbient.x: " << lightAmbient.x << endl;
		}
	}

	//切换相机
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !isKeyBPressed)	{
		switch (cameraType)
		{
		case CAR_CAMERA:
			cameraType = FIXED_CAMERA;
			break;
		case FIXED_CAMERA:
			cameraType = FREE_CAMERA;
			break;
		case FREE_CAMERA:
			cameraType = CAR_CAMERA;
			break;
		default:
			break;
		}
		//cout << "cameraType:" << cameraType << endl;
		isKeyBPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)	{
		isKeyBPressed = false;
	}

	//开启阴影
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && !shadowsKeyPressed)
	{
		isShadowsShowing = !isShadowsShowing;
		cout << "Shadows: " << isShadowsShowing << endl;
		shadowsKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE)
	{
		shadowsKeyPressed = false;
	}

	//开启法线贴图
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && !normalMapKeyPressed)
	{
		isNormalMap = !isNormalMap;
		cout << "NormalMap: " << isNormalMap << endl;
		normalMapKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_RELEASE)
	{
		normalMapKeyPressed = false;
	}

	//开启光泽度贴图
	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS && !shininessMapKeyPressed)
	{
		isShininessMap = !isShininessMap;
		cout << "ShininessMap: " << isShininessMap << endl;
		shininessMapKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_RELEASE)
	{
		shininessMapKeyPressed = false;
	}
}

//移动鼠标时，改变自由漫游相机朝向
void mouse_callback(GLFWwindow *window, double xPos, double yPos) {
	//第一次移动鼠标时，xpos和ypos会等于鼠标刚刚进入屏幕的那个位置，而lastXPos和lastYPos被初始化为屏幕中心位置。这样做可以避免计算出一个不合理的偏移量
	if (isFirstMouse) {//它的初始值为true
		isFirstMouse = false;
		lastXPos = (float)xPos;
		lastYPos = (float)yPos;
	}

	//根据当前鼠标位置和上一帧鼠标位置，计算鼠标位置偏移量
	float xOffset = (float)xPos - lastXPos;
	float yOffset = lastYPos - (float)yPos;//y的坐标要反过来
	lastXPos = (float)xPos;
	lastYPos = (float)yPos;

	freeCamera.ProcessMouseMovement(xOffset, yOffset);
}

//滚动滚轮时，改变投影矩阵的视野大小
void scroll_callback(GLFWwindow *window, double xOffset, double yOffset) {
	float maxFov = 90.0f;
	float minFov = 1.0f;
	if (fieldOfView >= minFov && fieldOfView <= maxFov) {
		fieldOfView -= (float)yOffset;
	}
	if (fieldOfView <= minFov) {
		fieldOfView = minFov;
	}
	if (fieldOfView >= maxFov) {
		fieldOfView = maxFov;
	}
}


//加载纹理
unsigned int loadTexture(char const * path)
{
	//创建纹理
	unsigned int textureID;
	glGenTextures(1, &textureID);

	//stbi_set_flip_vertically_on_load(true);//加载图像时翻转y轴
	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);//根据文件路径读取图像数据
	if (data)
	{//图像成功加载
		//判断图片格式
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);//使用图片数据生成纹理
		glGenerateMipmap(GL_TEXTURE_2D);//生成mipmap

		//为当前绑定的纹理设置环绕、过滤方式	//GL_REPEAT--超出的部分重复纹理图像		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);//生成了纹理和多级渐远纹理后，释放图像内存
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

//从六个纹理面中加载天空盒纹理
unsigned int loadCubemap(vector<std::string> faces)
{
	//创建并绑定纹理
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	//stbi_set_flip_vertically_on_load(true);//加载图像时翻转y轴
	int width, height, nrComponents;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);//根据文件路径读取图像数据
		if (data) {//图像成功加载
			//判断图片格式
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);//使用图片数据生成纹理
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	//GL_LINEAR--线性滤波	GL_CLAMP_TO_EDGE--超出的部分会重复纹理坐标的边缘
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}


//为光源渲染深度立方体贴图
const unsigned int DEPTHMAP_WIDTH = 2048, DEPTHMAP_HEIGHT = 2048;
unsigned int depthCubemap;//深度立方体贴图
float near_plane = 1.0f;//光的投影的近平面
float far_plane = 80.0f;//光的投影的远平面
unsigned int depthMapFBO = 0;
void renderDepthMap(glm::vec3 lightPos, const Shader &depthMapShader){
	if (depthMapFBO == 0) {
		//创建一个深度立方体贴图
		///-----------------------------------------
		glGenFramebuffers(1, &depthMapFBO);
		glGenTextures(1, &depthCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		//生成立方体贴图的每个面
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, DEPTHMAP_WIDTH, DEPTHMAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//把立方体贴图附加成帧缓冲的深度附件
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
		glDrawBuffer(GL_NONE);//这个帧缓冲对象不会渲染到一个颜色缓冲里
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	//生成光空间的变换矩阵
	///-----------------------------------------------
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)DEPTHMAP_WIDTH / (float)DEPTHMAP_HEIGHT, near_plane, far_plane);//透视投影矩阵
	//光空间变换矩阵	//6个观察方向：右、左、上、下、近、远
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

	//将场景渲染到深度立方体贴图
	///-----------------------------------------
	glViewport(0, 0, DEPTHMAP_WIDTH, DEPTHMAP_HEIGHT);//改变视口大小
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);//帧缓存绑定为深度立方体贴图的那个帧缓存
	glClear(GL_DEPTH_BUFFER_BIT);
	for (unsigned int i = 0; i < 6; ++i) {
		depthMapShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);//光空间变换矩阵
	}
	depthMapShader.setFloat("far_plane", far_plane);
	depthMapShader.setVec3("lightPos", lightPos);
	renderScene(depthMapShader);//渲染场景
	glBindFramebuffer(GL_FRAMEBUFFER, 0);//绑定回默认帧缓存
}

//渲染场景
void renderScene(const Shader &shader)
{
	//公共配置
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	shader.setInt("depthMap", 2);//光源深度贴图
	shader.setMat4("projection", projection);//投影矩阵
	shader.setMat4("view", view);//更新过的观察矩阵
	//设置光照所需参数
	shader.setVec3("lightPos", lightPos);
	shader.setVec3("viewPos", freeCamera.Position);//观察者位置
	shader.setBool("shadows", isShadowsShowing);//通过按键启用/禁用阴影
	shader.setFloat("far_plane", far_plane);//far_plane = 25.0f，点光源的投影的远平面
	shader.setBool("isShininessMap", isShininessMap);//通过按键启用/禁用光泽度贴图
	shader.setBool("isNormalMap", isNormalMap);//根据按键决定是否开启法向量贴图
	shader.setVec3("lightAmbient", lightAmbient);
	shader.setBool("isInstanced", false);//不使用实例化渲染

	//汽车
	model = glm::mat4(1.0f);
	model = glm::translate(model, car.Position);
	model = glm::rotate(model, glm::radians(car.Yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));//对于未来运输车多转180度，因为模型初始时朝向z轴正半轴
	model = glm::scale(model, glm::vec3(0.005f));//模型太大了，将它缩小
	shader.setMat4("model", model);
	carModel.Draw(shader);
	//树
	shader.setBool("isInstanced", true);//开启实例化渲染
	treeModel1.DrawInstance(shader, trees.treePositions_1.size());
	treeModel2.DrawInstance(shader, trees.treePositions_2.size());
	shader.setBool("isInstanced", false);//关闭实例化渲染
	//地面
	glDisable(GL_CULL_FACE);
	model = glm::mat4(1.0f);//初始化为单位矩阵
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(200.0f));
	glActiveTexture(GL_TEXTURE0);//激活纹理单元
	glBindTexture(GL_TEXTURE_2D, sidewalkColTex);//绑定纹理
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sidewalkReflTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, sidewalkGlossTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, sidewalkNrmTex);
	shader.setMat4("model", model);
	shader.setInt("texture_diffuse", 0);
	shader.setInt("texture_specular", 1);
	shader.setInt("texture_shininess", 2);
	shader.setInt("texture_normal", 3);
	renderQuad();
	glEnable(GL_CULL_FACE);

	//一堆箱子
	glActiveTexture(GL_TEXTURE0);//激活纹理单元
	glBindTexture(GL_TEXTURE_2D, sidewalk2ColTex);//绑定纹理
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sidewalk2ReflTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, sidewalk2GlossTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, sidewalk2NrmTex);
	shader.setMat4("model", model);
	shader.setInt("texture_diffuse", 0);
	shader.setInt("texture_specular", 1);
	shader.setInt("texture_shininess", 2);
	shader.setInt("texture_normal", 3);
	for (int i = 0; i < boxPositions.size(); i++) {
		model = glm::mat4(1.0f);
		model = glm::translate(model, boxPositions.at(i));
		//model = glm::scale(model, glm::vec3(0.5f));
		float angle = 20.0f * i;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		shader.setMat4("model", model);
		renderCube();
	}
}

//在光源位置渲染物体
void renderLamps()
{
	//显示代表点光源位置的箱子
	///-----------------------------------------
	//将矩阵传入光源位置的箱子的着色器
	model = glm::mat4(1.0f);
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(0.2f));//缩小
	basicShader.use();
	basicShader.setMat4("model", model);
	basicShader.setMat4("view", view);
	basicShader.setMat4("projection", projection);
	basicShader.setVec4("rgbaColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	//渲染光源位置的箱子
	renderCube();
}

//渲染边界标志球
void renderBorderPoints() {

	//显示标志边界的球
	///-----------------------------------------
	glDisable(GL_CULL_FACE);//关闭面剔除
	for (int i = 0; i < borderSignPositions.size(); i++) {
		model = glm::mat4(1.0f);
		model = glm::translate(model, borderSignPositions.at(i));
		//model = glm::scale(model, glm::vec3(30.0f));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		reflectShader.use();
		reflectShader.setMat4("model", model);
		reflectShader.setMat4("view", view);
		reflectShader.setMat4("projection", projection);
		reflectShader.setVec3("cameraPos", viewPos);
		skyboxShader.setInt("skybox", 0);//为纹理单元指定采样器
		renderSphere();
	}
	glEnable(GL_CULL_FACE);//启用面剔除

}

//渲染天空盒
void renderSkybox(const Shader &skyboxShader) {
	glDisable(GL_CULL_FACE);//关闭面剔除
	glDepthFunc(GL_LEQUAL);//深度缓冲将会填充上天空盒的1.0值，所以我们需要保证天空盒在值小于或等于深度缓冲而不是小于时通过深度测试
	view = glm::mat4(glm::mat3(view));//移除变换矩阵的位移部分
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	skyboxShader.setMat4("view", view);
	skyboxShader.setMat4("projection", projection);
	skyboxShader.setInt("skybox", 0);//为纹理单元指定采样器
	renderCube();
	glDepthFunc(GL_LESS);//将深度函数设回默认
	glEnable(GL_CULL_FACE);//启用面剔除
}

//加载纹理、模型等资源
void loadResource()
{
	//生成着色器
	///-----------------------------------------
	basicShader = Shader("../shaders/basic.vs", "../shaders/basic.fs");
	skyboxShader = Shader("../shaders/skybox.vs", "../shaders/skybox.fs");
	reflectShader = Shader("../shaders/reflect.vs", "../shaders/reflect.fs");
	lightDepthMapShader = Shader("../shaders/lightDepthMap.vs", "../shaders/lightDepthMap.fs", "../shaders/lightDepthMap.gs");
	blinnPhongShader = Shader("../shaders/BlinnPhong.vs", "../shaders/BlinnPhong.fs");

	//加载纹理资源
	///-----------------------------------------
	vector<std::string> faces{
		"../resources/textures/skybox/right.jpg",
		"../resources/textures/skybox/left.jpg",
		"../resources/textures/skybox/top.jpg",
		"../resources/textures/skybox/bottom.jpg",
		"../resources/textures/skybox/front.jpg",
		"../resources/textures/skybox/back.jpg",
	};
	skyboxTexture = loadCubemap(faces);//天空盒纹理
	//人行道
	sidewalkColTex = loadTexture("../resources/textures/sidewalk1/col.jpg");
	sidewalkReflTex = loadTexture("../resources/textures/sidewalk1/refl.jpg");
	sidewalkGlossTex = loadTexture("../resources/textures/sidewalk1/gloss.jpg");
	sidewalkNrmTex = loadTexture("../resources/textures/sidewalk1/nrm.jpg");
	//人行道2
	sidewalk2ColTex = loadTexture("../resources/textures/sidewalk2/col.jpg");
	sidewalk2ReflTex = loadTexture("../resources/textures/sidewalk2/refl.jpg");
	sidewalk2GlossTex = loadTexture("../resources/textures/sidewalk2/gloss.jpg");
	sidewalk2NrmTex = loadTexture("../resources/textures/sidewalk2/nrm.jpg");

	//加载模型
	///-----------------------------------------
	carModel = Model("../resources/objects/Lamborghini/Lamborghini_Aventador_nocollider.obj");
	treeModel1 = Model("../resources/objects/Tree/Tree.obj");
	treeModel2 = Model("../resources/objects/Tree2/Tree.obj");
}

//检测碰撞
void checkCollision()
{
	//盒子
	for (int i = 0; i < boxPositions.size(); i++) {
		if (car.isCollided(boxPositions.at(i), 1.4f)) {
			car.stopMove();
			break;
		}
	}
	//树木
	for (int i = 0; i < trees.treePositions_1.size(); i++) {
		if (car.isCollided(trees.treePositions_1.at(i), 0.3f)) {
			car.stopMove();
			break;
		}
	}
	for (int i = 0; i < trees.treePositions_2.size(); i++) {
		if (car.isCollided(trees.treePositions_2.at(i), 0.3f)) {
			car.stopMove();
			break;
		}
	}
	//地图边界附近
	if (car.isAtBorder(borderRadius)) {
		car.stopMove();
	}
	//边界标志球
	for (int i = 0; i < borderSignPositions.size(); i++) {
		if (car.isCollided(borderSignPositions.at(i), 1.0f)) {
			car.stopMove();
			break;
		}
	}

}

//获取边界标志点
void getBorderSignPositions()
{
	int pointNum = 8;
	for (float i = 0; i < 360; i += (360 / pointNum)) {
		glm::vec3 point = glm::vec3(borderRadius*sin(glm::radians(i)), 1.0f, borderRadius*cos(glm::radians(i)));
		borderSignPositions.push_back(point);
		//cout << point.x << " " << point.y << " " << point.z <<" "<<i<< endl;
	}
}
