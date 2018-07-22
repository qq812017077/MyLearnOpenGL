/*
	�������⣺
		�̳���˵����sRGB��������Զ�����gamma������Ȼ���˴�����֮���ƺ���Ч��
		��
			��shader�е�gamma�������ȫ����̬����
			
			����Ϊ����B����gamma��ʹ�ý����������
						�ر�gamma��ʹ��ԭ��������
				���ս̵̳�������������Ӧ�ñ䰵����ɫ������2.2���ݲ�����

				Ȼ��ʵ���ϲ�û���κα仯����
				��֪��������ںδ���
*/

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

/* Gamma
		٤��У����
		�����Ǽ���������е�������ɫ��������Ҫ������ʾ����ʾ���ϡ�
		�ڹ�ȥ������ͼ����ʾ��������������߹���ʾ���� cathode-ray tube (CRT) monitors��

		������ʾ����һ�������ص��ǣ������������ѹ���������������ȣ�
			�ӱ������ѹ�����һ����Լ2.2���ݵĹ�ϵ������ֵ���ⱻ��Ϊһ����ʾ����Gamma��٤��

	���䣺
	GammaҲ�лҶ�ϵ����ÿ����ʾ�豸�����Լ���Gammaֵ��������ͬ����һ����ʽ��
			�豸������� = ��ѹ��Gamma���ݣ��κ��豸Gamma�����϶��������1������1��һ�����������״̬��
		��������״̬�ǣ������ѹ�����ȶ�����0��1�����䣬��ô���ٵ�ѹ�͵��ڶ������ȡ�
		����CRT��Gammaͨ��Ϊ2.2�������������� = �����ѹ��2.2���ݣ�����Դӱ��ڵڶ���ͼ��
		����Gamma2.2ʵ����ʾ�������ܻ��Ԥ�ڰ����෴Gamma0.45�ͻ������Ԥ������
		����㽫Gamma0.45���ӵ�Gamma2.2����ʾ�豸�ϣ�����ƫ������ʾЧ������У��������򵥵�˼·���Ǳ��ڵĺ���

		��������֪������ǡ�ú�CRT����ʾ�������Ƶ�ָ����ϵ�ǳ�ƥ�䡣

	���۸�֪�����������������Ȳ�̫ƥ�䡣
	�����۶԰����������жȸߣ�
		���ʵ���ϣ�������������һ��������������ֳ����Ŀ��ܷ��˼�����

		�ԱȽ���������£�
				���۸�֪���ȣ�	0.0		0.1		0.2		0.3		0.4		0.5		0.6		0.7		0.8		
				ʵ���������ȣ�	0.0		0.1										0.2						
			��ʵ����������������һ����������������Ѿ���0.6 0.7 ������,����������˺ö౶.(��Ϊ�����ڵ������»�Ƚ�����)


		��Ϊ���ۿ������ǵ�һ�еĿ̶ȣ�����ֱ�����죬��ʾ��Ҳ��ʹ��ָ����ϵ����ʾ�����ɫ��
			����ԭʼ�������ȱ�ӳ�䵽 ��һ�еķ�����������ɫ����Ϊ�����������á�
			�������ǻ�ʹ�ù�ʽ��ģ�����۵Ŀ̶ȹ�ϵ������ʵ����������ڸ�����õ����飩
		
		���ַ����Ե�ӳ���ȷ�����ȿ��������ã���������Ⱦͼ���ʱ��������һ�����⣡
			
			���������ǵ�Ӧ�������õ���ɫ�����ȶ��ǻ������ǹ۲쵽�ģ�
			������Щѡ��ʵ����Ҳ�Ƿ���������/��ɫѡ�


		֮ǰ����һֱ���������Կռ��й�������ʵ��������һֱ����������ʾ���������ɫ�ռ䶨�����ɫ�ռ䡣
			����˵������֮ǰʹ�õ�����ֵ�������������ǲ���ȷ��
		
		��ʾ������ʾ������ͼ�������ͼ�����С��������ͬ�ģ�������������Ҳ����ͬ�ģ�ֻ���м����Ȳ��ֻᱻѹ����
			
		��Ϊ�����м����ȶ������Կռ��������ģ����������Ժ�ʵ���϶��᲻��ȷ��

	��
*/

