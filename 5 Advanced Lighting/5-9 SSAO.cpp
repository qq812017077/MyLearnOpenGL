#include <glad\glad.h>
#include <glfw3.h>

//��ѧ��
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <iostream>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Model.h"

/*
	���� ����SSAO�����У�Ϊ�˼����������������ɵ�kernel�������������ת��
	��ת�ǻ���kernel�ģ��̳���˵����Ϊ�������������߿ռ���������z����ת�������ת��������һ��xyƽ�棨���߿ռ��ڣ���������

	Ȼ������ɫ���У�����ת�����Ǻ�λ�ڹ۲�ռ��ڵķ��������ʹ�õģ� ��ֱ�ӱ������͸÷�����������������
	�����߿ռ�͹۲�ռ�ı任����
	�е�㲻��ԭ��
	����˵���ﲻ��Ҫ�𶥵㹹�����߿ռ䣬Ҳ�������߿ռ䲢����ȫ׼ȷ��

	һ���뷨�ǣ�
	������������ͬһֱ���ϵ����������Թ���һ��������������ֻҪ��֤��������z�����ŷ�����Ҳ���㹻�ˣ�
	����������Ҫ�Ǳ�֤��������ת�������Ϸ��������߾͹��ˡ�

*/

/*	SSAO
	������֮ǰ�Ļ������ս̳����Ѿ������˼򵥵Ľ��ܣ�Ambient light ���������ա�
	�������������Ǽ��볡����������е�һ���̶����ճ�������������ģ����ɢ��(Scattering)��
	����ʵ��������ڲ�ͬ�����ɢ�����Ų�ͬ��ǿ�ȡ����Լ�ӱ��յ����ǲ��ֳ���ҲӦ���б仯��ǿ�ȡ�

	һ�ּ�ӹ��յ�ģ�������ambient occlusion - �������ڱΡ�
	����ԭ����ͨ�������塢�׶��ͷǳ�������ǽ��䰵�ķ�������ģ�����ӹ��ա�
	��Щ����ܴ�̶����Ǳ���Χ�ļ������ڱεģ����߻�������룬�����Ӧ�����䰵��

	�������ڱ���һ����������ܴ�����ܿ�������Ϊ���ǲ��ò�������Χ�ļ����塣���ǿ��ԶԿռ���ÿһ��
	�������������ȷ�����ڱ��������������ʵʱ������Ѹ�ٱ�ò��ɼ��㡣

	SSAO��screen-space ambient occlusion ��Ļ�ռ价���ڱ� ��2007�귢����һ�����
		�ü���ʹ������Ļ�ռ�ĳ��������Ϣȥ�ж��ڱ���������ʹ����ʵ�ļ������ݡ�
		��һ��������������Ļ������ڱβ����ٶȿ죬���һ��ܻ�úܺõ�Ч����ʹ������Ϊ����ʵʱ�������ڱεı�׼��

	SSAO��ԭ���Ǻܼ򵥵ģ�
		������һ�������ı���(Screen-filled Quad)�ϵ�ÿ��Ƭ�����ǻ��ڸ�Ƭ����Χ�������Ϣ��������ڱ�ֵ��
	����ڱ�����֮��ᱻ�������ٻ��ߵ���Ƭ�εĻ������շ�����
		�ڱ�������ͨ���ɼ�Ƭ����Χ���ͺ���(Kernel)�Ķ��������������͵�ǰƬ�����ֵ�Աȶ��õ��ġ�
	����Ƭ�����ֵ�����ĸ�������������Ҫ���ڱ����ӡ�
		
		����������������Խ�࣬Ƭ�λ�õĻ�������Ҳ��Խ�١����Ч���������;����������ǲɼ���ֱ����صġ�
		�����������̫�ͣ���Ⱦ�ľ��Ȼἱ����٣����ǻ�õ�һ�ֽ�������(Banding)��Ч�������������̫�������Ӱ������
		
		���ǿ���ͨ����������Ե���������(Sample Kernel)�Ĳ����дӶ�������������Ŀ��ͨ�������ת�������ģ�
	�����ܹ�ʹ�ø��ٵ����������õ�һ���������Ľ��������Ҫ����һЩ���ۣ�does come at a price����Ϊ����ᵼ������
	һ�����Ե�����ģʽ��������Ҫͨ��ģ��������������
		
	��Ϊʹ�õĲ����ں���һ�����壬������ƽ̹��ǽ�ڿ��������ֻ�ɫ��Ϊһ����ڿƲ������ջ������Χ�ļ������С�
	
	����������ԭ�����Ǵ��㲻������Ĳ����ںˣ����ǲ������ű��淨������һ����������ں�

		ͨ���ڷ��������(Normal-oriented Hemisphere)��Χ���������ǽ����ῼ�ǵ�Ƭ�εײ��ļ�����.�������˻���
	���ڱλ����ɵĸо����Ӷ���������ʵ�Ľ����

*/

