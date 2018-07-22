#include <glad\glad.h>
#include <glfw3.h>

//��ѧ��
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*
	����̳�����ֻ������һ����Լ򵥵ĸ�˹ģ��������������ÿ��������ֻ��5��������
	ͨ�����Ÿ���İ뾶���ظ����������ģ�������в������ǾͿ�������ģ����Ч������Ϊģ��������
	�뷺��Ч������������أ�����ģ��Ч�����ܹ���������Ч������Щ������ģ���������벻ͬ��С��ģ
	��kernel����ö����˹������ѡ���Եؽ��Ȩ�ؽ������ʹ�á�����Kalogirou��EpicGames�ĸ�����
	Դ���������ͨ��������˹ģ����������������Ч����
*/


/*Bloom	: ����
	�����Ĺ�Դ����������ͨ�������Դ�����۲��ߵģ���Ϊһ����ʾ����ǿ�ȷ�Χ�����޵ġ�
	һ������ʾ��������������Դ�ķ�ʽ�����ɹ��Σ����εĹ��ڹ�Դ��Χ��
	���ܹ���Ч�ĸ��۲���һ����Щ��Դ���߹��������Ƿǳ����Ĵ����

	����ͨ������Ч�����ɵĹ���(light bleeding)���߹���Ч������ΪBloom������

	Bloomʹ��������������������Ե��Ӿ��У���ΪBloom�����ڸ���һ������ǳ����Ĵ����
	����һ��΢���ʱ�з������е�ʱ��Bloom�����������������Ĺ��ղ�������Χ��Ϸ��Ч����

	Bloom���Ժ�HDR�����Ľ��ʹ�á�һ������������ǣ�
		HDR��ͬ��Bloom����Ϊ����˽����ʹ����Щ���Ȼ������������ȫ��ͬ�ļ������������ڲ�ͬ��Ŀ�ġ�
	������Ĭ�ϵ�8Bit���ȵ�֡��������ʵ��Bloom�����������û��Bloom��Ч����ʹ��HDR��
	
	������HDRʹ��Bloom����Ч��ʵ�֡�

	Ҫʵ��Bloom������������һ������Ⱦһ�����ճ�����Ȼ����ȡ��������HDR��ɫ�������ͽ������ɼ��������ĳ���ͼ��
	����ȡ������������ͼƬ����ģ�����������ӵ�HDR�������档
		��
			1.��Ⱦһ�����ճ���
			2.��ȡ��HDR��ɫ������A
			3.��ȡ�пɼ���������ͼ��B
			4.��B����ģ������
			5.������Ľ����ӵ�A��
		��
	������������һ�����Ľ���������̣�
	
	1.	��ʵ������Ⱦ���ĸ������������Դ�����ǵ�����ֵ�� 1.5 �� 15.0 ֮�䡣
	
		�ǵ�Ӧ��HDR������ᱬ����
	
	2.	������Ҫ����Ⱦ�ĳ�������ȡ������ͼƬ�����ǿ�����Ⱦ�������Σ�Ȼ��ʹ�ò�ͬ����ɫ����
	�ֱ𱣴��ڲ�ͬ��֡�������С������������ǻ�ʹ��һЩС���ɣ�������ȾĿ��(Muliple Render Target MRT)
		MRT�������ǿ���ָ������������Ƭ����ɫ��������������ǿ�����һ����Ⱦ����ȡ������ͼ��

	��������ɫ�������ǰ������ָ��һ������location��ʶ�����������Ǳ�ɿ���һ��Ƭ����ɫ��д�뵽�ĸ���ɫ���壺
		
		layout (location = 0) out vec4 FragColor;
		layout (location = 1) out vec4 BrightColor;
	Ȼ�����ֻ���������ж��λ�ÿ���д���ʱ������á�
		Ϊ��ʹ�ö��Ƭ����ɫ���������������Ҫ�󶨶����ɫ��������֡��������С�����ʱ���Եģ�
	
	����֮ǰ���ǽ���ɫ����������GL_COLOR_ATTACHMENT0�ϣ������а󶨣��������ǿ���ʹ��GL_COLOR_ATTACHMENT1�����󶨶��
			for (unsigned int i = 0; i < 2; i++)
			{
				[......]
				// attach texture to framebuffer
				glFramebufferTexture2D(
					GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
				);
			}  
		���뿴�������������
	
	�������ǽ����ò���ʽ�ĸ���OpenGL����Ҫͨ��glDrawBuffers��Ⱦ������������У�
		����openGLĬ��ֻ��Ⱦ��֡����ĵ�һ����ɫ�����ϣ�
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

	3.	��˹ģ��
		��˹ģ���ǻ��ڸ�˹����
		��˹�����������м䴦��������ʹ������ֵ��ΪȨ��ʹ�ý���������ӵ����������Ȩ��
		�������Ƕ�Ƭ�δ�32*32����������ڲ�����������Ƭ�εľ������ӣ����ǲ��õ�Ȩ��ԽС��
		��ˣ�������Ҫһ����ά��Ȩ���ķ��Σ��������ά��˹���߷�����ȥ��ȡ����
		Ȼ�������ǣ����ÿ������в����Ļ� ������ 32*32 = 1024�Σ��������ǲ����ܵ�

		���˵��ǣ���˹�����и��ǳ���������ԣ����������ǰѶ�ά���̷ֽ�Ϊ������С�ķ��̣�һ������ˮƽȨ�أ���һ��������ֱȨ�ء�

		��������������������ʹ��ˮƽȨ�ؽ���ˮƽģ����Ȼ���ڴ�����������Ӧ�ô�ֱģ����
		����������ԣ������һ���ģ����ǿ��Խ�ʡ�������ŵ����ܣ�
		��Ϊ��������ֻ����32+32�β�����������1024�ˣ������������˹ģ��( two-pass Gaussian Blur)��

		Ҳ����˵��������Ҫ��ͬһ��ͼ������ģ�����Σ����ʹ��֡�������������¡�
		���ǽ�ʵ��һ��Ping-Pong ������������һ�Ի�������
		�������ǰ���һ��֡�������ɫ����Ž���ǰ��֡�������ɫ������ָ����������ͬʱʹ�ò�ͬ����ɫЧ����Ⱦ��
		�����Ͼ��ǲ��ϵ��л�֡���������ȥ���ơ�
		�����������ڳ�������ĵ�һ�������н���ģ����
		Ȼ���ڰѵ�һ��֡�������ɫ����Ž��ڶ���֡�������ģ����
		���ţ����ڶ���֡�������ɫ����Ž���һ��ģ����ѭ��������

	��ɫ����
		��Shader5-7.fragment_Gaussian_Blur�п��Կ��������Ƿֱ���ˮƽ�ʹ�ֱ���в��������������5�β�����
		����Χ�Ƶ�ǰ��Ƭ�ν��У�ע��ˮƽ�ʹ�ֱ�����Ƿ���ġ�
			������ͨ��һ��horizontal ��bool���ͣ���ˮƽ�ʹ�ֱ�ֿ����ġ���
		ͨ����1.0��������Ĵ�С����textureSize�õ�һ��vec2���õ�һ���������ص�ʵ�ʴ�С���Դ���Ϊƫ�ƾ���ĸ��ݡ�
	��������
		������Ҫ��������������������ÿ����ֻ������ɫ��������
			GLuint PingPongFBO[2];
			GLuint PingPongColorBuffer[2];
			glGenFramebuffers(2 , PingPongFBO);
			glGenTextures(2 , PingPongColorBuffer);
			for (int i = 0; i < 2; ++i) {
				glBindFramebuffer(GL_FRAMEBUFFER , PingPongFBO[i]);
				glBindTexture(GL_TEXTURE_2D, PingPongColorBuffer[i]);
		
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);	//�ص㣺ʹ��16λ�ĸ��㻺����
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, PingPongColorBuffer[i], 0);

			}
	�õ�һ��HDR�������������ȡ�����������������һ��֡���壬Ȼ�����ģ������10�Σ�5�δ�ֱ5��ˮƽ����
	
	���ģ���Ľ����ԭ������ϼ��ɣ�
*/

