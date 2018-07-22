#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>

//The sections of rendering 
/*The first part of the pipeline:
		Vertex shader takes a single vertex as input.
		Main purpose: transform 3D coordinates into different 3D coordinates 
*/
/*The primitive assembly:	定点着色器
		1.take data(all the vertices) from the vertex shader 
		2.form a primitive and assembles all the point(s) in the primitive shape given
*/
/*The  geometry shader:几何着色器
		1.take data(a collection of vertices that form a primitive) from the vertex shader 
		2.emitting new vertices to form new primitives.
*/
/*The  rasterization stage:	光栅化
		1.it maps the resulting primitive(s) to the corresponding pixels on the final screen
		2.resulting in fragments which are used by the fragment shader 
*/
/*Before the fragment shaders runs：
		1.lipping is performed. 
		2.Clipping discards all fragments that are outside your view, increasing performance.
*/
/*The  fragment shaders :	像素着色器
	//OpenGL中的一个fragment是OpenGL渲染一个独立像素所需的所有数据。
	1.calculate the final color of a pixel and this is usually the stage where all the advanced OpenGL effects occur
	2.it can use to calculate the final pixel color (like lights, shadows, color of the light and so on).
*/
/*The alpha test and blending stage	alpha测试和混合（blending）阶段
	1. checks the corresponding depth (and stencil) value of the fragment
	2. check if the resulting fragment is in front or behind other objects and should be discarded accordingly
	3. checks for alpha values、blends the objects accordingly
	4.
*/
//回调
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

	if (!glfwInit()) {		std::cout << "初始化失败";	}

	//2.配置参数
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//3.创建窗口及对应环境
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);// 创建的窗口的context指定为当前context

	//注册回调函数（窗口大小回调）
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//4.initialize GLAD：OpenGL函数的指针是由其管理的
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}


	//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);	没有也无妨
	//5.构建并编译Shader程序
		//i.顶点着色器
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1 , &vertexShaderSource , NULL);
	glCompileShader(vertexShader);	//动态编译
	int successCompile;//检查编译是否出错
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS ,&successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl; }
		//ii,片元着色器
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1 , &fragmentShaderSource , NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successCompile);
	if (!successCompile) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl; }
		//iii.链接
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

	unsigned int VBO, VAO;//这是两个引用ID
	//std::cout << "Before:  VBO:" << VBO << "  VAO:" << VAO << std::endl;
	glGenBuffers(1,&VBO);//生成1个缓冲对象,通过该函数以及一个缓冲ID。(注意第一个参数表示生成缓冲区的数量)
	glGenVertexArrays(1, &VAO);
	//std::cout << "After:  VBO:" << VBO << "  VAO:" << VAO << std::endl;
/*VAO中保存的内容：
	1:所启用及关闭的顶点属性
		glEnableVertexAttribArray	和	glDisableVertexAttribArray的调用
	2:顶点属性的解析方式
		glVertexAttribPointer所配置的内容
	3:以所配置顶点属性相关的VBO
*/
	glBindVertexArray(VAO);//绑定一个顶点数组对象
	glBindBuffer(GL_ARRAY_BUFFER,VBO);//通过缓冲对象的引用ID，将VBO绑定到GL_ARRAY_BUFFER类型上
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//把数据复制到GL_ARRAY_BUFFER目标（VBO）上
/*第四个参数存在三种形式：
	GL_STATIC_DRAW：the data will most likely not change at all or very rarely.
	GL_DYNAMIC_DRAW: the data is likely to change a lot.
	GL_STREAM_DRAW: the data will change every time it is drawn. */
	
	glVertexAttribPointer(0,3 , GL_FLOAT , GL_FALSE , 3*sizeof(float),(void*)0);//告诉OpenGL该如何解析顶点数据，
/*所有参数含义如下：
	第一个：这里设置为0，该参数指定要配置的属性：
				在顶点着色器中我们使用 layout(location = 0) 定义了position的位置值。
				我们希望把数据传送到这一属性，故传入0.表示把数据传给0号属性，这里即position
	第二个：这里设置为3，该参数指定属性的大小：
				因为属性是一个vec3，由3个值组成，故这里填3。
	第三个：这里设置为GL_FLOAT，即属性的类型
	第四个：说明是否希望数据标准化。
				如果我们设置为GL_TRUE，所有数据都会被映射到0（对于有符号型signed数据是-1）到1之间。
				我们把它设置为GL_FALSE。
	第五个：设置为3*sizeof(float)，含义为步长
				即连续的顶点属性组之间的间隔。
				设置为0可以来让OpenGL决定具体步长是多少（只有当数值是紧密排列时才可用）
	第六个：表示位置数据在缓冲中起始位置的偏移量。
				这里是0，因为数据在缓冲区开头！
*/
	glEnableVertexAttribArray(0);//参数即：顶点属性位置值，，启用顶点属性

	glBindBuffer(GL_ARRAY_BUFFER,0);//在使用VBO之后，可以解除绑定
									//注意这是允许的，调用glVertexAttribPointer注册VBO作为当前绑定的定点缓冲区对象
									//因此可以解绑，因为VAO已经保存了顶点属性要从哪里获取数据
									//（即glVertexAttribPointer函数会对VBO和定点属性进行链接）

	glBindVertexArray(0);//解绑VAO以避免该VAO被其他VAO偶然修改（但是几乎不会发生）

	//绘制模式，GL_LINE表示以线框多边形模式绘制
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	//6.搭建引擎
	while (!glfwWindowShouldClose(window))
	{
		//input
		processInput(window);

		//rendering commands here....
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//绘制
		glUseProgram(shaderProgram);	//使用程序
		glBindVertexArray(VAO);			//
		glDrawArrays(GL_TRIANGLES , 0 , 3);//第二个参数指定顶点数组索引，第三个指定顶点数

		glfwSwapBuffers(window);	//交换指定窗口的双缓存（前后缓存），颜色缓存
		glfwPollEvents();//检测是否所有事件都被触发，并更新窗口状态，调用对应的回调函数
	}

	//最后一件事，释放资源
	glfwTerminate();	
	return 0;
}

//检测按键输入
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
//回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}