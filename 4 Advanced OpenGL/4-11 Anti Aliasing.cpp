#include <glad\glad.h>
#include <glfw3.h>

//��ѧ��
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <map>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//ģ������
#include "Model.h"

/*Anti-Aliasing
	��ѧϰ��Ⱦ����;�У�����ܻ�ʱ��ʱ����ģ�ͱ�Ե�о�ݵ������
	��Щ��ݱ�Ե(Jagged Edges)�Ĳ����͹�դ������������ת��ΪƬ�εķ�ʽ�йء�

	����(Aliasing)��
	�кܶ��ֿ���ݣ�Anti-aliasing��Ҳ����Ϊ���������ļ����ܹ��������ǻ����������󣬴Ӷ�������ƽ���ı�Ե��

	���ز��������(Multisample Anti-aliasing, MSAA)��
		
		���ȣ�������Ҫ����OpenGL�Ĺ�դ�����ڲ�������
*/

/*��դ��/��դ������
	��դ���������մ�����Ķ���֮��Ƭ�γ���֮ǰ�������㷨�͹��̵���ϡ�
		��һ��ͼԪ�����ж�����Ϊ���룬������ת��Ϊһϵ�е�Ƭ�Ρ�

		�����϶�������������������꣬����Ƭ�β����ԣ���Ϊ���Ǳ���Ļ�ķֱ�����Լ����

		��˶������굽Ƭ�δ�������һ��һ��ӳ�䡣
		���Թ�դ�����ò���ĳ�ַ�ʽ������
			ÿһ�������������ڵ�Ƭ��/��Ļ���ꡣ

	����֪����Ļ�����ع��ɵģ�ÿ�����أ����飩�����Ķ���һ��������(Sample Point)��
	���ᱻ�����������ͼ���Ƿ��ڸ���ĳ�����ء�
		ͼ��ı�Ե������һ������ס��ĳЩ���أ�����û����ס�����㡣��˾Ͳ��ᱻ��Ƭ����ɫ��Ӱ��
		������Ļ�������������ƣ���Щ��Ե�������ܹ�����Ⱦ����������Щ�򲻻ᡣ
		�����������ʹ���˲��⻬�ı�Ե����ȾͼԪ������֮ǰ���۵��ľ�ݱ�Ե��

	���ز����Ĺ���:
		�������Ĳ������Ϊ��������㡣���ǲ���ʹ���������ĵĵ�һ�����㣬
		ȡ����֮�������ض�ͼ�����е�4���Ӳ�����(Subsample)��
		���ǽ�����Щ�Ӳ��������������ص��ڸǶȡ���Ȼ����Ҳ��ζ����ɫ����Ĵ�С�������Ӳ���������Ӷ����ӡ�

		���������������������ģ�����Ĳ������ܴ�������ȷ���ڸ��ʡ�

	���ز�����MSAA�������Ĺ�����ʽ�ǣ�
		�����������ڸ��˶��ٸ��Ӳ����㣬��ÿ��ͼԪ�У�ÿ������ֻ����һ��Ƭ����ɫ����
		Ƭ����ɫ����ʹ�õĶ������ݻ��ֵ��ÿ�����ص����ģ�
		
					�����õ��Ľ����ɫ�ᱻ������ÿ�����ڸ�ס���Ӳ������С��� ע���Ǳ��ڸǵ�
		����ɫ�������������ͼԪ��������ɫ����ʱ�����е���Щ��ɫ������
		ÿ�������ڲ�ƽ��������Ϊ��ͼ��4����������ֻ��2�����ڸ�ס�ˣ�
		������ص���ɫ��������������ɫ�������������������ɫ������������ɫ����ƽ��ֵ�������γ�һ�ֵ���ɫ��

	��ˣ�
		һ������������и���Ĳ����㱻�ڸǣ�������ɫԽ�ӽ��ŵ���ɫ��
		��Խ�ٵ��Ӳ����㱻�����������ǣ���ô���ܵ������ε�Ӱ���ԽС��
*/

/*OpenGL�е�MSAA
	���������Ҫ��OpenGL��ʹ��MSAA,������Ҫʹ��һ����ɫ������,���ܹ�Ϊ�������ر�������ɫֵ��
		����Ϊ���ز�����Ҫ����Ϊÿ�������㱣����ɫ��
	���������Ҫһ�������͵Ļ����������ܱ����ض������Ķ��ز������������������ز�������

	������Ĵ���ϵͳ�ṩ������һ�����ز���������,�������Ĭ�ϵĻ�������
		GLFWҲ��������������ܣ�
		������Ҫ���ľ��Ǹ���GLFW������Ҫʹ�ô�N��������Ķ��ز���������
		ͨ���ڴ�������֮ǰ������glfwWindowHint������ʹ����������ɫ��������

			glfwWindowHint(GLFW_SAMPLES, 4);

	֮�����Ǵ������ڣ���ʱ��ʹ��ÿ����Ļ��������ĸ���������ɫ��������
	GLFWҲ�Զ��Ĵ���һ�����ĸ��Ӳ��������Ⱥ�ģ�建������
	����ζ�����л������Ĵ�С�������ı���

		�����ʱ��OpenGL���������Զ�ʹ�ܶ��ز�����������������ֶ�����һ�£�
			glEnable(GL_MULTISAMPLE);  

		ʵ�ʵĶ��ز����㷨ʵ�����˹�դ�У�������Ҫ���ľ���ʹ�ܣ�

*/