/*
--����------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void renderPlane(Shader shader);
void renderCube(Shader shader, std::vector<glm::vec3> cubePositions, std::vector<glm::vec3> cubeRotateAxis);
void renderLight(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors);
void RenderQuad(Shader shader);

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT, bool gammaCorrection = false);
//HDR
void HDR_Test(GLFWwindow * window);

/*
--����------------------------------
*/
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool hdr = true;
bool hdrKeyPressed = false;
float exposure = 1.0f;
int extract = 0;
Camera camera(cameraPos, WorldUp, yaw, pitch);
int amount = 10;
int main(void) {
	if (!glfwInit()) {
		std::cout << "Initializing Failed" << std::endl;
	}

	//Set
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "5-1 Advanced Lighting", NULL, NULL);
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

	//glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}
	HDR_Test(window);
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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !hdrKeyPressed)
	{
		extract = 1;
		hdrKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		extract = 0;
		hdrKeyPressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (exposure > 0.0f)
			exposure -= 0.001f;
		else
			exposure = 0.0f;

	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		exposure += 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		if (amount > 2)
			amount -= 2;
	}
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		if (amount < 20)
			amount += 2;
	}
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
	std::cout << "extract : " << extract << std::endl;
	camera.ProcessKeyboard(curMovement, deltaTime, false);
}