/*Gamma  Correction
	٤��У��
		٤��У����˼·��Ӧ����ʾ��٤��ĵ��������յ���ɫ������ʵ�������֮ǰ����
		��������ɫ��ʾ����������ʱ���ÿ����ɫ��������������ת��Gamma���ߣ�
		����Ӧ���˼�����Gamma�Ժ����յ���ɫ�����Ϊ���Եġ�

		����٤������ϵ��Ϊ2.2.
			����������Ҫ���һ����ɫ��0.5��0.0��0.0��0.0��

			δ����Gama������
				���Ϊ (0.5, 0.0, 0.0, 0.0)^2.2 --> (0.218, 0.0, 0.0, 0.0)	-->�䰵��

			����Gama������
				�����ǰ����Gama������
				������ (0.5, 0.0, 0.0, 0.0)^1/2.2 --> (0.73, 0.0, 0.0, 0.0)	-->������
				�Խ���ֵ���������
					(0.73, 0.0, 0.0, 0.0)^2.2 --> (0.5, 0.0, 0.0, 0.0)	-->�ָ�

		���Gama���������ã����任ת��Ϊ����

		2.2�ǻ��ڴ󲿷���ʽGamma��һ��ƽ��ֵ�� Gamma2.2����ɫ�ռ��ΪsRGB��ɫ�ռ䡣
		ÿһ����ʾ�������Լ���Gamma���ߡ�����gamma2.2�ڴ�����������ϱ��ֶ�����
		�������ԭ����Ϸͨ��������Ҹı���Ϸ��Gamma���ã�����Ӧÿ����ʾ��



	ʹ��Gamma���������ַ�����
		1.ʹ��OpenGL���õ�sRGB֡����
			�����ͷǳ��򵥣�ֻ��Ҫʹ��GL_FRAMEBUFFER_SRGB��
			�Ϳ��Ը���OpenGL���������У�
				�ڱ�������ɫ������֮ǰ��Ӧ���ȴ�sRGB��ɫ�ռ�ʹ��٤��У��
			sRGB��һ�����¶�Ӧgamma2.2����ɫ�ռ䣬��Ҳ�Ǽ����豸��һ����׼��
			��ʹ��GL_FRAMEBUFFER_SRGB֮��
					OpenGL����ÿ��Ƭ����ɫ������֮���Զ��Ķ���������֡������ִ��gamma������ ����Ĭ�ϵ�֡��������
			
			glEnable(GL_FRAMEBUFFER_SRGB); 

		��ʹ��֮����Ⱦ�Ļ��潫���Զ��Ľ���gamma�����ˣ���Ϊ����Ӳ��ִ�еġ�
			����Ӧ�üǵ�������飺
			gammaУ������������ɫ�ռ�ת��Ϊ�����Կռ䣬���������һ������gammaУ���Ǽ�����Ҫ�ġ�
			��������������֮ǰ����gamma������
					�����������������ɫ�Ĳ�����������һ������ȷ����ɫ���в�����
			���磬�����ʹ�ö��֡���壬����ܴ���������֡����֮��
			���ݵ��м�����Ȼ�������Կռ���ɫ��ֻ�Ǹ����͸��������������Ǹ�֡����Ӧ��gammaУ����

		2.��Ƭ����ɫ�����Լ�����gamma����
			��΢����һ�㣬���ǻ��С�
				������ÿ�����������ɫ�����е����Ӧ��gammaУ���������ڷ��͵�֡����ǰ����ɫ�ͱ�У���ˡ�

				void main()
				{
					// do super fancy lighting
					[...]
					// apply gamma correction
					float gamma = 2.2;
					fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
				}
			�����д�����ѡ���
			����������ǣ�
				Ϊ�˱���һ�£��������������ɫ����������gammaУ��������������кܶ�������ɫ����
				���ǿ��ֱܷ����ڲ�ͬ���壬��ô��ͱ�����ÿ����ɫ���ﶼ����gammaУ���ˡ�
			һ�����������
				�����д���֮�����һ�����ڴ������ں��ڴ����ʱ�����gamma����������ֻ��Ҫһ�ν�����oK�˹�

*/	

