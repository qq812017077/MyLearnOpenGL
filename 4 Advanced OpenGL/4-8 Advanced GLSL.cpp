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

/*Advanced GLSL
	built-in variables ���ñ�������
	����֮ǰʹ����gl_Position�����Ƕ�����ɫ���Ĳü��ռ����λ��������
	�����Ҫ����Ļ����Ⱦ�κζ��������øñ����Ǳز����ٵ�Ҫ��
*/

/*vertex shader variables
---------------------------------
	gl_PointSize��
		�����һ�����ǿ�ѡ�õ���ȾͼԪ��render primitive����GL_POINTS��
		ÿһ�������Ķ��㶼��һ��ͼԪ�����ҿɱ���ȾΪһ���㡣
		���ǿ�������Ҫ��Ⱦ�ĵ�Ĵ�С��ͨ����OpenGL�� gl_PointSize������
		���ߣ����ǿ�����shader��ɫ������Ӱ�����ֵ��
		
		GLSL������һ����������У�gl_PointSize
		����һ��float�������������õ�Ŀ�Ⱥ͸߶ȣ���λ�����أ���
		ͨ���ڶ�����ɫ�����޸������Ĵ�С�����ǿ��Կ���ÿ������Ĵ�С��

		ע���Ч��Ĭ�ϱ��رգ���Ҫʹ��֮����ܿ��ƣ�
			glEnable(GL_PROGRAM_POINT_SIZE);  
		Ȼ���޸Ķ�����ɫ����
			eg��
				void main(){
					gl_Position = projection * view * model * vec4(aPos, 1.0);    
					gl_PointSize = gl_Position.z;    
				}
		�޸ĵ��СΪzֵ�������ԽԶ��Խ�� emmm , �ܲ��ϳ���
	
	ע��gl_Position��gl_PointSize��������������ǿ���ͨ���޸�������Ӱ������

	gl_VertexID��
		������ɫ��Ҳ��������һ���������������ֻ�ܶ�ȡ��
			gl_VertexID  ------> integer

		���α����������������ڻ��ƵĶ����ID��
		��ʹ��������Ⱦ��ʱ���ֵ�����˻��ƶ����������
		����ʹ���������Ƶ�ʱ�򣬸�ֵ�����˴���Ⱦ���ÿ�ʼ����ǰ�Ѵ���Ķ�������
*/

/*fragment shader variables
----------------------------
		����������� FragColor ;
	���ṩ���������õ����������
		gl_FragCoord�� -------ֻ��
			(��ȡ��ǰƬ�εĴ������꣬�Լ������)
			gl_FragCoord���� x , y , z �������������
			���У�
				x,y�����Ƭ�εĿռ����꣬�Դ��ڵ����½�Ϊԭ�㣬
				������ʹ��glViewport����һ��800*600�Ĵ���ʱ��
					x range �� 0 - 800 �� 
					y range �� 0 - 600 ��
			
		gl_FrontFacing�� bool����ֵ��
			�ñ����������ǣ���ǰƬ���������������һ���֣����Ǳ������һ���֡�
			��OpenGL�ܹ��ֱ�����������滹�Ǳ����棩
			ע����������޳���Ч�������޳�֮����������ûʲô���ˡ�

	gl_FragDepth��--------------��д
		��Ȼgl_FragCoord���Ի�ȡ��Ƭ�εĴ��������Լ���ȣ����ǲ����Զ�������޸ġ�
		ʵ���ϣ����ǿ���ͨ��gl_FragDepth���޸�Ƭ�ε������Ϣ��

			eg:
				gl_FragDepth = 0.0; // this fragment now has a depth value of 0.0
				����ֻ��Ҫ�򵥵�д��0.0-1.0�ĸ������ͺ��ˡ�

		���shaderû����ñ���д��ֵ��������Զ��Ļ�ȡgl_FragCoord.z��ֵ��
		
		��
		��Ҫע���ⲻ��һ���õ����飬��ʵ�ϣ�������ǲ���gl_FragCoord�����ֶ����ƵĻ���
		OpenGL���������ǰ��Ȳ���
		�����Ͼ�û�취��ǰ֪�����ǵ��޸Ĳ������������shader�н������޸ģ�����ǰ��Ȳ��Ի�ʧЧ��
		
		//���� �� 4.2�汾�ṩ�˸����µ����ԣ����������ǽ������ߵĵ��͡�
			layout (depth_<condition>) out float gl_FragDepth;
				Condition				Description
					any				�ر�������Ȳ���
					greater			openGLĬ������ֻд���gl_FragCoord.z�����ֵ��
					less			openGLĬ������ֻд���gl_FragCoord.z��С��ֵ��
				  unchanged			Ҫд��gl_FragDepth��ֻ��д��gl_FragCoord.z��ֵ
		����OpenGL

*/