float cubeVertices[] = {
	// positions         	//Normal				 // texture Coords	
	-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		 0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		 0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		 0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,		 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,		 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,		 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,		 1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,		1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,		 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,		1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,		 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,		 0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,		 0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,		1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,		 0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,		 0.0f, 1.0f
};

float planeVertices[] = {
	// x-z ƽ��
	// positions         	//Normal			 // texture Coords	
	-4.0f, -1.0f, -4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 5.0f,
	4.0f, -1.0f,  4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 0.0f,
	4.0f, -1.0f, -4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 5.0f,

	-4.0f, -1.0f, -4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 5.0f,
	-4.0f, -1.0f,  4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 0.0f,
	
	// x-y ƽ��
	/*-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	-2.0f, -1.0f,	2.0f, 2.0f,			0.0f,  0.0f, 1.0f,

	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	-2.0f,	 2.0f, -1.0f,	0.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f*/
};



void HDR_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader shader("Shader5-7.vertex", "Shader5-7.fragment");
	Shader Blend_HDRshader("Shader5-7.vertex_Blend", "Shader5-7.fragment_Blend");
	Shader blurShader("Shader5-7.vertex_Blend","Shader5-7.fragment_Gaussian_Blur");

	//Texture
	unsigned int planeTexture = loadTexture("wood.png", false);
	unsigned int cubeTexture = loadTexture("container2.png", false);

	//create 2 basic framebuffers	--	PingPong������
	GLuint PingPongFBO[2];
	GLuint PingPongColorBuffer[2];
	glGenFramebuffers(2 , PingPongFBO);
	glGenTextures(2 , PingPongColorBuffer);
	for (int i = 0; i < 2; ++i) {
		glBindFramebuffer(GL_FRAMEBUFFER , PingPongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, PingPongColorBuffer[i]);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);	//�ص㣺ʹ��16λ�ĸ��㻺����
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, PingPongColorBuffer[i], 0);

	}
	
	//FrameBuffer
	GLuint hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// create color buffer ��Ⱦ������������
	GLuint colorBuffers[2];
	glGenTextures(2, colorBuffers); 
	for (int i = 0; i < 2	; ++i) {
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);	//�ص㣺ʹ��16λ�ĸ��㻺����
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);	//
	}
	//����OpenGL��Ⱦ���������������У�
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// create depth buffer (renderbuffer)
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);

	// attach 
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	//������Ϊ��Ⱦ�������������㹻���ڴ�֮�����ǿ��Խ�������Ⱦ���塣
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// shader configuration
	// --------------------
	shader.use();
	shader.setInt("diffuseTexture", 0);
	Blend_HDRshader.use();
	Blend_HDRshader.setInt("hdr_scene", 0);
	Blend_HDRshader.setInt("bloomBlur", 1);
	Blend_HDRshader.setInt("hdr", hdr);
	blurShader.use();
	blurShader.setInt("image",0);
	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		//Draw
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//cube info
		std::vector<glm::vec3> cubePositions;
		cubePositions.push_back(glm::vec3(0.0f, -0.5f, 0.0f));
		cubePositions.push_back(glm::vec3(2.0f, 0.0f, 0.0f));
		cubePositions.push_back(glm::vec3(-2.0f, -0.5f, 1.0f));
		cubePositions.push_back(glm::vec3(2.0f, 2.0f, -1.0f));
		cubePositions.push_back(glm::vec3(-1.0f, 1.0f, -1.0f));
		cubePositions.push_back(glm::vec3(1.0f, 2.0f, -3.0f));

		std::vector<glm::vec3> cubeRotateAxis;
		cubeRotateAxis.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
		cubeRotateAxis.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
		cubeRotateAxis.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		cubeRotateAxis.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
		cubeRotateAxis.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));
		cubeRotateAxis.push_back(glm::vec3(-1.0f, -1.0f, -1.0f));
		// lighting info
		// -------------
		// positions
		std::vector<glm::vec3> lightPositions;
		std::vector<glm::vec3> lightColors;
		lightPositions.push_back(glm::vec3(0.0f, 3.0f, 0.0f)); // back light
		lightPositions.push_back(glm::vec3(-0.4f, 0.9f, 3.0f));
		lightPositions.push_back(glm::vec3(0.0f, 0.8f, 2.0f));
		lightPositions.push_back(glm::vec3(0.8f, 0.7f, -3.0f));
		// colors
		lightColors.push_back(glm::vec3(15.0f, 0.0f, 0.0f));
		lightColors.push_back(glm::vec3(0.0f, 10.0f, 0.0f));
		lightColors.push_back(glm::vec3(0.0f, 0.0f, 7.0f));
		lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));

		// 1. render scene into floating point framebuffer
		// -----------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//��ȾCube
		shader.use();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		shader.setVec3("viewerPos", camera.Position);
		shader.setInt("inverse_normals", false);//���������ڲ������ⲿ
		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
			shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planeTexture);
		renderPlane(shader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		renderCube(shader , cubePositions , cubeRotateAxis);

		//��ȾLight
		light_Shader.use();
		light_Shader.setMartix("view", view);
		light_Shader.setMartix("projection", projection);
		renderLight(light_Shader, lightPositions ,lightColors);

		// 2. ͨ������Ĳ������ǵõ���һ��HDR�����һ����������
		//	���ڣ����ǽ���Ⱦ�õ����������������һ����������Ȼ��������ģ������
		// ---------------------------------------------------------------------------------
		GLboolean horizontal = true, first_iteration = true;
		//int amount = 10;	//ʮ��ģ������5��ˮƽ��5�δ�ֱ
		blurShader.use();
		for (int i = 0; i < amount; ++i) {
			glBindFramebuffer(GL_FRAMEBUFFER, PingPongFBO[horizontal]);
			blurShader.setInt("horizontal", horizontal);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : PingPongColorBuffer[!horizontal]);
			RenderQuad(blurShader);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;	//ע���ڵ�һ����ʱ��ʹ�����Ȼ�����,֮��Ͳ����л�PingPong�Ļ���������ģ����
		}

		// 2. now render floating point color buffer to 2D quad and tonemap HDR colors to 
		//		default framebuffer's (clamped) color range
		// ---------------------------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Blend_HDRshader.use();
		Blend_HDRshader.setFloat1f("exposure", exposure);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, PingPongColorBuffer[!horizontal]);
		RenderQuad(Blend_HDRshader);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