/*	Sample buffers
	SSAO��Ҫ������Ϣ����Ϊ������ҪһЩ��ʽ���ж�һ��Ƭ�ε��ڱ����ӡ�����ÿһ��Ƭ�Σ����Ƕ���Ҫ������Ϣ��
		λ��������
		��������
		������ɫ
		һ�������ں�
		һ���������ת��������������ת�����ں�

	ͨ��ʹ����Ƭ�εĹ۲�ռ�λ�ã����ǿ��Խ�һ�����������ں�����Ƭ�εĹ۲�ռ��µı��淨������Ȼ��ʹ������ں�
	�Բ�ͬ��ƫ����ȥ�ɼ�λ�û�������������ÿһ����Ƭ���ں˲ɼ������ǻ����������λ�û������е���Ƚ��бȽϣ�
	���ж��ڱ������ڱ����ӵĽ���������������յĻ�������֡�����ͨ������һ����Ƭ�ε���ת���������ǿ��������ļ���
	����Ҫ�Ĳ��������������ῴ��

	��ΪSSAO��һ����Ļ�ռ似����������һ������ƽ���2D�ķ�����Ϊÿһ��Ƭ�μ�����Ч�������������ζ������û�г����ļ�����Ϣ��
	����Ҫ��������Ƭ����Ⱦ�������ݵ���Ļ�ռ������У��������ᷢ���������SSAO��ɫ�����������ǿ���ʹ��ÿһ��Ƭ�εļ�������
	
	����ӳ���Ⱦ�����ơ���Ҳ����˵SSAO���ӳ���Ⱦ�������ؼ��ݣ���Ϊ�����Ѿ���λ�úͷ���������G�������ˡ�
	�����ʹ��һ���򻯵��ӳ���Ⱦ��

	��������Ӧ���Ѿ���G�����������ÿƬ��λ�ú���ͨ���ݣ���˼��ν׶ε�Ƭ����ɫ���ǳ��򵥣�
			Shader5-9.fragment_SSAO_gbuffer
	��ΪSSAO��һ����Ļ�ռ似���������ڱεļ����ǻ��ڿɼ���ͼ������ڹ۲���ͼ��ʵ���㷨��������ġ�
	��ˣ����ν׶εĶ�����ɫ�����ṩ��FragPos��ת�����۲�ռ��С��������ļ���Ҳ�����ڹ۲�ռ��н��У�����
	Ӧȷ��G-buffer��λ�úͷ��߶����ڹ۲�ռ��У�ͨ������View ����
		
		ͨ��һЩС������ͨ�����ֵ�ع�ʵ��λ��ֵ�ǿ��ܵģ�Matt Pettineo�����Ĳ������ᵽ����һ���ɡ�
	��һ������Ҫ����ɫ�������һЩ���㣬����ʡ��������G�����д洢λ�����ݣ��Ӷ�ʡ�˺ܶ��ڴ档Ϊ��
	ʾ���ļ򵥣����ǽ�����ʹ����Щ�Ż����ɣ����������̽����

	gPosition��ɫ�����������������£�
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
	���������һ��λ���������ǿ������ڻ�ȡÿ���ں˲ɼ������ֵ��ע�����ǽ�λ����Ϣ����Ϊ�������ݸ�ʽ��
	���ַ�ʽ�£�λ��ֵ���ᱻ������[0.0 , 1.0], ͬʱҲע������Ļ��Ʒ�ʽΪGL_CLAMP_TO_EDGE���Ᵽ֤�����ǲ��������
	��������Ļ�ռ�������Ĭ�������������λ��/���ֵ��
		
	������������Ҫʵ�ʵİ���ɼ��ںˣ��Լ�һЩ�����Ⱦ�ķ���
*/