/*Off-screen MSAA ����MSAA
	����GLFW�����˴������ز������壬����MSAA�ǳ��򵥡�Ȼ����
	���������Ҫʹ�������Լ���֡����������������Ⱦ����ô���Ǿͱ���Ҫ�Լ��������ɶ��ز��������ˡ�
	������������Ҫ��������Ⱦ���ʹ�ã�

		PS��������Ⱦʵ�����ǰѻ�����Ⱦ�����������档������������������Ӿ���

	�����ַ������������������ز���������

	���ز���������
		������ɫ������

	���ز�����Ⱦ�������
		������Ⱥ�ģ�建����

	������ʽ��
		1.�һ�����ز�����֡����������ɫ+���+ģ�壺 ע���ö��ز���������
			������ɫ��
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
			������Ⱦ�������
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
		2.�һ���н�֡����������ͨ�Ļ�����������ת�����ݣ�
			// we only need a color buffer
				unsigned int screenTexture;
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	

				screenShader.setInt("screenTexture", 0);//
		3.�������������ز���������
				glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
				draw...

		4.�����ز�������������ɫ���ݻ������н黺������
			glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		5.����Ļ�����Ĭ�ϻ�������ʹ��һ��������������������������������ϡ�
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, screenTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
*/

/*
	����ķ�ʽ�е��ź���
		���ǻ�����Ҫ�����ݷ���һ����ͨ�Ļ�������ÿ�����ض��ǵ������㣩

		GLSL�ṩ�˶����ѡ�������������ɫ���л�ȡ���еĲ�����ɼ������
			�������ܹ�������ͼ���ÿ�����������в�����
			Ҫ���ȡÿ������������ɫֵ������Ҫ��
			
			������uniform����������Ϊsampler2DMS��������ƽ��ʹ�õ�sampler2D��
					uniform sampler2DMS screenTextureMS;
			ʹ��texelFetch�������ܹ���ȡÿ������������ɫֵ�ˣ�
				vec4 colorSample = texelFetch(screenTextureMS, TexCoords, 3);  // ��4��������
				
*/
unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para);
unsigned int loadCubeMap(std::vector<std::string> texture_faces);
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void Aliasing_Cube(GLFWwindow * window);
//off aliasing
void Off_Aliasing(GLFWwindow * window);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;



glm::vec3 cameraPos(0.0f, 1.0f, 3.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool openEffect = true;
int lastStatus;

Camera camera(cameraPos, WorldUp, yaw, pitch);

int main(void) {
	if (!glfwInit()) {
		std::cout << "Initializing Failed" << std::endl;
	}

	//Set
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_SAMPLES, 4);//����

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3-3_Model", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	//ע�ỷ��
	glfwMakeContextCurrent(window);
	//��ʼ���ص�
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//�ر�����
	glfwSetCursorPosCallback(window, mouse_callback);				//�����ص�
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Off_Aliasing(window);
	glfwTerminate();

}

/*
��������ƶ���xֵ��С�������ƶ���xֵ����
��������ƶ���yֵ��С�������ƶ���yֵ����
*/
bool firstMouse = true;
static float lastX = 400.0f, lastY = 300.0f;
void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
	std::cout << xPos << "   " << yPos << std::endl;
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
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll(yOffset);
}
void ProcessInput(GLFWwindow * window, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && lastStatus == GLFW_PRESS) {
		//ʹ�ÿո������ؾ��淴��Ч��
		if (openEffect) { 
			openEffect = false; 
			glDisable(GL_MULTISAMPLE);
		}
		else {
			openEffect = true;
			glEnable(GL_MULTISAMPLE);
		}
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);

	//�����������
	Camera_Movement curMovement = Camera_Movement::NONE;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		curMovement = Camera_Movement::FORWARD;
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		curMovement = Camera_Movement::LEFT;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		curMovement = Camera_Movement::BACKWARD;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		curMovement = Camera_Movement::RIGHT;
	}

	camera.ProcessKeyboard(curMovement, deltaTime, false);
}