static unsigned int cubeVAO, cubeVBO;
static unsigned int planeVAO, planeVBO;
static unsigned int quadVAO, quadVBO;
static bool CubeisGenerated = false;
static bool PlaneisGenerated = false;
static bool QuadisGenerated = false;

void renderPlane(Shader shader) {
	if (!PlaneisGenerated) {
		//VAO , VBO
		//Cube
		glGenVertexArrays(1, &planeVAO);
		glGenBuffers(1, &planeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(planeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//��������
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//����������
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//������������
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		PlaneisGenerated = true;
	}
	shader.use();
	glBindVertexArray(planeVAO);
	glm::mat4 model = glm::mat4();
	model = glm::scale(model , glm::vec3(3.0f,1.0f,3.0f));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

}
void renderCube(Shader shader , std::vector<glm::vec3> cubePositions , std::vector<glm::vec3> cubeRotateAxis) {
	if (!CubeisGenerated) {	//������û�����ɣ�������������
							//VAO , VBO
							//Cube
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//��������
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//����������
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//������������
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		CubeisGenerated = true;
	}
	shader.use();
	glBindVertexArray(cubeVAO);

	for (int i = 0; i < cubePositions.size(); ++i) {
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, cubePositions[i]);
		model = glm::rotate(model,glm::radians(i * 15.0f) , cubeRotateAxis[i]);
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);

}

void renderLight(Shader shader , std::vector<glm::vec3> lightPositions , std::vector<glm::vec3> lightColors) {
	if (!CubeisGenerated) {	//������û�����ɣ�������������
		//VAO , VBO
		//Cube
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//��������
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//����������
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//������������
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		CubeisGenerated = true;
	}
	//light
	for (unsigned int i = 0; i < lightPositions.size(); i++) {
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, lightPositions[i]);
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		shader.setMartix("model", model);
		shader.setVec3("cubeColor" , lightColors[i]);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void RenderQuad(Shader shader) {
	if (!QuadisGenerated) {
		float quadVertices[] = {
			// positions			// texture Coords
			-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
			1.0f,  1.0f, 0.0f,		1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		};
		// setup plane VAO
		
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindVertexArray(0);

		QuadisGenerated = true;
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para, bool gammaCorrection)
{
	/*
	Gamma�������������ڣ�
	�����ȡ������ɫ���ݲ�����ʵ������ʾ���������ݣ�
	������ʾ����ɫΪCrgb,�����ﱣ�����ɫӦΪ Crgb^(1/2.2)��
	���������κδ���֮�����һ��gamma��������Ϊ Crgb^(1/2.2)^2,�ڽ���gamma���ֻص���Crgb^(1/2.2)�����Ի����
	�����������Crgb����֮�����һ��gamma��������Ϊ Crgb^(1/2.2),�ڽ���gamma���ֻص���Crgb�������ͱ��������Թ�ϵ
	*/
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1) {
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3) {
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4) {
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
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
