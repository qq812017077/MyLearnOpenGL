#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//图像库
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "Shaders.h"

/*	Camera / View space（右手坐标系）
	View matrix: 将所有的世界坐标转换为视图坐标，新的试图坐标与摄像机的位置和方向相关。

	To define a camera:
			我们需要相机在世界坐标系中的坐标。	（坐标）
			相机观察的方向						（z轴负向）
			指向相机右侧的向量					（x轴正向）
			指向相机上方的向量					（y轴）
	
	位置
		需要提及的是：
			z轴正向是通过屏幕指向人（你我）的方向。所以我们希望摄像机向后移动，即向正方向移动。
			glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	方向：
			方向向量：
				即  相机位置向量  -  目标位置向量 -->即方向向量
				（注意这里的方向向量不是最好的名字，因为实际上其是指向目标向量的相反方向）
				glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
				glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
	
			我们知道相机指向父方向，但是我们希望方向向量指向正方向。
	右轴：
		right axis
			该向量用于表示相机空间的x轴正向。
			这里通过叉乘获得右向量：
				首先定义一个上向量（Up vector），然后与方向向量进行叉乘。
					因为上向量位于  yz构成的平面。
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
			（up × cameraDirection		右手定则，四指指向up到z轴的方向	）
	上轴：
		Up axis
			同样通过右手定则。
				进行叉乘： cameradirection × x轴
				glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

		通过以上的步骤，我们得到一个 Loot At 矩阵

	Look At：
		使用矩阵的好处之一：
			如果你定义了一个坐标空间，里面有3个相互垂直的轴，你可以用这三个轴外加一个平移向量来创建一个矩阵
			
				你可以用这个矩阵乘以任何向量来变换到那个坐标空间。
						|  Rx   Ry		Rz	 0  |   |  1    0	 0	 −Px	|
			LookAt = 	|  Ux   Uy		Uz   0  | * |  0    1	 0   −Py	| 
						|  Dx   Dy		Dz   0  |	|  0    0	 1   −Pz	|
						|  0    0		0	 1	|	|  0    0	 0	  1		| 
				可以理解为先对点进行平移变换，然后再变换坐标系（变换基）
			注意Position向量是相反数，因为我们希望世界朝相反的方向移动。
*/

/*	Movement speed
	目前，我们的使用一个常量速度来控制相机移动，实际上是每一帧对其进行固定的移动。
		这样会导致一个问题:
			有的人一秒钟能绘制更多帧，而有的人一秒钟则绘制相对较少的帧，
			这样会导致不同的机器下，移动速度不一致。
		而我们希望所有的机器侠拥有者相同的速度。

		解决方法：
			使用deltatime ：该变量保存了渲染上一帧所花费的时间
				（the time it takes to render the last frame）
				当我们的deltaTime变大时意味着上一帧渲染花了更多时间，
				所以这一帧使用这个更大的deltaTime的值乘以速度，会获得更高的速度，这样就与上一帧平衡了。

*/