/*Normal-oriented hemisphere �������
	������Ҫ���ű��淨�߷������ɴ�������������������������̵̳Ŀ�ʼ���ܵ�������������Ҫ�����γɰ����ε�������
	���ڶ�ÿ�����淨�߷������ɲ������ķǳ����ѣ�Ҳ����ʵ�ʣ����ǽ������߿ռ�(Tangent Space)�����ɲ������ģ�
	��������ָ����z����

	����������һ����λ�������ǿ��Ի��һ��ӵ�����64����ֵ�Ĳ������ģ�
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
		std::default_random_engine generator;
		std::vector<glm::vec3> ssaoKernel;
		for (unsigned int i = 0; i < 64; ++i)	//64������
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,	//-1.0 - 1.0
				randomFloats(generator) * 2.0 - 1.0,	//-1.0 - 1.0
				randomFloats(generator)					//0.0 - 1.0
			);
			sample  = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = (float)i / 64.0; 
			ssaoKernel.push_back(sample);  
		}
		���������߿ռ�����-1.0��1.0Ϊ��Χ�任x��y���򣬲���0.0��1.0Ϊ��Χ�任������z�������������-1.0��1.0֮��任��
	�����ǻ�õ�һ����״�ɼ��ںˣ�����Ϊ�ɼ��ں˳�����淨���������õ�����ʸ�������ڰ����
		
		��ǰ�����еĲ���������ֲ��ڲ����ںˣ��������Ǹ�Ը��Կ���ʵ��Ƭ�ε��ڱθ������Ȩ�أ�Ҳ���ǽ�������������ԭ��ֲ���
	���ǿ�����һ�����ٲ�ֵ����ʵ������
			...[���Ϻ���]
		   scale = lerp(0.1f, 1.0f, scale * scale);
		   sample *= scale;
		   ssaoKernel.push_back(sample);  
		}

		lerp�������£�
			float lerp(float a, float b, float f)
			{
				return a + f * (b - a);
			}  
	��͸�������һ���󲿷���������ԭ��ĺ��ķֲ���
		
		ÿһ���ں˲ɼ�����������ȥƫ�ƹ۲�ռ��Ƭ��λ���Բɼ���Χ�ļ��Ρ�Ϊ�˻����ʵ��Ч����������Ҫ�ڹ۲�ռ��л�ô���
	�Ĳ���������������ܵ�Ӱ��̫��
		Ȼ�������������ÿ��Ƭ�εĻ���������һЩ������������ǿ����������ٴ����Ĳ�������

*/


/*Random kernel rotations ����ں���ת
	ͨ������һЩ����Ե����������ϣ����ǿ��Դ����ٻ�ò��������������������
	���ǿ��ԶԳ�����ÿһ��Ƭ�δ���һ�������ת�����������ܿ콫�ڴ�ľ���
	���ԣ����õķ����Ǵ���һ��С�������ת��������ƽ������Ļ�ϡ�

	���Ǵ���һ��4*4�ĳ������߿ռ���淨�����������ת�����飺
		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0, 
				randomFloats(generator) * 2.0 - 1.0, 
				0.0f); 
			ssaoNoise.push_back(noise);
		} 
	��Ϊ�����ں˳��������߿ռ����z���������趨z����Ϊ0.0���Ӷ�Χ��z����ת��

	Ȼ�󴴽�4*4�������������������ת������ȷ�����价�Ʒ�ʽ����ΪGL_REPEAT���Ӷ���֤�����ʵ�ƽ������Ļ�ϡ�
		GLuint noiseTexture;
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	���������������е�����������ݣ�������������Ҫʵ��SSAO��

*/