/*	sRGB textures
		��Ϊ��ʾ��������ʾ��sRGB�ռ���Ӧ����gamma����ɫ��
		���������ڼ�����ϻ���ʲô��������ѡ�����ɫ���Ƕ��ǻ�����ʾ����������ɫ��
		����ζ�ţ����д���ͱ༭��ͼƬ�����ǻ������Կռ䣬������sRGB�ռ䣨����
			eg������Ļ�ϼӱ�һ������ɫ��ɫ���Ǹ������۲쵽�����ȣ�������ζ�żӱ��˺�ɫԪ�ء�
		
		�������ʦ����sRGB�ռ��д���������������ͼ������������������ǵ���ȾӦ����ʹ����Щ�������Ǳ��뿼�������

		������ʹ��Ӧ��gamma����֮ǰ���ⲻ�����⣬��Ϊ������sRGB�ռ��п����������ԣ���������Ҳ�ǹ�����sRGB�ռ��У�
		����û�н�������������Ҳû���⡣
		��
			���������ǻ����������۲쵽����ʾ��������ͼƬ������ʵ���ϣ������Ѿ�ʹ����Gamma���������ǿ������ǽ�����ģ�
			
			����˵�����������ʱ�򣬵õ�����gamma���������ɫ����
			
			���Լ���ʵ�ʵ���ɫֵΪ Crgb, ��
						Crgb^2.2 �õ��Ĳ������ǿ���������Ӧ��ʵ����ɫ�� �����ǵĴ����ǲ�֪���ģ�
					���ǵĴ��빤�������Կռ��У�������еĽ��н����� Crgb^(1/2.2)
					���������ʾ���� Crgb,  ������Crgb^2.2�����Ի�������������
			����취���ǣ�
				���䴦��Ϊ  Crgb^2.2�� ��������ʵ�ʵ���ɫ�ˣ�
		��
					
		�����������Ҫ��ÿ��������н�����������鷳��

		���˵��ǣ�OpenGL�������ṩ����һ��������������ǵ��鷳�������GL_SRGB��GL_SRGB_ALPHA�ڲ������ʽ��

		���������OpenGL�д�����һ����������ָ��Ϊ��������sRGB�����ʽ����֮һ��
		OpenGL���Զ�����ɫУ�������Կռ��У�����������ʹ�õ�������ɫֵ���������Կռ��е��ˡ�

		���ǿ���������һ������ָ��Ϊһ��sRGB����
			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			
			����㻹�������������������alphaԪ�أ��ؾ����뽫������ڲ���ʽָ��ΪGL_SRGB_ALPHA��

			��Ҫע����ǣ�
				ʹ����ɫ����������diffuse��������������sRGB�еġ�
				����specular_maps�������˷��������� normal_maps�������˾���������Խ��Խ���������������Կռ���


*/

/*
--����------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);


unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT , bool gammaCorrection = false);
//Blinn_Phong
void Gamma_Test(GLFWwindow * window);
/*
--����------------------------------
*/
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

glm::vec3 cameraPos(0.0f, 1.0f, 3.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool openEffect = true;
bool gammaEnabled = true;
int lastStatus;
int Exchange_lastStatus;
Camera camera(cameraPos, WorldUp, yaw, pitch);

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
	Gamma_Test(window);
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
			//glDisable(GL_MULTISAMPLE);
		}
		else {
			openEffect = true;
			//glEnable(GL_MULTISAMPLE);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE && Exchange_lastStatus == GLFW_PRESS) {
		//ʹ��B�������л���ɫ����
		if (gammaEnabled) {
			gammaEnabled = false;
		}
		else {
			gammaEnabled = true;
		}
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);
	Exchange_lastStatus = glfwGetKey(window, GLFW_KEY_B);
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

