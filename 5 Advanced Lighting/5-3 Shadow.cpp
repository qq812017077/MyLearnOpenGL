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

/*Shadow Mapping
	Shadow����Ϊ�ڵ�����û�й��ߵĽ��������Դ�Ĺ�����Ϊ�ܵ�һЩ������ڵ���
	����������ĳ����ʱ�򣬸ô��ʹ�����Shadow�С�

	ShadowΪһ�����ճ��������˴�������ʵ�������ù۲��߹۲����֮��Ŀռ��ϵ��ĸ����ס�
	�����˳����Ͷ����Ը��õ���ȸС�

	Ŀǰ��û�зǳ�������shadow�㷨���кܶ�õ�������㷨��������һЩ�Լ���ë����Ҫ�����ǿ��ǡ�

	һ���ڴ����videogame����ʹ�õļ����ǣ�����һ�������Ч������������ʵ�֣���
		Shadow mapping
	
	Shadow mapping˼�룺
		�ӹ�Դ����ͼλ�ý�����Ⱦ���棬���п��õ���һ���Ǳ������ģ������п���������Ӧ���������ġ�
	����ӹ�Դ�����Ÿ���������һ�����߿��Ի�ã�ÿ�����߷�����һ����������ĵ㣬��ô��������Զ�Ĳ��ֵĵ�Ͳ��ᱻ�����ˡ�
	������һ���ǳ�û��Ч�ʵķ���������ʵʱ��Ⱦ�����ر���Ч��
	
	ʵ��������Ҫ�������������Ƶģ����ǲ���ͨ�����ߣ�����ͨ����Ȼ�������

		��Ȼ������е�ֵ������������ӽ��£�һ��Ƭ�ε������Ϣ����ô
	�������Ҫ�ӹ���ӽ���Ⱦ�����������յ������Ϣ�������������أ� �������ǾͿ��Բɼ���
	�ӹ���ӽ�����������ֵ��Ϣ��	���գ����ֵ�ͻ���ʾ�ӹ�Դ��͸��ͼ�¼����ĵ�һ��ƬԪ��
	���ǽ������������Ϣ������һ������ depth map ���� shadow map�������С�

	ͶӰ�Ļ���˼·�������£�
		���ȣ�������Ҫ�ڹ�Դ��͸����ͼ�£����ڹ��λ�ú͹�ķ��򹹽�Model��View��Perspective����(ƽ�й�Ļ�Ӧ������������)��
	Perspective * View ��Ϊ����������ת������Դ�ӽ�������ı任������������ΪT��
		����Ҫ�����ƬԴP�������ڹ�Դ͸��ͼ�µ������ϢΪ0.9����
		1.���Ƚ���Դ͸��/����ͼ�µ������Ϣ������Shadow_Map�£�
				�������ͼ�������������������Ϣ��P����������ϢֵΪ0.4����
		2.��ȡƫԶP��T�µ�λ�ã��Լ�����ͼ�µ����ֵ��
		3.�Ƚ�P�����ֵ��0.9���Ͷ�Ӧ�����µ������Ϣֵ��0.4��
		4.����P���ڸ�Զ����˵��Ӧ����Ӱ�¡�
*/

/*��Ӱʧ��(Shadow Acne)
	�ڰ�����������д���������е��������ϲ���ź���
	��Ӱӳ�仹���е㲻��ʵ��
		�ذ��ı�����Ⱦ���ܴ�һ�齻����ߡ�
	��Ӱʧ��(Shadow Acne)��
		��Ϊ��Ӱ��ͼ�����ڷֱ��ʣ����ƬԪ���ܴ������ͼ��ͬһ��ֵ��ȥ������
		һ�������ǣ�
			һ��������ؿ��ܱ�����Χ�Ķ��ƬԪ��ʹ�ã�Ҳ����˵�ڽ��бȽ�ʱ��
		��Ȼ������ÿ��ƬԪӦ�����Ų�ͬ�����ֵ������ʵȴ����Ϊ���ֵ��ͬ��
		����������ص�ֵΪ10��������Χ����9.8 �� 10.2 ����ʵ����ȵ�Ƭ�Σ�
		��ô9.8�ıȽϺ�ͱ���������10.2���򲻻ᡣ
		����������ͻ��γɽ�����ߣ�

		����İ취���ǣ�
			�Ա�������/�����ͼ��ֵӦ��һ��ƫ�ƣ���������С����ͼ���ĳһ��ֵ��ʵ����ȣ�
			float bias = 0.005;
			float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
			���ϣ�����ʵ�ʵõ�������ƶ�һ�ξ��룬�����ܵ�����ʧ���Ӱ�졣

	!!!
		ѡ����ȷ��ƫ����ֵ���ڲ�ͬ�ĳ�������ҪһЩ����������΢��У�����������£�ʵ���Ͼ�������ƫ����ֱ������ʧ�涼���Ƴ������⡣
		��������˹��󣬽�û��Ӱ�ӣ�ʵ�����ƫ��֮��ԶС�������ȡ���
*/