/*	SSAO��ɫ��	The SSAO shader
	SSAO��ɫ����2D�������ı��������У�������ÿһ�����ɵ�Ƭ�μ����ڱ�ֵ(Ϊ�������յĹ�����ɫ����ʹ��)��
	����������Ҫ�洢SSAO�׶εĽ�������ǻ���Ҫ�ڴ�������һ��֡�������

		unsigned int ssaoFBO;
		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		unsigned int ssaoColorBuffer;
		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	��Ϊ�����ڱεĽ����һ�������ĻҶ�ֵ�����ǽ���Ҫһ������ĺ�ɫ������������������ɫ�����������ø�ʽΪΪGL_RED��

	��ȾSSAO���������̿�����Ӧ����������ӣ�
		// geometry pass: render stuff into G-buffer
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			[...]
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  
  
		// use G-buffer to render SSAO texture
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT);    
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);
			shaderSSAO.use();
			SendKernelSamplesToShader();
			shaderSSAO.setMat4("projection", projection);
			RenderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
		// lighting pass: render scene lighting
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLightingPass.use();
		[...]
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		[...]
		RenderQuad();
	
	shaderSSAO��ɫ��������Ӧ��G-buffer�������������Լ���������ں˲�����Ϊ���룬
		�������� Shader5-9.fragment_SSAO��

		�����˼�뼴��
			�����ȡ��Ӧλ���£����䷨����Ϊ����İ���ռ��ڵĵ㣨�˰������𲽻�ȡ64�������㣩��
		ͨ���Ա���������ĵ����Ⱥ͸�λ�õ���ȹ�ϵ��������Ϊ�����������ȴ��ڼ���Ӧ��ʵ����ȣ��������ڱ����ӵ�ֵ
*/


/*
--����------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void renderPlane(Shader shader);
void renderCube(Shader shader, std::vector<glm::vec3> cubePositions, std::vector<glm::vec3> cubeRotateAxis = std::vector<glm::vec3>());
void renderLight(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors = std::vector<glm::vec3>());
void RenderQuad(Shader shader);

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT, bool gammaCorrection = false);

void SSAO_Test(GLFWwindow * window);
void SSAO_Gbuff_Test(GLFWwindow * window);

/*
--����------------------------------
*/
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool hdr = true;
bool hdrKeyPressed = false;
float exposure = 1.0f;

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
	SSAO_Test(window);
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
		hdrKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
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
	//std::cout << "extract : " << extract << std::endl;
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

