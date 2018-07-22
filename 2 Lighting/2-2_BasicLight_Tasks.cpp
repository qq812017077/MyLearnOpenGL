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
#include "Camera.h"

/*
	！！！！！！！！
		注意此节使用的是点光源。

		点光源的效果！！！
*/

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow * window, double xPos, double yPos);
void scroll_callback(GLFWwindow * window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);


//相机属性
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static float lastX = 400.0f, lastY = 300.0f;
float pitch, yaw = -90.0f;
float aspect = 45.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

Camera camera(cameraPos, cameraUp, yaw, 0.0f);//定义了一个相机, front 由yaw 和 pitch确定

bool openSpecular = true;
int lastStatus; 
void Light_Task1(GLFWwindow * window);
void Light_Task2(GLFWwindow * window);
void Light_Task3(GLFWwindow * window);
void Light_Task4(GLFWwindow * window);
int main(void) {
	if (!glfwInit()) {
		std::cout << "Failed to Initialization" << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2-2 BasicLight", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);	//光标回调函数
	glfwSetScrollCallback(window, scroll_callback);		//鼠标滚轮监测
														//loading GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Light_Task4(window);
	glfwTerminate();
	return 0;
}


void ProcessInput(GLFWwindow * window , float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	Camera_Movement camera_Movement;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && lastStatus == GLFW_PRESS) {
		//使用空格来开关镜面反射效果
		if (openSpecular) openSpecular = false;
		else openSpecular = true;
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera_Movement = Camera_Movement::FORWARD;
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera_Movement = Camera_Movement::BACKWARD;
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera_Movement = Camera_Movement::LEFT;
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera_Movement = Camera_Movement::RIGHT;
	else
		camera_Movement = Camera_Movement::NONE;
	//std::cout << deltaTime << std::endl;
	camera.ProcessKeyboard(camera_Movement, deltaTime, false);

	return;
}


/*包括了每个顶点及其法向量
*/
float cubeVertices[] = {
	//每个面6个顶点，
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};


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

	camera.ProcessMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow * window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll(xOffset);
}


/*
Try to move the light source around the scene over time using cos and sin.

尝试移动灯光，
*/
void Light_Task1(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);//深度使能

							//Shader shader("shader2-1.vertex", "shader2-1.fragment");
	Shader lightShader("Shader2-2.vertex", "Shader2-2.fragment_Task");
	Shader lampShader("Shader2-2.vertex", "lamp.fragment");
	//Vertices
	unsigned int VAO, VBO, lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//之前已经传过数据了，这里就不用重复操作
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//while
	glm::vec3 lightPos(1.0f, 1.0f, -2.0f);
	lightShader.use();
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	lightShader.setVec3("objectColor", objectColor);
	lightShader.setVec3("lightColor", lightColor);
	lightShader.setVec3("lightPos", lightPos);
	float lastTime{};
	while (!glfwWindowShouldClose(window)) {

		float deltaTime, curTime ;
		curTime = glfwGetTime();
		deltaTime = curTime - lastTime;
		lastTime = curTime;

		//Process Input
		ProcessInput(window, deltaTime);

		lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
		lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;
		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightShader.setVec3("viewerPos", camera.Position);

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 curLightPos;
		view = camera.GetViewMatrix();	//直接获取相机
		projection = glm::perspective(glm::radians(aspect), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		lampShader.setMartix("projection", projection);

		//渲染lamp
		lampShader.use();
		lampShader.setMartix("view", view);
		lampShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));	//缩放
		lampShader.setMartix("model", model);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//渲染其他物体.
		lightShader.use();
		lightShader.setVec3("lightPos", lightPos);	//更新lamp的位置。
		lightShader.setBool("openSpecular", openSpecular);

		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::mat4();
			lightShader.setMartix("model", model);//使用标准model,不进行任何操作!
		lightShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));	//适应于法向量的变换矩阵
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//swap & Polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return;
}

/*
	Play around with different ambient, diffuse and specular strengths and see how they impact the result. 
	Also experiment with the shininess factor. Try to comprehend why certain values have a certain visual output.
*/
void Light_Task2(GLFWwindow * window) {
	/*
	尝试使用不同的环境光、散射镜面强度，观察光照效果。

		物体最终的显示即：
			result = lightColor * objectColor.
		环境光-----lightColor：
			没什么好说的，就是一个常量因子，针对所有的粒子一定。
				
				直接对应到光照颜色(lightColor)上面，反映了一定强弱的光照，
			其值越大，表示环境光越强，
		
		漫反射-----
			其对于不同的顶点效果不同，主要看光到该顶点的方向和其法向量的夹角。
				
				lightDirec * normalDirec
			夹角小，则反射的光照颜色(lightColor)强度大。

		镜面反射----
			其对于不同顶点效果不同，主要根据观察者的观察视角发生变化。
				
					主要看观察者跟光的反射角度之间的夹角。
					因为如果光的镜面反射光强度一致，但是如果人眼方向与反射光夹角越小，接收到的光应该更多，也就更亮。
				同时，通过取幂值，可以极大的拉开差两个点的差距，从而产生光斑的效果。
	*/
}