/*����:Peter panning
	ʹ����Ӱƫ�Ƶ�һ��ȱ���ǣ�
		��������ʵ����Ƚ�����ƫ�ơ�
			ƫ���п����㹻�������ڿ��Կ�����Ӱ���ʵ������λ�õ�ƫ��
		�����Ӱʧ�������������Ϊ���忴�������������ڱ��棺����˵������Ӱ����΢�ķ��롣
	���ǿ���ʹ��һ��С��Ϸ��������������������:������Ⱦ��������ʱ��ʹ�������޳���front face culling����
		����֮ǰʹ�ù������޳����������ġ�
		
		Ϊ���޸�peter���ƣ�����Ҫ���������޳����ȱ��뿪��GL_CULL_FACE��

		glCullFace(GL_FRONT);
		RenderSceneToDepthMap();
		glCullFace(GL_BACK); // ��Ҫ�������ԭ�ȵ�culling face
*/

/*Over sampling
	��һ���Ӿ��������ڹ�Ŀɼ�׶�����ĳЩ����һ�ɱ���Ϊ�Ǵ�����Ӱ�У���������Ĵ�����Ӱ֮�С�
	(��ǰ�ĳ����У��ذ��һ����Ե�Ǽ�����Զ��������֮�С�)
	������������ԭ������Ϊ��
		�ڹ�׵�����ͶӰ�������1.0f����˲ɼ����������������[0,1]�ķ�Χ��
		��������Ļ��Ʒ�ʽ�����ǽ��õ�����ȷ����Ƚ���������ǻ�����ʵ�����Թ�Դ�����ֵ
		Ŀǰ����ʹ�õ���GL_REPEAT��

		�������������г��������ͼ���������ȷ�Χ��1.0���������������꽫��Զ������Ӱ֮�У�
		���ǿ��Դ���һ���߿���ɫ��Ȼ��������ͼ��������ѡ������ΪGL_CLAMP_TO_BORDER��

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		�����Ļ����������Ǻ�ʱ�ɼ������ͼ���귶Χ�������������������1.0����ȡ�����ӰֵΪ0.0��

	�����Ļ���shadow��������׶�ķ�ΧzΪ1.0�����������ǽ������������⣬
	��������ʵ����Ⱦʱ���ò��ֵ����ֵ��
		ʵ���ϣ��������׵��Զƽ��Ĳ��֣����������zֵ����1.0�����ʱ�򣬾ͻᱻ��Ϊ����Ӱ�У����������ҪԶ��


	����������쳣����Ӱ�ͱ�����ˣ�����

	���ڣ�
		ֻ���������ͼ��Χ���ڵı�ͶӰ��fragment���������Ӱ�������κγ�����Χ�Ķ�����û����Ӱ��
	��������Ϸ��ͨ����ֻ������Զ�����ͻ������֮ǰ���Ǹ����Եĺ�ɫ����Ч������ʵ��
*/

/*PCF
	Ŀǰ����Ӱ���кܴ�Ĳ��㣺��ֱ������Բ��㣬���Ŵ���Ӱ��ʱ��������ԵĹ۲쵽��ݡ�
	��Ϊ���������һ���̶��ķֱ��ʣ����ƬԪ��Ӧ��һ���������ء�
	
	��˶��ƬԪ��������ͼ��ͬһ�����ֵ���в������⼸��ƬԪ��õ�����ͬһ����Ӱ
	����ͻ������ݱ�Ե��

	����ͨ����������ֱ��������پ�ݣ����߳����ù�׶�御���ܽӽ�������

	��һ�ֽ��������ʹ��PCF �� percentage-closer filtering
		�����ӽ����ˣ��������ữShadow Map��Ե,
			����һ�ֶ��ֲ�ͬ���˷�������ϣ����ڲ����ữ��Ӱ��
	˼�룺
			����������ж�β�����ÿ�βɼ��������������иı䡣
			����ÿ�������Ĳ�����������Ƿ�����Ӱ�С�
			�����еĽ����ϣ�ȡƽ��ֵ�����Ǿ��ܵõ�һ���õ���Ӱ��

		һ���򵥵İ취��
			�򵥵Ĳɼ��ܱ����ز�ȡƽ����
	�������Զ������������ƫ�ƣ�ȷ��ÿ�������������Բ�ͬ�����ֵ��
	ʹ�ø��������������texelSize��������Ϳ���������Ӱ����ͳ̶ȡ�

	ʵ����PCF���и�������ݣ��Լ��ܶ༼��Ҫ����Ҫ���������������Ӱ��Ч���������ڱ������ݳ��ȿ��ǣ����ǽ������Ժ����ۡ�


*/

