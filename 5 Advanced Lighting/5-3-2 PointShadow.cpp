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
	��������������������������������������������������
	���´��ںܶ�����󣬹��ڵ��Դ��Ӱ�Ĵ����棬��ʵ��������
		1.һ���棬������ɫ���Ĵ�����һ�������δ���������6���ӽ��µ������Σ�

		2.���ڴ���������ϵ�任����Ϊԭ�������ϵ�£�����ȡ�����Ϣʱ���任�����View���������������⣡
			Ϊ��Up������ֵ����������⣡��
			�޸�֮�󼴻���ָ�ʽ���⡣��

	��������������������������������������������������
*/
/*
	Point Shadows
	�������ǵĽ������ڸ��ַ������ɶ�̬��Ӱ������������������ڵ��Դ���������з����ϵ���Ӱ��
	(��һ������ѧϰ�˶������Ӱӳ�䣬��directional shadow mapping , �ڵ�һ�����Դ�����ɵ���Ӱ��)

	�ü�����Ϊ����Ӱ�������ʽ�ġ�ȫ����Ӱӳ�䣺 omnidirectional shadow maps��
	
	���㷨�����ϺͶ�����Ӱӳ����ͬ��
		���ǻ�ӹ���ӽ�����һ�������ͼ�����ݵ�ǰ��fragmentλ�òɼ������ͼ��
	���ñ�������ֵ��ÿ��fragment���бȽϣ������Ƿ�����Ӱ�С�
	
		��Ҫ�Ĳ�ͬ���ڣ������ͼ��ʹ�á�
		������Ҫ��Ⱦ�������ͼ���䳡�������ڵ��Դ�ĸ�������һ����ͨ��2D�����ͼ������ʤ���������
		
		����취��
			��������ͼ���Դ���6����Ļ������ݣ������Խ�����������Ⱦ����������ͼ��
	ÿ�����ϣ������ǵ������Դ���ܵ����ֵ��������
	���ɵ����������ͼ֮�󱻴��ݸ����Ƭ����ɫ��������ɫ��ʹ��һ����������ȥ�ɼ���������ͼ��
	�̶���ȡƬ�ε���ȣ�����ӽǣ���
		����鷳�ĵط��������������ͼ�����ɡ�
*/