float planeVertices[] = {
	// x-z ƽ��
	// positions          // texture Coords		//Normal
	-8.0f, -1.0f, -8.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -1.0f,  8.0f,	5.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -1.0f, -8.0f,	5.0f, 5.0f,			0.0f,  1.0f, 0.0f,

	-8.0f, -1.0f, -8.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	-8.0f, -1.0f,  8.0f,	0.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -1.0f,  8.0f,	5.0f, 0.0f,			0.0f,  1.0f, 0.0f,

};

void Gamma_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);	//͸��
	
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader plane_Shader("Shader5-2.vertex", "Shader5-2.fragment");
	//Texture
	unsigned int wallTexture = loadTexture("wall.jpg",false);							//�����н���
	unsigned int wallTexture_GammaCorrection = loadTexture("wall.jpg",true);	//���н���

	//VAO , VBO
	//plane
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	//light
	// lighting info
	// -------------
	glm::vec3 lightPositions[] = {
		glm::vec3(-3.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(3.0f, 0.0f, 0.0f)
	};
	glm::vec3 lightColors[] = {
		glm::vec3(0.25),
		glm::vec3(0.50),
		glm::vec3(0.75),
		glm::vec3(1.00)
	};
	glm::vec3 lightPosition(0.0f, 0.0f, 0.0f);

	unsigned int lightVAO, lightVBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Shader��������
	plane_Shader.use();
	plane_Shader.setVec3("light[0].position", lightPositions[0]);
	plane_Shader.setVec3("light[0].ambient",	glm::vec3(0.2f, 0.2f, 0.2f));
	plane_Shader.setVec3("light[0].diffuse",	lightColors[0]);
	plane_Shader.setVec3("light[0].specular",	lightColors[0]);
	plane_Shader.setVec3("light[1].position", lightPositions[1]);
	plane_Shader.setVec3("light[1].ambient",	glm::vec3(0.2f, 0.2f, 0.2f));
	plane_Shader.setVec3("light[1].diffuse",	lightColors[1]);
	plane_Shader.setVec3("light[1].specular",	lightColors[1]);
	plane_Shader.setVec3("light[2].position", lightPositions[2]);
	plane_Shader.setVec3("light[2].ambient",	glm::vec3(0.2f, 0.2f, 0.2f));
	plane_Shader.setVec3("light[2].diffuse",	lightColors[2]);
	plane_Shader.setVec3("light[2].specular",	lightColors[2]);
	plane_Shader.setVec3("light[3].position", lightPositions[3]);
	plane_Shader.setVec3("light[3].ambient",	glm::vec3(0.2f, 0.2f, 0.2f));
	plane_Shader.setVec3("light[3].diffuse",	lightColors[3]);
	plane_Shader.setVec3("light[3].specular",	lightColors[3]);

	plane_Shader.setInt("material.diffuse", 0);
	plane_Shader.setInt("material.specular", 1);
	plane_Shader.setFloat1f("material.shininess", 16.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gammaEnabled ? wallTexture_GammaCorrection : wallTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,  wallTexture);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;
		//Draw
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//plane
		plane_Shader.use();
		model = glm::mat4();
		plane_Shader.setMartix("model", model);
		plane_Shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		plane_Shader.setMartix("view", view);
		plane_Shader.setMartix("projection", projection);
		plane_Shader.setVec3("viewerPos", camera.Position);
		plane_Shader.setBool("openEffect", openEffect);
		plane_Shader.setBool("gamma", gammaEnabled);
		glBindVertexArray(planeVAO);
		if (gammaEnabled) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, wallTexture_GammaCorrection);
			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, wallTexture_GammaCorrection);
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, wallTexture);
			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, wallTexture);
		}
		
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
		std::cout << (gammaEnabled ? "Gamma enabled" : "Gamma disabled") << std::endl;
		//light
		light_Shader.use();
		light_Shader.setMartix("view", view);
		light_Shader.setMartix("projection", projection);
		for (int i = 0; i < 4 ; ++i) {
			model = glm::mat4();
			model = glm::translate(model, lightPositions[i]);
			model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
			light_Shader.setMartix("model", model);
			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
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