void SSAO_Gbuff_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader geometryPass_Shader("Shader5-9.vertex_gbuffer", "Shader5-9.fragment_gbuffer");	//���������ݴ�����G-buffer
	Shader shaderSSAO("Shader5-9.vertex_SSAO", "Shader5-9.fragment_SSAO");
	Shader lightPass_shader("Shader5-9.vertex_light", "Shader5-9.fragment_light");

	//���׻�����
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit/nanosuit.obj";
	Model nanosuitModel(model_path);

	//G-buffer������
	GLuint gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	GLuint gPosition, gNormal, gAlbedo; //����������

	//0 - Position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL��ʾ�յ�������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);	//Position - 0

																								//1 - Normal buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL��ʾ�յ�������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);	//Normal - 1

																								// color + specular color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	//3 - Create and attach Depth buffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// lighting info
	// -------------
	glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
	glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

	lightPass_shader.use();
	lightPass_shader.setInt("gPosition", 0);
	lightPass_shader.setInt("gNormal", 1);
	lightPass_shader.setInt("gAlbedo", 2);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glEnable(GL_DEPTH_TEST);
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		std::vector<glm::vec3> cubePosition;
		cubePosition.push_back(glm::vec3(0.0f, 7.0f, 0.0f));
		//Draw
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		// 1.geometry pass: render stuff into G-buffer
		//glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// room cube  -- һ�����������
		geometryPass_Shader.setMartix("projection", projection);
		geometryPass_Shader.setMartix("view", view);
		geometryPass_Shader.setInt("invertedNormals", 1); // invert normals as we're inside the cube
		renderCube(geometryPass_Shader, cubePosition);
		geometryPass_Shader.setInt("invertedNormals", 0);

		// nanosuit model on the floor	--���ڵذ��ϵ�ģ��
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 5.0));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(0.5f));
		geometryPass_Shader.setMartix("model", model);
		nanosuitModel.Draw(geometryPass_Shader);

		// 2. lighting pass:
		// -----------------------------------------------------------------------------------------------------
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//lightPass_shader.use();
		//// send light relevant uniforms
		//glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
		//lightPass_shader.setVec3("light.Position", lightPosView);
		//lightPass_shader.setVec3("light.Color", lightColor);
		//// Update attenuation parameters
		//const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
		//const float linear = 0.09;
		//const float quadratic = 0.032;
		//lightPass_shader.setFloat1f("light.Linear", linear);
		//lightPass_shader.setFloat1f("light.Quadratic", quadratic);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, gPosition);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, gNormal);
		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, gAlbedo);
		//RenderQuad(lightPass_shader);

		//
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
//�ӳ���Ⱦ
void SSAO_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader geometryPass_Shader("Shader5-9.vertex_gbuffer", "Shader5-9.fragment_gbuffer");	//���������ݴ�����G-buffer
	Shader shaderSSAO("Shader5-9.vertex_SSAO", "Shader5-9.fragment_SSAO");
	Shader lightPass_shader("Shader5-9.vertex_light", "Shader5-9.fragment_light");

	//���׻�����
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit/nanosuit.obj";
	Model nanosuitModel(model_path);

	//G-buffer������
	GLuint gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	GLuint gPosition, gNormal, gAlbedo; //����������

	//0 - Position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL��ʾ�յ�������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);	//Position - 0

	//1 - Normal buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL��ʾ�յ�������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);	//Normal - 1

	// color + specular color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	//3 - Create and attach Depth buffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//SSAO ֡���� �Լ� SSAO ģ��֡����
	unsigned int ssaoFBO, ssaoBlurFBO;
	glGenFramebuffers(1, &ssaoFBO);					glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);	
	unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	// and blur stage
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//SSAO�����ں�
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	for (unsigned int i = 0; i < 64; ++i)
	{
		//������64��sample����������ֲ��ڳ���z������İ������ڣ�
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,	//-1.0 �� 1.0
			randomFloats(generator) * 2.0 - 1.0,	//-1.0 �� 1.0
			randomFloats(generator)					// 0.0 �� 1.0
		);
		sample = glm::normalize(sample);	//��λ����
		sample *= randomFloats(generator);	//
		float scale = (float)i / 64.0;		

		//��������һ���󲿷���������ԭ��ĺ��ķֲ���
		/*
			lerp:���Բ�ֵ
			float lerp(float a , float b , scale){
				return a + scale * (b - a);
			}
		*/
		float lerp = 0.1f + scale * scale * (1.0f - 0.1f);
		scale = lerp;
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	std::vector<glm::vec3> ssaoNoise;	//����ת������kernal
	for (GLuint i = 0; i < 16; i++)
	{
		glm::vec3 noise(	
			randomFloats(generator) * 2.0 - 1.0,		//-1.0 - 1.0
			randomFloats(generator) * 2.0 - 1.0,		//-1.0 - 1.0
			0.0f);	//��������							// 0.0		z��Ϊ0���� ��Ϊ xyƽ���������
		ssaoNoise.push_back(noise);
	}
	unsigned int noiseTexture; glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);	//������������������������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// lighting info
	// -------------
	glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
	glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

	lightPass_shader.use();
	lightPass_shader.setInt("gPosition", 0);
	lightPass_shader.setInt("gNormal", 1);
	lightPass_shader.setInt("gAlbedo", 2);
	lightPass_shader.setInt("ssao", 3);
	shaderSSAO.use();
	shaderSSAO.setInt("gPosition", 0);
	shaderSSAO.setInt("gNormal", 1);
	shaderSSAO.setInt("texNoise", 2);
	//shaderSSAOBlur.use();
	//shaderSSAOBlur.setInt("ssaoInput", 0);


	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glEnable(GL_DEPTH_TEST);
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		std::vector<glm::vec3> cubePosition;
		cubePosition.push_back(glm::vec3(0.0f, 3.0f, 0.0f));
		//Draw
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 50.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		// 1.geometry pass: render stuff into G-buffer, ���� λ�á��������ͷ�����
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		geometryPass_Shader.use();
		geometryPass_Shader.setMartix("projection", projection);
		geometryPass_Shader.setMartix("view", view);

		// room cube  -- һ�����������
		geometryPass_Shader.setInt("invertedNormals", 1); // invert normals as we're inside the cube
		renderCube(geometryPass_Shader, cubePosition);
		geometryPass_Shader.setInt("invertedNormals", 0);
		// nanosuit model on the floor	--���ڵذ��ϵ�ģ��
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(0.3f));
		geometryPass_Shader.setMartix("model", model);
		nanosuitModel.Draw(geometryPass_Shader);

		// 2. generate SSAO texture
		//glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shaderSSAO.use();
			shaderSSAO.setMartix("projection", projection);
			// Send kernel + rotation 
			for (unsigned int i = 0; i < 64; ++i)
				shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);	//noiseTexture
			RenderQuad(shaderSSAO);
		
		// 3. blur SSAO texture to remove noise
		// ------------------------------------
		/*glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAOBlur.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		renderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

		// 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
		// -----------------------------------------------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightPass_shader.use();
		// send light relevant uniforms
		glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
		lightPass_shader.setVec3("light.Position", lightPosView);
		lightPass_shader.setVec3("light.Color", lightColor);
		// Update attenuation parameters
		const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
		const float linear = 0.09;
		const float quadratic = 0.032;
		lightPass_shader.setFloat1f("light.Linear", linear);
		lightPass_shader.setFloat1f("light.Quadratic", quadratic);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);
		glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		RenderQuad(lightPass_shader);
		
		//
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
	model = glm::scale(model, glm::vec3(3.0f, 1.0f, 3.0f));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

}
void renderCube(Shader shader, std::vector<glm::vec3> cubePositions, std::vector<glm::vec3> cubeRotateAxis ) {
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
		if (!cubeRotateAxis.empty())
			model = glm::rotate(model, glm::radians(i * 15.0f), cubeRotateAxis[i]);
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);

}

void renderLight(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors) {
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
		if (!lightColors.empty())
			shader.setVec3("cubeColor", lightColors[i]);
		else
			shader.setVec3("cubeColor", glm::vec3(1.0f, 1.0f, 1.0f));
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
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

void Model_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	Shader geometryPass_Shader("Shader5-8.vertex_gbuffer", "Shader5-9.fragment_SSAO_gbuffer");
	//���׻�����
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit/nanosuit.obj";
	Model nanosuitModel(model_path);


	std::vector<glm::vec3> objectPositions;
	objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));

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

		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		geometryPass_Shader.use();
		geometryPass_Shader.setMartix("projection", projection);
		geometryPass_Shader.setMartix("view", view);
		geometryPass_Shader.setBool("inverse_normals", false);
		for (GLuint i = 0; i < objectPositions.size(); i++) {
			model = glm::mat4();
			model = glm::translate(model, objectPositions[i]);
			model = glm::scale(model, glm::vec3(0.25f));
			geometryPass_Shader.setMartix("model", model);
			nanosuitModel.Draw(geometryPass_Shader);
		}
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}