/*Generating the depth cubemap	�������������ͼ��
	Ҫ��������һ��������ͼ��������Ҫ��Ⱦ�������Σ�ÿ��һ���档

	����һ��
		ʹ��������ͬ��view������Ⱦ����6�Ρ�ÿ�θ���������ͼ��һ����ͬ���浽framebuffer��

			for(unsigned int i = 0; i < 6; i++)
			{
				GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;		//��ͬ����
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, depthCubemap, 0);	����󶨵�framebuffer
				BindViewMatrix(lightViewMatrices[i]);					//�󶨲�ͬ��view����
				RenderScene();											//��Ⱦ����
			}

		�����㷨�Ŀ����Ƚϰ��󣺶���һ�������������ͼ��Ҫ���ж����Ⱦ���á�
	��������
		������ɫ����������ʹ��һ����Ⱦ���������������������ͼ��
		1.���ȴ���һ����������ͼ
			GLuint depthCubemap;
			glGenTextures(1, &depthCubemap);
		2.������������ͼ��ÿ���棬��������Ϊ2D���ֵ����ͼ��
			const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);	��������ͼ��
			for (GLuint i = 0; i < 6; ++i)
				 glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
						SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		3.Ϊ������ͼ�����������
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		4.ʹ�� glFramebufferTexture ֱ�Ӹ���cubemap��Ϊ֡����������ȸ����
		����������£�������Ҫ����cubemap�����ÿ���浽�滺�����Ȼ����Ⱦ6�Σ�ÿ���л�֡��������
		��Ȼ���Ŀ�굽һ����ͬ��cubemap�档�������Ǵ���ʹ�ü�����ɫ�����ʿ���һ���Դ����꣩
			���룺
				glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				�ٴ���ʾ������ֻ��Ҫ�����Ϣ���ʽ���ɫ�Ļ���������д���ر���
		5.ʹ��ȫ����Ӱӳ�䣺
			��1�����������ͼ��
				glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
				glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
					glClear(GL_DEPTH_BUFFER_BIT);
					ConfigureShaderAndMatrices();					//֡�������ɫ���;�������
					RenderScene();									//�����Ϣ����
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

			��2�����������ͼΪ����������Ӱ��
				glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
				ConfigureShaderAndMatrices();						//Ĭ�ϻ������ɫ���;�������
				glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);	//������Ⱦ
				RenderScene();
			����Ĵ���˼·������ԭ�ȵ�һ�£��仯����ϸ�ڲ���
		6.Light space transform
			������ҪһЩ�����������������м�����任��6����ķ�������Ӧ�Ĺ�ռ䡣
			���Ͻڵ����ƣ����ǻᴴ����Ŀռ�任���󣬵������ÿ����һ������

			ÿһ���任���󶼰���һ��projection��һ��view����
			
			Projection��
				ʹ��perspective projection ����
			��ΪͶӰ������ÿ�������ϲ�����ı䣬���ǿ�����6���任�������ظ�ʹ�á�
				float aspect = (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT;
				float near = 1.0f;
				float far = 25.0f;
				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
				ע���������Ұ��Χ����Ϊ90�ȣ���ȷ������Ұ���㹻���Ժ��ʵ�������������ͼ��һ����
			�������е��涼�ܹ�����������롣

			View��
				������Ҫ������ͬ��View����
				������Ҫʹ��glm::loogAt�������������۲췽��ÿһ��ָ��һ�������ķ���
					right, left, top, bottom, near and far.
					std::vector<glm::mat4> shadowTransforms;
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0));
			
			ͨ����projection��ϼ����Եõ�������ͬ�Ĺ�Դ�ռ�任����
			������Խ����Ǵ�����ɫ��ʹ�á�

		7.�����ɫ��
			Ҫ��Ⱦ���ֵ��һ�����������ͼ�����ǽ���Ҫ�ܹ�������ɫ����������ɫ����Ƭ����ɫ�����ͼ�����ɫ����
			������ɫ�����򵥵�ת�����㵽�������꣬Ȼ��ֱ�Ӵ��ݸ�������ɫ����
				
			������ɫ��������ת�����е��������궥�㵽6����ͬ�Ĺ�ռ䡣
				���룺3�������ζ��㣬һ��uniform���飺��Ŀռ�任����
					Shader5-32.vertex_depth
				������ɫ����һ�����ñ�����gl_Layer,��ָ�������Ļ���ͼ�ε�����������ͼ���ĸ��档��������ʱ��when left alone��
			������ɫ����������һ������Ļ���ͼ�η��͵��ܵ�����һ�׶Σ����ǵ����Ǹ������ֵ��ʱ�������ܹ����ƽ�ÿ������ͼ��
			��Ⱦ���ĸ�������ͼ�С���Ȼ��ֻ�е���������һ�����ӵ������֡�������������ͼ�������Ч.
				���������
					Shader5-32.geometry_depth
				
			Ƭ����ɫ����
				��������ǽ������Լ�����ȣ������Ⱦ���ÿ��fragmentλ�ú͹�Դλ��֮������Ծ��롣
				�����Լ������ֵʹ��֮�����Ӱ�������ֱ�ۡ���
				���룺FragPos(���Լ�����ɫ��)�����λ�������Լ�׶���Զƽ��ֵ��
				�������ǻ�ȡƬ�κ͹�Դ�ľ��룬����׶���Զƽ��ֵ������ӳ�䵽[0,1]
				��������ΪƬ�ε����ֵ��

			ʹ����Щ��ɫ����Ⱦ��������������ͼ���ӵ�֡������󼤻��Ժ����õ�һ����ȫ���������������ͼ
		8.Omnidirectional shadow maps
			�����ж���׼�������󣬾Ϳ�����Ⱦʵ�ʵ�ȫ����Ӱ�ˡ�
				������̺Ͷ�����Ӱӳ��̳����ƣ�����������ǰ󶨵������ͼ��һ����������ͼ��
			������2D�������ҽ����ͶӰ��Զƽ�淢�͸�����ɫ����
			�������£�
				glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				shader.Use();
				// ... send uniforms to shader (including light's far_plane value)
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
				// ... bind other textures
				RenderScene();
			�����RenderScene������һ�����cube������ȾһЩcube������ɢ���ڴ��������������Դ�ڳ������롣
			
			������ɫ����Ƭ����ɫ����ԭ������Ӱӳ����ɫ���󲿷ֶ�һ����
				��֮ͬ�����ڹ�ռ���Ƭ����ɫ��������Ҫһ��fragment�ڹ�ռ��λ�ã��������ǿ���ʹ��һ�����������������ֵ��

		��Ϊ���������ɫ��������Ҫ������λ�������任����ռ䣬�������ǿ����Ƴ�FragPosLightSpace������
			���Shader5-3-2.vertex
		��Ƭ����ɫ���е�Blinn-Phong������֮ǰ����һ�£��仯������Ӱ�ļ�����룺
			���Shader5-3-2.fragment
			����Ĵ����У�
				��Ӱ����ʹ�õĵ���Cube��ͼ��Cube��ͼ�Ĳɼ���Ҫ���������������������Լ򵥵�ͨ����Դλ�ú�Ƭ��λ�����õ�
			����ע�⣬shadow����Ӱ����diffuse��specular�����
*/