float cubeVertices[] = {
	// positions          // texture Coords		//Normal
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,		  0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,		 0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,		  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,		  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,		 0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,		 0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,		 0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,		  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,		 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,		 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,		 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,		  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,		  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,		  0.0f,  1.0f,  0.0f
};
GLfloat newCubeVertices[] = {
	// Positions       
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,

	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,

	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,

	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f
};
void Aliasing_Cube(GLFWwindow * window) {
	Shader shader("Shader4-11.vertex", "Shader4-11.fragment");
	//glEnable(GL_MULTISAMPLE);//enable ���ز���
	glDisable(GL_MULTISAMPLE);
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1,&VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(newCubeVertices),&newCubeVertices[0],GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0 , 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		glEnable(GL_DEPTH_TEST);
		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		// render
		// ------
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		//first pass
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		model = glm::mat4();
		shader.setMartix("model", model);
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES,0,36);
		glBindVertexArray(0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Off_Aliasing(GLFWwindow * window) {

	Shader shader("Shader4-11.vertex", "Shader4-11.fragment");
	Shader screenShader("Shader.vertex_canvas","Shader.fragment_canvas");
	//����֡����
	unsigned int frameBufferObject;
	glGenFramebuffers(1, &frameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER,frameBufferObject);

	//��ɫ������
	int samples = 4;
	unsigned int multiSample_Buffer ;
	glGenTextures(1, &multiSample_Buffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multiSample_Buffer);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples,GL_RGB, SCR_WIDTH , SCR_HEIGHT , GL_TRUE);
	/*
		����1��Ŀ��
		����2����������ϣ����ͼ���е�������
		����6��GL_TRUE��ͼ�񽫻��ÿ������ʹ����ͬ������λ���Լ���ͬ�������Ӳ����������
	*/
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//��ǰ�󶨵�֡�������ھ�����һ������ͼ����ʽ�Ķ��ز�����ɫ���塣
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D_MULTISAMPLE , multiSample_Buffer , 0);
	
	//��Ⱥ�ģ�建����
	unsigned int renderBufferObject;
	glGenRenderbuffers(1, &renderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples ,GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);
	//������Ϊ��Ⱦ�������������㹻���ڴ�֮�����ǿ��Խ�������Ⱦ���塣
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	// configure second post-processing framebuffer
	unsigned int intermediateFBO;
	glGenFramebuffers(1, &intermediateFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
	// create a color attachment texture
	unsigned int screenTexture;
	glGenTextures(1, &screenTexture);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// we only need a color buffer

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*
	���������Ƶ�һ����ͼ�ϣ�������Ҫ���²���;
	1.��Ⱦ�������µ�֡�����У��ǵ��Ȱ�Ϊ��ǰ�����֡��������
	2.��Ĭ�ϵ�֡����
	3.����һ�����������Ⱦ��Ļ���ı��Σ���ʹ���µ�֡����������ɫ������Ϊ�����ͼ��
	*/

	float canvasVertices[] = {
		/*
		ע����������Ͷ��������������������:(bottom-left: 0,0    top-right: 1,1)
		*/
		//right bottom       // texture 
		1.0f,  1.0f, 0.0f,	1.0f,1.0f,
		-1.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		1.0f, -1.0f, 0.0f,	1.0f, 0.0f,

		//left top
		1.0f, 1.0f,  0.0f,	1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f,	0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f
	};
	// quad VAO
	unsigned int canvasVAO, canvasVBO;
	glGenVertexArrays(1, &canvasVAO);
	glGenBuffers(1, &canvasVBO);
	glBindVertexArray(canvasVAO);
	glBindBuffer(GL_ARRAY_BUFFER, canvasVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(canvasVertices), canvasVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	screenShader.setInt("screenTexture",0);

	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(newCubeVertices), &newCubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	
	/*��һ����4����Ļ�ռ������������Դ�����Ƶ�һ��ͬ����4����Ļ�ռ������������Ŀ�������С�
		Դ����  -> Ŀ������
	*/
	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		// render
		// ------
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		
		//first pass
		
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		
		//DrawScence
		shader.use();
		model = glm::mat4();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		shader.setMartix("model", model);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		//second pass
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferObject);	//���Զ���֡��������ȡ
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);				//��������Ļ��ʾ������
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		//third pass
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		screenShader.use();
		glBindVertexArray(canvasVAO);
		glDisable(GL_DEPTH_TEST);	//�ڻ��Ƽ򵥻������ı��Σ���ʱ�����ǲ���Ҫ������Ȳ��ԣ�
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, screenTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}


inline void MyBindUniform(unsigned int shaderID, const char * uniformBlockName, unsigned int bindingPoint) {
	unsigned int UniformBlockIndex = glGetUniformBlockIndex(shaderID, uniformBlockName);
	glUniformBlockBinding(shaderID, UniformBlockIndex, bindingPoint);
	return;
}

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Tex_Wrap_Para);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Tex_Wrap_Para);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