/*
	Interface blocks �ӿڿ�

	��ĿǰΪֹ��������Ҫ�Ӷ�����ɫ����Ƭ����ɫ����������ʱ������һֱ��ͨ����
		in,out ������ƥ����������ķ�ʽ�����ݵġ�

	������򵥵�������ʽ�����ǵ��������ʱ�򣬿�������������Ҫ����ṹ������飡

	GLSL�ṩ��һ�ֽ����ӿڿ�Ķ������������ǡ�
		�����������ǽṹ�壬 ��������ͨ��in �� out ��˵������ ���뻹������飺
		
		...
		out VS_OUT
		{
		vec2 TexCoords;
		} vs_out;

		void main(){
			...
			vs_out.TexCoords = .... ;
		}
	
		��������������һ������vs_out�Ľӿڿ飬�����������е����������
		������ϣ������ɫ���������������Ϊ����ʱ����Ҳ��ǳ����á�

		֮����Ƭ����ɫ���У�����Ҫ���ö�Ӧ������飺

		...
		in VS_OUT
		{
		vec2 TexCoords;
		} fs_in;

		void main(){
			...
			... = fs_in.TexCoords ;
		}
		ע�����Ӧ��һ��:���� VS_OUT ������ʵ������ ������ fs_in���������κ����֣�
		(�����κλ����Ե����֣��ȸ�vs_out)
		
		ֻҪ�����ӿڿ������һ�������Ƕ�Ӧ��������������ƥ��������
		�����ڼ�����ɫ���зǳ����ã�
*/


