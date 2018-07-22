#include <glad\glad.h>
#include <glfw3.h>

//��ѧ��
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//ͼ���
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "Shaders.h"
#include "Camera.h"

/*
	����������������
		ע��˽�ʹ�õ��ǵ��Դ��

		���Դ��Ч��������
*/

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow * window, double xPos, double yPos);
void scroll_callback(GLFWwindow * window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);


//�������
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static float lastX = 400.0f, lastY = 300.0f;
float pitch, yaw = -90.0f;
float aspect = 45.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

Camera camera(cameraPos, cameraUp, yaw, 0.0f);//������һ�����, front ��yaw �� pitchȷ��

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
	glfwSetCursorPosCallback(window, mouse_callback);	//���ص�����
	glfwSetScrollCallback(window, scroll_callback);		//�����ּ��
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
		//ʹ�ÿո������ؾ��淴��Ч��
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


/*������ÿ�����㼰�䷨����
*/
float cubeVertices[] = {
	//ÿ����6�����㣬
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
��������ƶ���xֵ��С�������ƶ���xֵ����
��������ƶ���yֵ��С�������ƶ���yֵ����
*/
void mouse_callback(GLFWwindow * window, double xPos, double yPos) {
	if (firstMouse) // this bool variable is initially set to true
	{
		//��һ�ε���ʱ��xPos,yPos��lastX,lastY���޴���Ҫ����һ�ε���
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	//std::cout << "xPos:  " << xPos << "yPos:  " << yPos << std::endl;
	float sensitivity = 0.05f;	//���ж�
	float xOffset = xPos - lastX;	//�����ƶ�Ϊ���������ƶ�Ϊ����
	float yOffset = lastY - yPos;	//�����ƶ�Ϊ���������ƶ�Ϊ����

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow * window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll(xOffset);
}


/*
Try to move the light source around the scene over time using cos and sin.

�����ƶ��ƹ⣬
*/
void Light_Task1(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);//���ʹ��

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
	//֮ǰ�Ѿ����������ˣ�����Ͳ����ظ�����
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
		view = camera.GetViewMatrix();	//ֱ�ӻ�ȡ���
		projection = glm::perspective(glm::radians(aspect), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		lampShader.setMartix("projection", projection);

		//��Ⱦlamp
		lampShader.use();
		lampShader.setMartix("view", view);
		lampShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));	//����
		lampShader.setMartix("model", model);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//��Ⱦ��������.
		lightShader.use();
		lightShader.setVec3("lightPos", lightPos);	//����lamp��λ�á�
		lightShader.setBool("openSpecular", openSpecular);

		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::mat4();
			lightShader.setMartix("model", model);//ʹ�ñ�׼model,�������κβ���!
		lightShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));	//��Ӧ�ڷ������ı任����
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
	����ʹ�ò�ͬ�Ļ����⡢ɢ�侵��ǿ�ȣ��۲����Ч����

		�������յ���ʾ����
			result = lightColor * objectColor.
		������-----lightColor��
			ûʲô��˵�ģ�����һ���������ӣ�������е�����һ����
				
				ֱ�Ӷ�Ӧ��������ɫ(lightColor)���棬��ӳ��һ��ǿ���Ĺ��գ�
			��ֵԽ�󣬱�ʾ������Խǿ��
		
		������-----
			����ڲ�ͬ�Ķ���Ч����ͬ����Ҫ���⵽�ö���ķ�����䷨�����ļнǡ�
				
				lightDirec * normalDirec
			�н�С������Ĺ�����ɫ(lightColor)ǿ�ȴ�

		���淴��----
			����ڲ�ͬ����Ч����ͬ����Ҫ���ݹ۲��ߵĹ۲��ӽǷ����仯��
				
					��Ҫ���۲��߸���ķ���Ƕ�֮��ļнǡ�
					��Ϊ�����ľ��淴���ǿ��һ�£�����������۷����뷴���н�ԽС�����յ��Ĺ�Ӧ�ø��࣬Ҳ�͸�����
				ͬʱ��ͨ��ȡ��ֵ�����Լ����������������Ĳ�࣬�Ӷ�������ߵ�Ч����
	*/
}

/*
	Do Phong shading in view space instead of world space
	���ڹ۲�ռ� ����������ؼ��м�����Ϲ��� 

	�ڹ۲�ռ��У��۲��ߴ��� ԭ��λ�ã�����

	��ˣ���Ҫ�������ģ�
		����������������λ�ã���λ�ã�������ת����ת������ͼ�ռ䣩
		��
			ע�⣺���Ƕ����λ�õ����꣨���������lightPos��������������ϵ�����꣨�����걻���ڹ���model����
			����ķ������ڽ����������Ĵ����ҲӦ�������յĲ��
			������������������ view * model �������Ͻ��еı任������

			�����и��ɻ���Ϊview �� �任����Ӧ��ֻ�ǽ����� �ƶ�����ת��û�н��в��������Ų���������˿��ܷŵ�������ֱ�ӳ���Ӧ��ҲOK��

			�����屾��û�н����ƶ�������
*/
void Light_Task3(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);

	Shader objectShader("Shader2-2.vertex_Task2", "Shader2-2.fragment_Task2");
	Shader lampShader("lamp.vertex", "lamp.fragment");
	//������������Ͷ��󻺳���
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

	
	glBindVertexArray(lightVAO);	//���ڹ�Դ����������Ҫ����������
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//��ɫ����
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
		ProcessInput(window , deltaTime);	//��ȡ���룬���������λ�ã���
		

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
		model = glm::mat4();//��ʼ��
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
	ʹ��Gouraud��ɫ�������Ƿ�����ɫ��
		�������һ��㲻ͬ��

		���ǿ��Կ��������εķֽ��ߣ��� ��һ�����ԵĶԽ��߷ֽ��ߣ�������Ƭ����ͬʱ��һ������ǳ�����
		
		���Կ��������� �������������Σ����ϽǵĶ��㱻������ʣ�µ�û�У�����м��Ƭ�ν������Բ�ֵ��
		Ҳ����˵��
			ֻ�г�ʼ���õĶ���ᱻ������������Ϊ����ֱ�Ӹ������ɫֻ�Ƕ��㲿�֣�����Ĳ��ֲ������Բ�ֵ�Ľ����
			
			���������η���Ĳ����غϣ��Ӷ����ֵĹ����ĶԽ���!

		��Ӧ��ע���һ���ǣ��ڶ�����ɫ�����иô���ĺ������ֻ�ܶԿ��ƵĶ���������ȴ�������������ȶ����������Բ�ֵ��

		����fragment��ɫ���У�����Զ�FragPos���д��� ��ô�ͺܿ����ǣ�Բ�ι��ֻ�ܿ�Ƭ����ɫ�����ɣ����Ƕ�����ɫ�����д����ĵ㣩
		
*/
void Light_Task4(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);

	Shader objectShader("Shader2-2.vertex_Task3", "Shader2-2.fragment_Task3");
	Shader lampShader("lamp.vertex", "lamp.fragment");
	//������������Ͷ��󻺳���
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


	glBindVertexArray(lightVAO);	//���ڹ�Դ����������Ҫ����������
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//��ɫ����
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
		ProcessInput(window, deltaTime);	//��ȡ���룬���������λ�ã���


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
		model = glm::mat4();//��ʼ��
		objectShader.setMartix("model", model);
		objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		glBindVertexArray(objectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}