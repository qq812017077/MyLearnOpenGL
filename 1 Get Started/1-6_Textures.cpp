#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>
#include "Shaders.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
/*
	stb_image.h 是一个很好用的头文件，可以用来加载大部分的文件格式。
	并且很容易集成到项目中。

	定义STB_IMAGE_IMPLEMENTATION
		使得预处理器将处理该头文件以至于其仅包含相关定义的源代码
*/


void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void processInput(GLFWwindow * window);
void LoadingPhoto(unsigned int target, const char * photoFilePath, unsigned int format);
/*glTexParameteri
该函数为指定的目标（比如这里是2D纹理）的每一个轴向设置操作方式
*/
//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);//横向使用镜像
//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);//纵向使用镜像
																	  /*	第一个参数：指定目标
																	  第二个参数：指定对哪一个轴执行什么操作：这里为两个轴进行wrap操作，
																	  第三个操作：选择wrap的模式：
																	  GL_REPEAT：纹理默认行为，即重复
																	  GL_MIRRORED_REPEAT：重复+镜像
																	  GL_CLAMP_TO_EDGE：将坐标约束在0到1之间，超出的部分会重复纹理坐标的边缘，产生一种边缘被拉伸的效果
																	  GL_CLAMP_TO_BORDER：超出的坐标为用户指定的边缘颜色：
																	  float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
																	  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);		这里传入一个颜色
																	  */

/*纹理的采集方式：
Texture Sampling:has a loose interpretation and can be done in many different ways.

纹理坐标:(bottom-left: 0,0    top-right: 1,1)

Texture Wrapping 纹理环绕方式:
GL_REPEAT：纹理默认行为，即重复
GL_MIRRORED_REPEAT：重复+镜像
GL_CLAMP_TO_EDGE：将坐标约束在0到1之间，超出的部分会重复纹理坐标的边缘，产生一种边缘被拉伸的效果
GL_CLAMP_TO_BORDER：超出的坐标为用户指定的边缘颜色。

在纹理中，坐标轴为s,t
t(对应y轴)
|   /
|  / r(对应z轴)
| /
|/___________s(对应x轴)
*/

/*纹理过滤
	考虑同一个纹理图对于不同大小物体时，纹理像素与之的对应应如何处理。
		
		考虑纹理坐标和纹理像素的区别：
				纹理坐标：给模型顶点设置的那个数组，
				纹理像素：即图片所对应到的像素，
					
					OpenGL以这个顶点的纹理坐标数据去查找纹理图像上的像素，然后进行采样提取纹理像素的颜色

		纹理过滤中比较重要的两个方式：
			1.GL_NEAREST : 临近过滤，OpenGL会选择：
													纹理像素中心点离纹理坐标最近的那个像素的颜色
					原话：OpenGL selects the pixel which center is closest to the texture coordinate
			2.GL_LINEAR : 线性过滤，OpenGL根据：
													纹理坐标周围的像素进行一个线性插值。只是纹理坐标更近的点贡献更大而已。

	（可以在纹理被缩小的时候使用邻近过滤，被放大时使用线性过滤）
	操作函数：
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
*/


/*Mipmaps
	多级渐远纹理
		一系列的纹理图像，后一个纹理图像是前一个的二分之一
		通过	glGenerateMipmaps	函数来生成MipMaps
		在渲染中切换多级渐远纹理的Level时（比如由远及近会导致该现象），OpenGL会在两个不同级别的纹理层之间产生不真实的生硬边界。
		实际上我们可以指定不同多级纹理层之间的过滤方式：

				GL_NEAREST_MIPMAP_NEAREST：使用最邻近的多级渐远纹理来匹配像素大小，并使用邻近插值进行纹理采样
				GL_LINEAR_MIPMAP_NEAREST：使用最邻近的多级渐远纹理级别，并使用线性插值进行采样
				GL_NEAREST_MIPMAP_LINEAR：在两个最匹配像素大小的多级渐远纹理之间进行线性插值，使用邻近插值进行采样
				GL_LINEAR_MIPMAP_LINEAR：在两个邻近的多级渐远纹理之间使用线性插值，并使用线性插值进行采样

			同样，可以通过glTexParameteri进行其的模式设置

			一个常见的错误是，将放大过滤的选项设置为多级渐远纹理过滤选项之一。这样没有任何效果，
			因为多级渐远纹理主要是使用在纹理被缩小的情况下的：纹理放大不会使用多级渐远纹理，
			为放大过滤设置多级渐远纹理的选项会产生一个GL_INVALID_ENUM错误代码。
*/