/*	Uniform buffer object 
		Uniform�������

	�����ڶ����ɫ����ʱ�����ǿ�����Ҫ��������uniform�������������ǵĴ����ֵ����ÿһ��shader��������һ���ġ�
	uniform buffer objects��
		�������Ƕ���һϵ���ڲ�ͬ��ɫ���б�����ֵͬ�ı�����
		ʹ�øñ��������ǽ���Ҫ����һ����ر�����
		��Ȼ��������Ȼ��Ҫ�ֶ���ÿ����ɫ��������Ӧ��uniform������

		ע������һ����������
			1.���ǿ���ͨ��glGenBuffers����
			2.�󶨵�GL_UNIFORM_BUFFERĿ���ϣ�
			3.�����ݱ����ڻ�������
			����Uniform��������д�����������һЩ����ģ�
	eg:
		layout (std140) uniform Matrices{
		mat4 projection ; 
		mat4 view ;
		}
		֮�����ǿ���ʹ��projection��view��������Ҫ�κ�ǰ׺
		layout (std140)����˼��˵:
			��ǰ�����Uniform�����������ʹ��һ���ض����ڴ沼�֡�
			������������Uniform�鲼��(Uniform Block Layout)��
	
	********************************************
	Uniform�鲼��(Uniform Block Layout)
		һ��uniform������ݱ�����һ�������������У�������һ��Ԥ�����ڴ棨ûɶ��ģ�
		��Ϊ����ڴ治�ᱣ���κι����������͵���Ϣ��
			����������Ҫ����openGL��
				�ڴ����һ���ֶ�Ӧshader�е���һ��uniform������
		eg��
			layout (std140) uniform ExampleBlock
			{
				float value;
				vec3  vector;
				mat4  matrix;
				float values[3];
				bool  boolean;
				int   integer;
			};  
		������Ҫ֪��ÿһ�������Ĵ�С�Լ�ƫ�������������ǿ��԰����ǰ��ո��Ե�˳�򱣴��ڻ������С�
			
			��openGL��ÿһ��Ԫ�صĴ�С��������ù涨˵��������ֱ����C++���������Ͷ�Ӧ��
				vectors	��
				matrices��	��������Ӧ�˸������������͡�
			openGLû�����˵�����ǣ������ļ�ࡣ
				������Ӳ�������Լ�����Ϊ���ʵķ�ʽȥ���ñ�����
					������һЩӲ���Ὣvec3 ��float��������һ��
				���������е�Ӳ����ˣ���Щ���ܻ��vec3����һ��4��float�����顣
			
			���ַ�ʽ��
				1.������	
					Ĭ������£�GLSLʹ��һ�ֱ���Ϊ�����ֵ��ڴ沼�ַ�����
			��������Ϊһ��ƫ������Ӳ�����壬���ǽ��ڶ�������һ�¹���

			ʹ�ù�����ʱ��GLSL�ǿ���Ϊ���Ż�����uniform������λ�ý��б䶯�ģ�ֻҪ������˳�򱣳ֲ��䡣
			��Ϊ���ǲ�֪��ÿ��������ƫ�������������ǲ�֪�������׼ȷ������ǵ�Uniform���塣
				2.std140
					std140������ʽΪÿһ���������͵��ڴ沼�֣�ͨ��һϵ�й������������ǵ����ƫ������
			����������ʽ�ἰ�ģ����ǿ����ֶ������ÿ��������ƫ������
					ÿ����������һ����׼��������base Alignment����������һ��������ռ�ݵĿռ䣨�����������Padding��
			����ʹ��std140�Ĳ��ֹ�����м���ġ�Ȼ�󣬶���ÿһ�����������ǻ�������Ķ���ƫ������
			�ö���ƫ������һ�������Ի���鿪ʼ���ֽ�ƫ������һ�������Ķ����ֽ�ƫ�Ʊ����ǻ�׼�������ı�����
				
				������г��󲿷�ͨ�ù���
					��GLSL�еĴ󲿷ֱ�����������Ϊ���ֽ�����ÿ4���ֽڽ�����һ��N����ʾ
						
						����								���ֹ���

				����������int��bool					ÿ�������Ļ�׼������ΪN��
						����					2N����4N������ζ��vec3�Ļ�׼������Ϊ4N��
				  ����������������				 ÿ��Ԫ�صĻ�׼��������vec4����ͬ��
						����			����Ϊ�����������飬ÿ�������Ļ�׼��������vec4����ͬ��
					�ṹ��				��������Ԫ�ظ��ݹ�������Ĵ�С��������䵽vec4��С�ı�����
		
			��������������������
			layout (std140) uniform ExampleBlock
				{
											��׼������			//����ƫ����
					float value;				4					 // 0 
					vec3  vector;				16					 // 16  (must be multiple of 16 so 4->16)			
					mat4  matrix;				16					 // 32  (column 0)
												16					 // 48  (column 1)
												16					 // 64  (column 2)
												16					 // 80  (column 3)
					float values[3];			16					 // 96  (values[0])
												16					 // 112 (values[1])
												16					 // 128 (values[2])
					bool  boolean;				4					 // 144
					int   integer;				4					 // 148
					
					//
						ע������int λ����148.����uniform�Ļ�����Ϊ152
				};
			!����һ���ؼ������ vec4����Ӧ4N������������Ԫ�ػ���������������У�����vec4

		����std140���ֵĹ������Ǿ���ʹ������glBufferSubData�ĺ������������ݰ���ƫ����������������
		��Ȼstb140�������Ч�ģ��������ٱ�֤��ÿ�������µ����uniform��������һ�µġ�


		���ԣ��ڶ���uniform���ʱ����������Ϊ std140�����Ǹ���openGLʹ��std140���֡�
		����֮�⣬�����������ֹ�ѡ�����Ƕ���Ҫ�������ǰ��ѯƫ������
				1.shared	����
				2.packed	���գ�
					packed���Ͳ��ֲ��ܱ�֤��ÿ�������ж����ֲ��䣬
					��Ϊ�����������ȥ��uniform������Uniform�����Ż���������ÿ����ɫ���ж������ǲ�ͬ�ġ�
*/