/*	视角移动（ Look around）：鼠标控制
		如果要改变视角，我们必须改变cameraFront（基于鼠标的输入），

		Euler angles 欧拉角
			欧拉角是代表三维旋转的三个值：
					Pitch :   俯仰角
					Yaw   :   偏航角
					Roll  :	  滚转角
			对于我们的摄像机系统来说，我们只关心俯仰角和偏航角
			(吃鸡里面的Q和E，应该代表了一定程度的滚转角的使用，不过仅仅通过鼠标的话，控制俯仰角和偏航角已经是极限了大概)
			假设当前的观察方向向量为Direction：
					俯仰角：Pitch
					偏航角：Yaw
				有	
					在y轴的分量：
						Direction.y = sin(Pitch);
					在xz轴的投影
					Direction.xz = cos(Pitch);
						继而根据Yaw
						Direction.x = cos(Pitch) * cos(Yaw) ; 
						Direction.z = cos(Pitch) * sin(Yaw) ; 
	鼠标控制：
			更改输入模式，使其隐藏光标：
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
			通过调用鼠标监听函数，来获取鼠标的信息。
				void mouse_callback(GLFWwindow* window, double xpos, double ypos);
				glfwSetCursorPosCallback(window, mouse_callback);

			FPS系统：
				1.计算鼠标的偏移量（两帧之间）
				2.将偏移值添加入摄像机的Yaw 和Pitch值
				3.给Yaw 和Pitch值添加最大值和最小值的限制
				4.计算方向向量。
	NOTE:
		因为一开始有对摄像头赋予一个方向，
			所以pitch 和yaw 业应该给予相应的值，使其对应到同样的方向上！
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow * window , double xPos , double yPos);
void scroll_callback(GLFWwindow * window , double xOffset , double yOffset);
void ProcessInput(GLFWwindow * window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static float lastX = 400.0f, lastY = 300.0f;
float pitch , yaw =-90.0f; 
float aspect = 45.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void Camera_1(GLFWwindow * window);
int main(void) {
	if (!glfwInit()){
		std::cout << "Failed to Initialization" << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "1-8_Camera", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window,mouse_callback);	//光标回调函数
	glfwSetScrollCallback(window, scroll_callback);		//鼠标滚轮监测
	//loading GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Camera_1(window);
	glfwTerminate();
	return 0;
}


void ProcessInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}float cameraSpeed = 0.05f; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	return;
}


float cubeVertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

void Camera_1(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	Shader shader("shader1-9.vertex","shader1-9.fragment");

	//Texture 
	const char * PhotoPath[2] = {
		"awesomeface.png",
		"container.jpg"
	};
	int width, height, nrChannels;
	unsigned char * data = NULL;
	stbi_set_flip_vertically_on_load(true);//	调转y轴
	unsigned int textures[2];
	glGenTextures(2, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	data = stbi_load(PhotoPath[0], &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	data = stbi_load(PhotoPath[1], &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//Vertices
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 + 3 * sizeof(float)));	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//bind Texture
	shader.use();
	shader.setInt("ourTexture1", 0);
	shader.setInt("ourTexture2", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	//while
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		ProcessInput(window);
		
		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();

		float radius = 10.0f;
		float camX = sin(glfwGetTime())*radius;
		float camY = cos(glfwGetTime())*radius;
		glm::mat4 view;
		//view = glm::translate(view, glm::vec3(0.0f,0.0f,-3.3f));
		//view = glm::lookAt(glm::vec3(camX, 0.0f, camY),		//Position
		//	glm::vec3(0.0f, 0.0f, 0.0f),					//Target
		//	glm::vec3(0.0f,1.0f,0.0f));						//Up
		view = glm::lookAt(cameraPos, cameraPos + cameraFront,cameraUp);
		shader.setMartix("view", view);

		glm::mat4 projection;
		projection = glm::perspective(glm::radians(aspect),(float)SCR_WIDTH/(float)SCR_HEIGHT,0.1f,100.0f );
		shader.setMartix("projection", projection);
		
		for (int i = 0; i < sizeof(cubePositions) / sizeof(cubePositions[0]); ++i) {
			float angle = 20.0f * i;
			glm::mat4 model;
			model = glm::translate(model, cubePositions[i]);
			model = glm::rotate(model, (float)glfwGetTime()*glm::radians(angle), glm::vec3(1.0f, 3.0f, 5.0f));
			shader.setMartix("model", model);

			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//swap & Polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return;
}

bool firstMouse = true;
/*	
	鼠标向左移动，x值减小；向右移动，x值增大
	鼠标向上移动，y值减小；向下移动，y值增大
*/
void mouse_callback(GLFWwindow * window, double xPos, double yPos) {
	if (firstMouse) // this bool variable is initially set to true
	{
		//第一次调用时的xPos,yPos和lastX,lastY相差巨大，需要进行一次调整
		lastX = xPos;	
		lastY = yPos;
		firstMouse = false;
	}
	//std::cout << "xPos:  " << xPos << "yPos:  " << yPos << std::endl;
	float sensitivity = 0.05f;	//敏感度
	float xOffset = xPos - lastX;	//向左移动为负，向右移动为正。
	float yOffset = lastY - yPos;	//向上移动为正，向下移动为负。
	lastX = xPos; 
	lastY = yPos;
	//std::cout << "xOffset:  " << xOffset << "yOffset:  " << yOffset << std::endl;
	xOffset *= sensitivity;
	yOffset *= sensitivity;
	yaw += xOffset  ;
	pitch += yOffset ;
	
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	//std::cout << "front.x:  " << front.x << "front.y:  " << front.y << "front.z:  " << front.z << std::endl;
	cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow * window, double xOffset, double yOffset) {
	if (aspect >= 1.0f && aspect <= 45.0f)
		aspect -= yOffset;	//yOffset代表了滚轮的滚动！
	if (aspect <= 1.0f)
		aspect = 1.0f;
	if (aspect >= 45.0f)
		aspect = 45.0f;
}