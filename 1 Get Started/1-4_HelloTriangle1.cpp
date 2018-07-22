#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>

//The sections of rendering 
/*The first part of the pipeline:
		Vertex shader takes a single vertex as input.
		Main purpose: transform 3D coordinates into different 3D coordinates 
*/
/*The primitive assembly:	������ɫ��
		1.take data(all the vertices) from the vertex shader 
		2.form a primitive and assembles all the point(s) in the primitive shape given
*/
/*The  geometry shader:������ɫ��
		1.take data(a collection of vertices that form a primitive) from the vertex shader 
		2.emitting new vertices to form new primitives.
*/
/*The  rasterization stage:	��դ��
		1.it maps the resulting primitive(s) to the corresponding pixels on the final screen
		2.resulting in fragments which are used by the fragment shader 
*/
/*Before the fragment shaders runs��
		1.lipping is performed. 
		2.Clipping discards all fragments that are outside your view, increasing performance.
*/
/*The  fragment shaders :	������ɫ��
	//OpenGL�е�һ��fragment��OpenGL��Ⱦһ����������������������ݡ�
	1.calculate the final color of a pixel and this is usually the stage where all the advanced OpenGL effects occur
	2.it can use to calculate the final pixel color (like lights, shadows, color of the light and so on).
*/
/*The alpha test and blending stage	alpha���Ժͻ�ϣ�blending���׶�
	1. checks the corresponding depth (and stencil) value of the fragment
	2. check if the resulting fragment is in front or behind other objects and should be discarded accordingly
	3. checks for alpha values��blends the objects accordingly
	4.
*/
//�ص�
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const  char * vertexShaderSource =  "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
	"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
	"}\n\0";

int main(void) {

	if (!glfwInit()) {		std::cout << "��ʼ��ʧ��";	}

	//2.���ò���
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//3.�������ڼ���Ӧ����
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);// �����Ĵ��ڵ�contextָ��Ϊ��ǰcontext

	//ע��ص����������ڴ�С�ص���
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//4.initialize GLAD��OpenGL������ָ������������
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}


	//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);	û��Ҳ�޷�
	//5.����������Shader����
		//i.������ɫ��
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1 , &vertexShaderSource , NULL);
	glCompileShader(vertexShader);	//��̬����
	int successCompile;//�������Ƿ����
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS ,&successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl; }
		//ii,ƬԪ��ɫ��
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1 , &fragmentShaderSource , NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl; }
		//iii.����
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS , &successCompile);
	if (!successCompile) {
		glGetProgramInfoLog(shaderProgram, sizeof(infoLog)/sizeof(infoLog[0]) , NULL , infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;	}
		//iiii.delete shader
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
	};

	unsigned int VBO, VAO;//������������ID
	//std::cout << "Before:  VBO:" << VBO << "  VAO:" << VAO << std::endl;
	glGenBuffers(1,&VBO);//����1���������,ͨ���ú����Լ�һ������ID��(ע���һ��������ʾ���ɻ�����������)
	glGenVertexArrays(1, &VAO);
	//std::cout << "After:  VBO:" << VBO << "  VAO:" << VAO << std::endl;
/*VAO�б�������ݣ�
	1:�����ü��رյĶ�������
		glEnableVertexAttribArray	��	glDisableVertexAttribArray�ĵ���
	2:�������ԵĽ�����ʽ
		glVertexAttribPointer�����õ�����
	3:�������ö���������ص�VBO
*/
	glBindVertexArray(VAO);//��һ�������������
	glBindBuffer(GL_ARRAY_BUFFER,VBO);//ͨ��������������ID����VBO�󶨵�GL_ARRAY_BUFFER������
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//�����ݸ��Ƶ�GL_ARRAY_BUFFERĿ�꣨VBO����
/*���ĸ���������������ʽ��
	GL_STATIC_DRAW��the data will most likely not change at all or very rarely.
	GL_DYNAMIC_DRAW: the data is likely to change a lot.
	GL_STREAM_DRAW: the data will change every time it is drawn. */
	
	glVertexAttribPointer(0,3 , GL_FLOAT , GL_FALSE , 3*sizeof(float),(void*)0);//����OpenGL����ν����������ݣ�
/*���в����������£�
	��һ������������Ϊ0���ò���ָ��Ҫ���õ����ԣ�
				�ڶ�����ɫ��������ʹ�� layout(location = 0) ������position��λ��ֵ��
				����ϣ�������ݴ��͵���һ���ԣ��ʴ���0.��ʾ�����ݴ���0�����ԣ����Ｔposition
	�ڶ�������������Ϊ3���ò���ָ�����ԵĴ�С��
				��Ϊ������һ��vec3����3��ֵ��ɣ���������3��
	����������������ΪGL_FLOAT�������Ե�����
	���ĸ���˵���Ƿ�ϣ�����ݱ�׼����
				�����������ΪGL_TRUE���������ݶ��ᱻӳ�䵽0�������з�����signed������-1����1֮�䡣
				���ǰ�������ΪGL_FALSE��
	�����������Ϊ3*sizeof(float)������Ϊ����
				�������Ķ���������֮��ļ����
				����Ϊ0��������OpenGL�������岽���Ƕ��٣�ֻ�е���ֵ�ǽ�������ʱ�ſ��ã�
	����������ʾλ�������ڻ�������ʼλ�õ�ƫ������
				������0����Ϊ�����ڻ�������ͷ��
*/
	glEnableVertexAttribArray(0);//����������������λ��ֵ�������ö�������

	glBindBuffer(GL_ARRAY_BUFFER,0);//��ʹ��VBO֮�󣬿��Խ����
									//ע����������ģ�����glVertexAttribPointerע��VBO��Ϊ��ǰ�󶨵Ķ��㻺��������
									//��˿��Խ����ΪVAO�Ѿ������˶�������Ҫ�������ȡ����
									//����glVertexAttribPointer�������VBO�Ͷ������Խ������ӣ�

	glBindVertexArray(0);//���VAO�Ա����VAO������VAOżȻ�޸ģ����Ǽ������ᷢ����

	//����ģʽ��GL_LINE��ʾ���߿�����ģʽ����
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	//6.�����
	while (!glfwWindowShouldClose(window))
	{
		//input
		processInput(window);

		//rendering commands here....
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//����
		glUseProgram(shaderProgram);	//ʹ�ó���
		glBindVertexArray(VAO);			//
		glDrawArrays(GL_TRIANGLES , 0 , 3);//�ڶ�������ָ����������������������ָ��������

		glfwSwapBuffers(window);	//����ָ�����ڵ�˫���棨ǰ�󻺴棩����ɫ����
		glfwPollEvents();//����Ƿ������¼����������������´���״̬�����ö�Ӧ�Ļص�����
	}

	//���һ���£��ͷ���Դ
	glfwTerminate();	
	return 0;
}

//��ⰴ������
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
//�ص�����
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}