/*
	ʹ��Uniform����
		1.����һ��uniform������
			unsigned int uboExampleBlock;
			glGenBuffers(1,&uboExampleBlock);
			glBindBuffer(GL_UNIFORM_BUFFER , uboExampleBlock);
			glBufferData(GL_UNIFORM_BUFFER , 152 , NULL , GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER,0);
		2.��shader�ͻ������󶨵�Binding Point�ϡ�
		�����ǣ���β�����OpenGL֪���ĸ�shader�µ��ĸ�Uniform�����Ӧ�����ĸ�Uniform����
			��OpenGL�������У�������һЩ�󶨵�(Binding Point)��
				���Խ�һ��Uniform������������������shader�µ�Uniform�������ӵ�ͬһ���󶨵㡣
		��
			PS��Binding point��0��ʼ����������������������

			��1����ȡ��Ӧ��ɫ���µ��ض�����������
				//��ȡUniform��������Uniform������(Uniform Block Index)����ɫ�����Ѷ���Uniform���λ��ֵ����
			unsigned int lights_index = glGetUniformBlockIndex(shaderA.ID, "Lights"); 
			��2�����ض���ɫ�����ض������������󶨵��ض�λ��
			glUniformBlockBinding(shaderA.ID, lights_index, 2);	//��Uniform��󶨵�һ���ض��İ󶨵�
				
				NOTE���о����б�Ҫ�ϲ���1������������
			��3����Ŀ�껺�����󶨵��ض��󶨵��£�
				glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboExampleBlock);
				//or
				glBindBufferRange(GL_UNIFORM_BUFFER, 2, uboExampleBlock, 0, 152);
			��4������������ɣ��������������ݵĸ��ơ�
			eg��Ҫ�����uniform����boolean�����ǿ��������·�ʽ����Uniform�������
				glBindBuffer(GL_UNIFORM_BUFFER, uboExampleBlock);
				int b = true; // GLSL�е�bool��4�ֽڵģ��������ǽ�����Ϊһ��integer
				glBufferSubData(GL_UNIFORM_BUFFER, 144, 4, &b); 
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

*/
unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para);
unsigned int loadCubeMap(std::vector<std::string> texture_faces);
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

unsigned int loadTexture(char const * path);
unsigned int loadVertexToVBO(float const * data);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void FragCoord_Test(GLFWwindow* window);
void UniformBufferObject_Test(GLFWwindow* window);

glm::vec3 cameraPos(0.0f, 0.0f, 2.0f);
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	//
	//FragCoord_Test(window);
	UniformBufferObject_Test(window);
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
		if (openEffect) openEffect = false;
		else openEffect = true;
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

void FragCoord_Test(GLFWwindow* window) {
	//ʹ����Ȳ���
	glEnable(GL_DEPTH_TEST);//��Ȳ���

	glEnable(GL_PROGRAM_POINT_SIZE);//ʹ�ܵ��С���ơ�

	Shader shader("Shader4-8.vertex", "Shader4-8.fragment");
	camera.MovementSpeed = 1.5f;
	//Texture
	unsigned int cubeTexture = loadTexture(std::string("container.jpg").c_str(), GL_REPEAT);
	unsigned int floorTexture = loadTexture(std::string("wall.jpg").c_str(), GL_REPEAT);

	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
	glBindVertexArray(0);
	
	shader.use();
	shader.setInt("texture",0);
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
		//DrawScence
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//first pass
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//cube
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,cubeTexture);
		shader.use();
		model = glm::mat4();
		shader.setMartix("model", model);
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}