/*PCF
	��Ϊȫ����Ӱ��ͼ���ںʹ�ͳ��Ӱ��ͼͬ����ԭ�����ͬ�����зֱ��ʲ����ľ�ݱ������
	Percentage-closer filtering ����PCF��������ͨ����fragmentλ����Χ���˶�����������Խ��ƽ�������̶�ƽ����Щ��ݱߣ�

	��������ú�ǰ��̳�ͬ�����Ǹ��򵥵�PCF�������������������ά�ȣ����������ģ�
		��Shader5-3-2.fragment

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
void Depth_Map_Point(GLFWwindow * window);
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
	Depth_Map_Point(window);

	glfwTerminate();

}

/*
��������ƶ���xֵ��С�������ƶ���xֵ����
��������ƶ���yֵ��С�������ƶ���yֵ����
*/
bool firstMouse = true;
static float lastX = (float)SCR_WIDTH / 2.0, lastY = (float)SCR_HEIGHT / 2.0;
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
void Depth_Map_Point(GLFWwindow * window) {
	/*
		����
		1.���ȴ���һ����������ͼ
		2.������������ͼ��ÿ���棬��������Ϊ2D���ֵ����ͼ��
		3.Ϊ������ͼ�����������
		4.ʹ�� glFramebufferTexture ֱ�Ӹ���cubemap��Ϊ֡����������ȸ����
		5.ʹ��ȫ����Ӱӳ�䣺
		��1�����������ͼ��
		��2�����������ͼΪ����������Ӱ��
		6.Light space transform
		7.�����ɫ��
		8.Omnidirectional shadow maps
	*/
	glEnable(GL_DEPTH_TEST);

	Shader shader("Shader5-3-2.vertex", "Shader5-3-2.fragment");
	Shader depthShader("Shader5-3-2.vertex_depth","Shader5-3-2.geometry_depth", "Shader5-3-2.fragment_depth");

	wallTexture = loadTexture("wall.jpg", false);
	containerTexture = loadTexture("container.jpg", false);
	/*
	��������һ��depth map--cubeMap�� ���ӳ����һ������������ڹ�Դ��͸����ͼ��Ⱦ
	������Ҫʹ�õ�frameBuffer
	*/
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	//creating a 2D texture that we'll use as the framebuffer's depth buffer
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//����cube��ÿ����Ϊ2D�����ֵ������ͼ
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	//���ɵ����ӳ�䲻Ӧ��̫���ӣ���Ϊ����ֻ��Ҫ�����Ϣ����������ָ�������ʽΪGL_DEPTH_COMPONENT��

	//����������ͼ�����ӵ�framebuffer֮�ϡ� ע�ⲻ��Ҫcolor buffer��
	//��ΪOpenGL�£�һ��֡�������������û����ɫ�������ǲ������ģ���������ʽָ����������ɫ���ݵ���Ⱦ��
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);//�������Ϣ
	glDrawBuffer(GL_NONE);//�޻��ƻ�����
	glReadBuffer(GL_NONE);//�޶�ȡ������
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	/*canvasShader.use();
	canvasShader.setInt("depthMap", 0);*/

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
	glm::vec3 lightPos(0.0f, 1.0f, 0.0f);
	shader.use();
	shader.setVec3("pointLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.setVec3("pointLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setVec3("pointLight.specular", glm::vec3(0.4f, 0.4f, 0.4f));
	shader.setFloat1f("pointLight.constant", 1.0f);
	shader.setFloat1f("pointLight.linear", 0.09f);
	shader.setFloat1f("pointLight.quadratic", 0.032f);

	shader.setInt("depthMap", 2);
	shader.setFloat1f("far_plane",100.0f);
	//material �� ������ͼ
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setFloat1f("material.shininess", 16.0f);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		lightPos.z = sin(glfwGetTime() * 0.5) * 3.0;
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 lightSpaceMatrix;

		//Draw

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//first step --- �������ӳ�䣬����ӽ�
		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glCullFace(GL_FRONT);

		model = glm::translate(model, lightPos);	//���λ��
		view = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));	//����ӽ�
		float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
		float near_plane = 1.0f;
		float far_plane = 25.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);
		std::vector<glm::mat4> shadowTransforms;
		//����˳�򱣴�
		/*�����up����y��ֻ����-1�� ����1�Ļ���������⣡��
			ԭ�����ģ�
			��������������������������������������������
			������������������������������������������
			��������������������������������������
			������������Ĳ�������������������
			������������������������������
			��������������������������
			����������������������
			������������������
			��������������
			����������
			������
			��
		*/
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0,  -1.0, 0.0)));	//x������
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));	//x�Ḻ��
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));	//y������
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));//y�Ḻ��
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0,  -1.0, 0.0))); //z������
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));	//z�Ḻ��
		depthShader.use();
		depthShader.setVec3("lightPos", lightPos);
		depthShader.setFloat1f("far_plane", far_plane);//!!!! ע������͹��perspective ͬ�������Ǻ��������Ұ������
		for (int i = 0; i < 6; ++i) {
			std::stringstream ss;
			ss << "shadowMatrices[" << i << "]";
			//std::cout << ss.str() << std::endl;
			depthShader.setMartix(ss.str() , shadowTransforms[i]);
		}
		depthShader.setMartix("model" , model);


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
		//projection = shadowProj;
		//view = glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
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
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		shader.setVec3("pointLight.position", lightPos);
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setVec3("viewerPos", camera.Position);
		shader.setFloat1f("far_plane",25.0f);
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
	shader.setMartix("normalMatrix", glm::transpose(glm::inverse(model)));
	glBindVertexArray(planeVAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	glBindVertexArray(cubeVAO);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(1.0f, -0.25f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMartix("model", model);
	shader.setMartix("normalMatrix", glm::transpose(glm::inverse(model)));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	//model = glm::scale(model, glm::vec3(0.5f));
	shader.setMartix("model", model);
	shader.setMartix("normalMatrix", glm::transpose(glm::inverse(model)));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMartix("model", model);
	shader.setMartix("normalMatrix", glm::transpose(glm::inverse(model)));
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