/*
	Do Phong shading in view space instead of world space
	即在观察空间 而不是世界控件中计算冯氏光照 

	在观察空间中，观察者处于 原点位置！！！

	因此，需要将其他的：
		包括法向量，物体位置，光位置，均进行转换（转换到视图空间）
		！
			注意：我们定义的位置的坐标（比如这里的lightPos）即是世界坐标系的坐标（该坐标被用于构建model）！
			这里的法向量在进行正规矩阵的处理后，也应该是最终的产物。
			（这里的正规矩阵是在 view * model 的整体上进行的变换！！）

			这里有个疑惑，因为view 的 变换矩阵应该只是进行了 移动和旋转，没有进行不规则缩放操作！！因此可能放到最外面直接乘以应该也OK！

			（物体本身没有进行移动操作）
*/
void Light_Task3(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);

	Shader objectShader("Shader2-2.vertex_Task2", "Shader2-2.fragment_Task2");
	Shader lampShader("lamp.vertex", "lamp.fragment");
	//创建顶点数组和对象缓冲区
	unsigned int objectVAO, VBO, lightVAO;
	
	glGenVertexArrays(1, &lightVAO);
	glGenVertexArrays(1,&objectVAO);
	glGenBuffers(1,&VBO);

	//Object objectVAO
	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertices),cubeVertices,GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	
	glBindVertexArray(lightVAO);	//对于光源来讲，不需要法向量数据
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//颜色数据
	glm::vec3 lightPos(1.0f,1.0f,-2.0f);//
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	objectShader.use();
	objectShader.setVec3("lightPos", lightPos);
	objectShader.setVec3("objectColor", objectColor);
	objectShader.setVec3("lightColor", lightColor);
	float lastTime{};
	while (!glfwWindowShouldClose(window)) {

		//Process Input
		float deltaTime = 0, curTime{};
		curTime = glfwGetTime();
		deltaTime = curTime - lastTime;
		lastTime = curTime;
		ProcessInput(window , deltaTime);	//获取输入，并更新相机位置！！
		

		//Clear Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Render
		glm::mat4 model, view, projection;

		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//lamp
		lampShader.use();
		lampShader.setMartix("view", view);
		lampShader.setMartix("projection", projection);
		model = glm::translate(model,lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lampShader.setMartix("model",model);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES,0,36);
		//object
		objectShader.use();

		objectShader.setBool("openSpecular", openSpecular);
		objectShader.setMartix("view", view);
		objectShader.setMartix("projection", projection);
		model = glm::mat4();//初始化
		objectShader.setMartix("model", model);
		objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(view * model)));
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
/*
	Implement Gouraud shading instead of Phong shading. 
	使用Gouraud着色，而不是冯氏着色。
		结果会有一点点不同。

		我们可以看到三角形的分界线！！ 即一个明显的对角线分界线（即两个片），同时有一个顶点非常亮。
		
		可以看到有左上 右下两个三角形，右上角的顶点被点亮而剩下的没有，因此中间的片段进行线性插值，
		也就是说：
			只有初始设置的顶点会被点亮！！！因为我们直接赋予的颜色只是顶点部分，其余的部分才是线性插值的结果！
			
			两个三角形发光的部分重合，从而出现的光亮的对角线!

		最应该注意的一点是，在顶点着色器进行该处理的后果就是只能对控制的顶点进行亮度处理，而其余的亮度都将进行线性插值。

		而在fragment着色器中，则可以对FragPos进行处理。 那么就很可能是，圆形光斑只能靠片段着色器生成（除非顶点着色器中有大量的点）
		
*/
void Light_Task4(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);

	Shader objectShader("Shader2-2.vertex_Task3", "Shader2-2.fragment_Task3");
	Shader lampShader("lamp.vertex", "lamp.fragment");
	//创建顶点数组和对象缓冲区
	unsigned int objectVAO, VBO, lightVAO;

	glGenVertexArrays(1, &lightVAO);
	glGenVertexArrays(1, &objectVAO);
	glGenBuffers(1, &VBO);

	//Object objectVAO
	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	glBindVertexArray(lightVAO);	//对于光源来讲，不需要法向量数据
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//颜色数据
	glm::vec3 lightPos(1.0f, 1.0f, 2.0f);//
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	objectShader.use();
	objectShader.setVec3("lightPos", lightPos);
	objectShader.setVec3("objectColor", objectColor);
	objectShader.setVec3("lightColor", lightColor);
	float lastTime{};
	while (!glfwWindowShouldClose(window)) {

		//Process Input
		float deltaTime = 0, curTime{};
		curTime = glfwGetTime();
		deltaTime = curTime - lastTime;
		lastTime = curTime;
		ProcessInput(window, deltaTime);	//获取输入，并更新相机位置！！


											//Clear Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Render
		glm::mat4 model, view, projection;

		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//lamp
		lampShader.use();
		lampShader.setMartix("view", view);
		lampShader.setMartix("projection", projection);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lampShader.setMartix("model", model);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//object
		objectShader.use();
		objectShader.setVec3("viewerPos", camera.Position);
		objectShader.setBool("openSpecular", openSpecular);
		objectShader.setMartix("view", view);
		objectShader.setMartix("projection", projection);
		model = glm::mat4();//初始化
		objectShader.setMartix("model", model);
		objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}