inline void MyBindUniform(unsigned int shaderID , const char * uniformBlockName ,unsigned int bindingPoint) {
	unsigned int UniformBlockIndex = glGetUniformBlockIndex(shaderID,uniformBlockName);
	glUniformBlockBinding(shaderID, UniformBlockIndex, bindingPoint);
	return;
}
void UniformBufferObject_Test(GLFWwindow * window) {
	//ʹ����Ȳ���
	glEnable(GL_DEPTH_TEST);//��Ȳ���

	Shader shader_green("Shader4-8.vertex_ubo", "Shader4-8.fragment_ubo1");
	Shader shader_red("Shader4-8.vertex_ubo", "Shader4-8.fragment_ubo2");
	Shader shader_blue("Shader4-8.vertex_ubo", "Shader4-8.fragment_ubo3");
	Shader shader_littleBlue("Shader4-8.vertex_ubo", "Shader4-8.fragment_ubo4");
	camera.MovementSpeed = 1.5f;

	/*
		1.����һ��uniform������
		2.��shader�ͻ������󶨵�Binding Point�ϡ�
		��1����ȡ��Ӧ��ɫ���µ��ض�����������
		
		��2�����ض���ɫ�����ض������������󶨵��ض�λ��
		
		��3����Ŀ�껺�����󶨵��ض��󶨵��£�
		
		��4������������ɣ��������������ݵĸ��ơ�
	*/

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	unsigned int uboMatrices;
	glGenBuffers(1,&uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER , uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 16*4*2, NULL , GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	//����Ӧshader�µ�uniform ��󶨵�0����
	MyBindUniform(shader_blue.ID,"Matrices",0);
	MyBindUniform(shader_red.ID, "Matrices", 0);
	MyBindUniform(shader_green.ID, "Matrices", 0);
	MyBindUniform(shader_littleBlue.ID, "Matrices", 0);
	//��Uniform ������󵽰󶨵�
	glBindBufferRange(GL_UNIFORM_BUFFER , 0 , uboMatrices , 0 , 16*4*2 );
	//���ݸ���
	glBufferSubData(GL_UNIFORM_BUFFER, 0 , 16*4 , &projection[0][0]);
	

	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
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
		//DrawScence
		glm::mat4 model;
		glm::mat4 view = camera.GetViewMatrix();
		glBufferSubData(GL_UNIFORM_BUFFER, 16 * 4, 16 * 4, &view[0][0]);

		//first pass
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//cube
		glBindVertexArray(cubeVAO);

		shader_blue.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f));  // �ƶ������Ͻ�
		shader_blue.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shader_red.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f));  // �ƶ������Ͻ�
		shader_red.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shader_green.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-0.75f, -0.75f, 0.0f));  // �ƶ������Ͻ�
		shader_green.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shader_littleBlue.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.75f, -0.75f, 0.0f));  // �ƶ������Ͻ�
		shader_littleBlue.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
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

unsigned int loadCubeMap(std::vector<std::string> texture_faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	//��Ȼ����ͨ������һ����������Ҫָ���价�ƺ͹��˷�����
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	/*
	ע�����ǽ�����ά�ȵĻ��Ʒ�ʽ������Ϊ��GL_CLAMP_TO_EDGE������Ϊ
	��������֮�������������ܲ����ܴ����ĸ��棨����Ӳ�����ƣ�������ͨ��ʹ��GL_CLAMP_TO_EDGE
	ÿ�����ǲɼ���Ե��ʱ��OpenGL�����Ƿ������ǵı�Եֵ��

	�����GL_TEXTURE_WRAP_R�����������R���꣨��Ӧ������ĵ�����ά�ȣ���

	*/

	int width, height, nrChannels;
	unsigned char *data;
	for (GLuint i = 0; i < texture_faces.size(); i++)
	{
		data = stbi_load(texture_faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap texture failed to load at path: " << texture_faces[i] << std::endl;
			stbi_image_free(data);
		}

	}
	return textureID;
}
