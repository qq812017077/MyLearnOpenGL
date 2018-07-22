//OpenGL初始化库
#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//图像库
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "Shaders.h"

/*
stb_image.h 是一个很好用的头文件，可以用来加载大部分的文件格式。
并且很容易集成到项目中。

定义STB_IMAGE_IMPLEMENTATION
使得预处理器将处理该头文件以至于其仅包含相关定义的源代码
*/

/*矩阵
matrix : transformation
once you grow accustomed to them, they'll prove extremely useful.
try to understand them as much as you can.	(try your best to understand)
*/

/*non-uniform scale 不均匀缩放
不同的轴的缩放程度不一，如：
| 0.5  0  0  0 |     | 3 |
|  0   2  0  0 |  *  | 2 |
|  0   0  1  0 |	 | 0 |
|  0   0  0  1 |     | 1 |
x y z 轴的缩放程度均不一样
(第四个维度的意义不表示坐标，注意这是个三维空间，第四个参数以后说明)
*/

/*Translation	位移
和缩放矩阵一样，在4×4矩阵上有几个特别的位置用来执行特定的操作，对于位移来说它们是第四列最上面的3个值。
如果我们把位移向量表示为(Tx,Ty,Tz)(Tx,Ty,Tz)，我们就能把位移矩阵定义为：
|  1   0  0  Tx |     | x |		| x + Tx |
|  0   1  0  Ty |  *  | y |	=	| y + Ty |
|  0   0  1  Tz |	  | z |		| z + Tz |
|  0   0  0  1  |     | 1 |		|   1  	 |
注意这个是符合向量乘法的！！
且注意，正因为w分量为1， Tx，Ty，Tz具有平移的意义！
*/

/*Homogeneous coordinates 齐次坐标
| x |
| y |
| z |
| w |
一个向量的w分量被称为  齐次坐标。
想要得到一个向量的3维向量，把x、y、z依次除以w坐标即可。
实际上，w分量一般为0.也就是说，一般情况下，x,y,z就代表了坐标。

齐次坐标的优势：
允许我们对3维向量进行平移（若没有w分量，我们没法进行平移操作）
并且我们可以利用w分量创建3D效果！！
NOTE:
如果一个向量的齐次坐标是0，这个坐标就是方向向量(Direction Vector)，
因为w坐标是0，这个向量就不能位移（译注：这也就是我们说的不能位移一个方向）。

有了位移矩阵我们就可以在3个方向(x、y、z)上移动物体，它是我们的变换工具箱中非常有用的一个变换矩阵。
*/

/*Rotation  旋转
在3维中，旋转由角度和旋转轴定义。
在3维中，2维物体的旋转轴设定为z轴
绕X轴旋转
|  1   0		0	 0  |     | x |		|		x		   |
|  0  cosθ  -sinθ  0  |  *  | y |	=	| cosθy − sinθz  |
|  0  sinθ   cosθ  0  |	  | z |		| sinθy + cosθz  |
|  0   0		0	 1	|     | 1 |		|		1	  	   |
绕Y轴旋转
|  cosθ	 0		sinθ	0	|		| x |		| cosθx + sinθz  |
|   0		1		 0		0	|  *	| y |	=	|		 y		   |
|  -sinθ	0		cosθ	0	|		| z |		| -sinθx + cosθz |
|  0		0		0		1	|		| 1 |		|		1	  	   |
绕Z轴旋转
|  cosθ  -sinθ	0	0	|     | x |		| cosθx − sinθy	|
|  sinθ   cosθ	0	0	|  *  | y |	=	| sinθx + cosθy	|
|  0		0		1	0	|	  | z |		|		z			|
|  0		0		0	1	|     | 1 |		|		1	  	    |

Gimbal Lock(万向节死锁)

*/

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void processInput(GLFWwindow * window);
void LoadingPhoto(unsigned int target, const char * photoFilePath, unsigned int format);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


void Transformation_1(GLFWwindow * window);

int main(void)
{
	/*glm::mat4 trans;
	trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
	std::cout << glm::value_ptr(trans) << std::endl;
	system("pause");*/
	if (!glfwInit()) {
		std::cout << "Failed to Initialization" << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "1-5_Shaders_", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//初始化GLAD库
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Transformation_1(window);


	glfwTerminate();
	//system("pause");
	return 0;
}

const char * Photo_Path = "container.jpg";
void Transformation_1(GLFWwindow * window) {
	Shader shader("shader1-7.vertex", "shader1-7.fragment");

	//贴图
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//加载图片
	stbi_set_flip_vertically_on_load(true);//	调转y轴
	int width, height, nrChannels;
	unsigned char * data = stbi_load(Photo_Path, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);//为目标生成多级纹理
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	//生成了纹理和相应的多级渐远纹理后，释放图像的内存并解绑纹理对象是一个很好的习惯。
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);//解绑

	float vertices[] = {
		// positions          // colors           // texture coords
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // top left 
		0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f   // bottom right	
	};

	unsigned int indices[] =
	{
		0,1,2,
		1,2,3
	};

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	/*glm::mat4 trans;
	trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));*/
	//std::cout << glm::value_ptr(trans) << std::endl;
	while (!glfwWindowShouldClose(window)) {
		//process input 
		processInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, texture);//使用的时候注意绑定
		shader.use();
		glm::mat4 trans;
		/*
		尽管代码中我们是先进行位移，然后再旋转的，但是实际上
		首先应该是执行旋转，然后再位移。
		*/
		trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
		trans = glm::rotate(trans, glm::radians((float)glfwGetTime()), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
		/*
		第一个参数：说明发送的位置：对应的uniform是哪个
		第二个参数：说明发送的矩阵数量
		第三个参数：是否要对矩阵进行转置
		OpenGL开发者通常使用一种内部矩阵布局，叫做列主序(Column-major Ordering)布局。GLM的默认布局就是列主序，所以并不需要置换矩阵，我们填GL_FALSE。
		第四个参数：
		*/
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void test(void) {
	glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);//齐次坐标为1
	glm::mat4 trans;	//4×4单位矩阵
	trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f)); //进行结合
	vec = trans * vec;	//转换  矩阵×向量进行转换
	std::cout << "x:" << vec.x << "   y:" << vec.y << "   z:" << vec.z << std::endl;
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