/*	Orthographic vs projection  ����VsͶӰ
	��ʹ����������ͶӰ������Ⱦ��������ʱ����һЩ����
		����ͶӰ���󲢲��Ὣ������͸��ͼ���б��Σ���������/���߶���ƽ�еģ�����ڶ���������Ǻܺõ�ͶӰ����
		��͸��ͶӰ��������͸�ӶԶ�����б��Σ��⽫������ͬ�Ľ����

	͸�Ӿ�����ڴ���ʵ��λ�õĹ�Դ���Ÿ�ʵ�ʵ����壬
	���͸��ͶӰͨ�������ڵ��Դ��۹�ƣ�������ͶӰ������ƽ�й⡣

	��һ��ϸ΢(subtle)���͸��ͶӰ���󣬽���Ȼ����Ӿ���������õ�һ������ȫ�׵Ľ�������������������ڻ����ϣ���
	������Ϊ����͸�Ӿ�������ȱ�ת��Ϊ�˷��������ֵ�����ڽ�ƽ�����Ÿ��ߵķֱ��ʡ�
	Ϊ�˿�����ʹ������ͶӰһ�����ʵĹ۲쵽���ֵ��������Ƚ������������ֵת��Ϊ���Եģ���������Ȳ��Խ̳����Ѿ����۹���

*/

/*
--����------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void renderScene(const Shader &shader);
unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT);
//Depth_Map
void Depth_Map(GLFWwindow * window);
/*
--����------------------------------
*/
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool openEffect = true;
bool functoinExchange = true;
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
	Depth_Map(window);

	glfwTerminate();

}

/*
��������ƶ���xֵ��С�������ƶ���xֵ����
��������ƶ���yֵ��С�������ƶ���yֵ����
*/
bool firstMouse = true;
static float lastX = (float)SCR_WIDTH / 2.0 , lastY = (float)SCR_HEIGHT / 2.0;
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
		if (functoinExchange) {
			functoinExchange = false;
		}
		else {
			functoinExchange = true;
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
	-8.0f, -0.5f, -8.0f,	0.0f, 8.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -0.5f,  8.0f,		8.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -0.5f, -8.0f,		8.0f, 8.0f,			0.0f,  1.0f, 0.0f,

	-8.0f, -0.5f, -8.0f,	0.0f, 8.0f,			0.0f,  1.0f, 0.0f,
	-8.0f, -0.5f,  8.0f,	0.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -0.5f,  8.0f,		8.0f, 0.0f,			0.0f,  1.0f, 0.0f,

};

float canvasVertices[] = {
	// positions        
	-1.0f,	-1.0f,	0.0f,	0.0f, 1.0f,
	1.0f,	-1.0f,	0.0f,	1.0f, 0.0f,
	1.0f,	1.0f,	0.0f,	1.0f, 1.0f,
	1.0f,	1.0f,	0.0f,	0.0f, 1.0f,
	-1.0f,  1.0f, 	0.0f,	0.0f, 0.0f,
	-1.0f, -1.0f,  	0.0f,	1.0f, 0.0f
};							