/*Loading and creating textures
		加载texture

*/


/*Texture Unit
	实际上我们可以给纹理采集器（Sampler：位于着色器中）赋于位置值。
		这允许我们可以同时使用多个纹理。
	我们称一个纹理的位置叫做 纹理单元(Texture Unit)。 默认值为0，且默认的纹理单元自动激活，因此不需要我们进行位置的赋值操作。
	（注意不是所有的显卡驱动都会赋予默认值！）

	OpenGL至少保证有16个纹理单元供你使用，也就是说你可以激活从GL_TEXTURE0到GL_TEXTRUE15。
	它们都是按顺序定义的，所以我们也可以通过GL_TEXTURE0 + 8的方式获得GL_TEXTURE8，这在当我们需要循环一些纹理单元的时候会很有用。

	glActiveTexture(GL_TEXTURE0); //在绑定纹理之前先激活纹理单元
	glBindTexture(GL_TEXTURE_2D, texture);

*/


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void Texture_1(GLFWwindow * window);

int main(void)
{
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

	Texture_1(window);
	
	//Final
	glfwTerminate();
	return 0;
}

//GLfloat texCoords[] = {
//	0.0f, 0.0f, // 左下角
//	1.0f, 0.0f, // 右下角
//	0.5f, 1.0f // 上中
//};
void Texture_1(GLFWwindow * window){

	//Shader程序
	Shader shader("shader1-6.vertex","shader1-6.fragment");

	//生成贴图
	unsigned int texture[2];
	glGenTextures(2, texture);//通过ID引用，第一个参数同样表示数量。

	//同样需要绑定
	glBindTexture(GL_TEXTURE_2D, texture[0]);//绑定之后的所有配置都针对其进行
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	/*加载Picture*/
	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
	LoadingPhoto(GL_TEXTURE_2D, "container.jpg",GL_RGB);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	LoadingPhoto(GL_TEXTURE_2D, "awesomeface.png", GL_RGBA);
	glBindTexture(GL_TEXTURE_2D, 0);//解绑


	float vertices[] = {
		// positions          // colors           // texture coords
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // top left 
		0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f   // bottom right	
	};
	unsigned int indices[] = {
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
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float) ));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)( 0 + 6 * sizeof(float) ));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);//在使用VBO之后，解除绑定
	glBindVertexArray(0);

	shader.use();
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually
	shader.setInt("ourTexture2", 1); // or with shader class
	while (!glfwWindowShouldClose(window)) {
		//process input 
		processInput(window);

		//rendering commands here....
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		
		//glBindTexture(GL_TEXTURE_2D, texture);	//这里不需要手动的进行uniform的赋值，其会自动进行
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture[1]);

		shader.use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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

void LoadingPhoto(unsigned int target, const char * photoFilePath, unsigned int format) {
	/*加载Picture*/
	int width, height, nrChannels;//宽、高、 number of color channels（颜色通道数）、
	unsigned char *data = stbi_load(photoFilePath, &width, &height, &nrChannels, 0);

	if (data) {
		switch (target) {
		case GL_TEXTURE_2D:
			glTexImage2D(target, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);/*生成纹理
																							  第一个参数：the texture target，意味着会生成与当前绑定的纹理对象在同一个目标上的纹理（不干扰1D和3D）
																							  第二个参数：为纹理指定mipmap的等级，这里我们填0，也就是基本级别
																							  第三个参数：说明想要保存纹理为何种格式，这里的图片为RGB格式。
																							  第四和第五个参数：结果纹理的宽和高。
																							  第六个参数：总是被设为0（历史遗留问题）
																							  第七、八个参数：指定源图片的格式和数据类型。
																							  我们使用RGB值加载这个图像，并把它们储存为char(byte)数组，我们将会传入对应值。
																							  */
																							  //当调用glTexImage2D时，当前绑定的纹理对象就会被附加上纹理图像。但是注意：
																							  //加载的纹理图片仅是最基本的级别。这个时候如果想使用mipmap，将不得不手动生成不同的图片（通过不断增加第二个参数的值）
																							  //好消息是，在生成纹理后，glGenerateMipmap 可以帮助我们自动生成所有需要的纹理。

			glGenerateMipmap(GL_TEXTURE_2D);
			break;
		default:
			break;
		}
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	//生成了纹理和相应的多级渐远纹理后，释放图像的内存并解绑纹理对象是一个很好的习惯。
	stbi_image_free(data);
}