unsigned int planeVAO, planeVBO;
unsigned int cubeVAO, cubeVBO;
unsigned int canvasVAO, canvasVBO;
unsigned int wallTexture;
unsigned int containerTexture;
void Depth_Map(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	
	Shader shader("Shader5-3.vertex", "Shader5-3.fragment");
	Shader depthShader("Shader5-3.vertex_directLight" , "Shader5-3.fragment_directLight");
	Shader canvasShader("Shader5-3.vertex_canvas", "Shader5-3.fragment_canvas");

	wallTexture = loadTexture("wall.jpg", false);
	containerTexture = loadTexture("container.jpg", false);
	/*
	��������һ��depth map�� ���ӳ����һ������������ڹ�Դ��͸����ͼ��Ⱦ
	������Ҫʹ�õ�frameBuffer
	*/
	unsigned int depthMapFBO;
	glGenFramebuffers(1,&depthMapFBO);
	//creating a 2D texture that we'll use as the framebuffer's depth buffer
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMapTexture;
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D , depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//����Ϊ��ȸ���
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//�����Ļ��Ʒ�ʽ�ᵼ���ڹ�Դ����׶��֮����������Shadow�У�
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//����ѡ��GL_CLAMP_TO_BORDERѡ����ǻ���Ҫָ��һ����Ե����ɫ------���������˵��
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	//glBindTexture(GL_TEXTURE_2D , 0);

	//���ɵ����ӳ�䲻Ӧ��̫���ӣ���Ϊ����ֻ��Ҫ�����Ϣ����������ָ�������ʽΪGL_DEPTH_COMPONENT��

	//����������ͼ�����ӵ�framebuffer֮�ϡ� ע�ⲻ��Ҫcolor buffer��
	//��ΪOpenGL�£�һ��֡�������������û����ɫ�������ǲ������ģ���������ʽָ����������ɫ���ݵ���Ⱦ��
	glBindFramebuffer(GL_FRAMEBUFFER , depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);//�������Ϣ
	glDrawBuffer(GL_NONE);//�޻��ƻ�����
	glReadBuffer(GL_NONE);//�޶�ȡ������
	glBindFramebuffer(GL_FRAMEBUFFER , 0);
	canvasShader.use();
	canvasShader.setInt("depthMap",0);

	//VAO , VBO
	//canvas

	glGenVertexArrays(1, &canvasVAO);
	glGenBuffers(1, &canvasVBO);
	glBindVertexArray(canvasVAO);
	glBindBuffer(GL_ARRAY_BUFFER, canvasVAO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(canvasVertices), &canvasVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glBindVertexArray(0);

	//plane
	
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
	//cube
	
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//light
	glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
	shader.use();
	shader.setVec3("directLight.direction", -lightPos);
	shader.setVec3("directLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.setVec3("directLight.diffuse", glm::vec3(0.3f,0.3f,0.3f));
	shader.setVec3("directLight.specular", glm::vec3(0.1f,0.1f,0.1f));

	//material �� ������ͼ
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setInt("shadowMap", 2);
	shader.setFloat1f("material.shininess", 16.0f);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;
		
		glm::mat4 model ;
		glm::mat4 view ;
		glm::mat4 projection ;
		glm::mat4 lightSpaceMatrix;

		//Draw

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//first step --- �������ӳ�䣬����ӽ�
		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glCullFace(GL_FRONT);
		
		model = glm::translate(model , lightPos);	//���λ��
		view = glm::lookAt(lightPos,glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));	//����ӽ�
		float near_plane = 1.0f, far_plane = 7.5f;
		projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);//������ͼ
		
		depthShader.use();
		depthShader.setMartix("view", view);
		depthShader.setMartix("projection", projection);
		lightSpaceMatrix = projection * view;

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);//�����Ȼ�������û����ɫ������

		//RenderScene
		
		renderScene(depthShader);
		glCullFace(GL_BACK); // ��Ҫ�������ԭ�ȵ�culling face

		//second step --- ���г������ƣ�������ӽ�
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//ConfigureShaderAndMatrices()
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		//Canvas ��Ⱦ
		//canvasShader.use();
		//canvasShader.setFloat1f("near_plane", near_plane);
		//canvasShader.setFloat1f("far_plane", far_plane);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		////RenderScene
		//glBindVertexArray(canvasVAO);
		//glDrawArrays(GL_TRIANGLES,0,6);
		
		shader.use();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setMartix("normalMatrix",glm::transpose(glm::inverse(model)));
		shader.setMartix("lightSpaceMatrix", lightSpaceMatrix);
		renderScene(shader);
		
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void renderScene(const Shader &shader)
{

	// floor
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glm::mat4 model;
	shader.setMartix("model", model);
	glBindVertexArray(planeVAO);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	glBindVertexArray(cubeVAO);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	//model = glm::scale(model, glm::vec3(0.5f));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
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
