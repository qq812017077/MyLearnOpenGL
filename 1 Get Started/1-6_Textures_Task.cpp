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
unsigned int processInput(GLFWwindow * window);
void LoadingPhoto(unsigned int target, const char * photoFilePath, unsigned int format);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void Texture_Task1(GLFWwindow * window);
void Texture_Task2(GLFWwindow * window);
void Texture_Task3(GLFWwindow * window);
void Texture_Task4(GLFWwindow * window);
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

	//Texture_Task1(window);
	//Texture_Task3(window);
	Texture_Task4(window);
	//Final
	glfwTerminate();
	return 0;
}

/*Make sure only the happy face looks in the other/reverse direction by changing the fragment shader
仅仅修改fragment shader 让笑脸朝相反的方向看：
		只需颠倒纹理的横坐标：
				FragColor = mix(texture(ourTexture1, TexCoord) , texture(ourTexture2, vec2(1 - TexCoord.x,TexCoord.y)) , 0.2f);
*/
void Texture_Task1(GLFWwindow * window) {

	//Shader程序
	Shader shader("shader1-6.vertex", "shader1-6.fragment");

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
	LoadingPhoto(GL_TEXTURE_2D, "container.jpg", GL_RGB);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT  /*GL_MIRRORED_REPEAT*/);//Wrap mode
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
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 6 * sizeof(float)));
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

/*Experiment with the different texture wrapping methods by specifying texture coordinates in the range 0.0f to 2.0f instead of 0.0f to 1.0f
	试试看能不能在箱子的角落放置4个笑脸
	
	解决：实际上，只要单纯将纹理坐标改为0-2范围后，会出现4个重复的笑脸（只修改）
		（因为完整图片的贴图范围永远为0-1）。所以：
		0-2.0f意味着面积×4，需要重复贴四张图片
		0-0.5f意味着面积/4，仅需要1/4个贴图来进行填充

	然而问题是：
			因为这里的纹理坐标是通用的，因此修改意味着同时对箱子和笑脸进行修改。
		即：
			不仅笑脸翻了4倍，箱子也一样。
		有什么办法能让笑脸放置4个，而箱子只是一个呢？（即对两者应用不同的纹理坐标）
*/
void Texture_Task2(GLFWwindow * window) {
	//Shader程序
	Shader shader("shader1-6.vertex", "shader1-6.fragment");

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
	LoadingPhoto(GL_TEXTURE_2D, "container.jpg", GL_RGB);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER  /*GL_MIRRORED_REPEAT*/);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	LoadingPhoto(GL_TEXTURE_2D, "awesomeface.png", GL_RGBA);
	glBindTexture(GL_TEXTURE_2D, 0);//解绑


	float vertices[] = {
		// positions          // colors           // texture coords
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 2.0f,  // top left 
		0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   2.0f, 2.0f,   // top right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   2.0f, 0.0f   // bottom right	
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
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 6 * sizeof(float)));
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

/*Try to display only the center pixels of the texture image on the rectangle in such a way that the individual pixels are getting visible by changing the texture coordinates. 
	Set the texture filter to GL_NEAREST to see the pixels more clearly

	尝试仅仅显示单个像素（通过改变纹理坐标），达到仅仅显示图片中间的单个像素。记得使用 GL_NEAREST 以控制显示更清晰
*/
void Texture_Task3(GLFWwindow * window) {
	//Shader程序
	Shader shader("shader1-6.vertex", "shader1-6.fragment");

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
	LoadingPhoto(GL_TEXTURE_2D, "container.jpg", GL_RGB);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER  /*GL_MIRRORED_REPEAT*/);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	LoadingPhoto(GL_TEXTURE_2D, "awesomeface.png", GL_RGBA);
	glBindTexture(GL_TEXTURE_2D, 0);//解绑


	float vertices[] = {
		// positions          // colors           // texture coords
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.49f, 0.51f,  // top left 
		0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   0.51f, 0.51f,   // top right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.49f, 0.49f,   // bottom left
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   0.51f, 0.49f   // bottom right	
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
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 6 * sizeof(float)));
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
void Texture_Task4(GLFWwindow * window) {
	//Shader程序
	Shader shader("shader1-6.vertex", "shader1-6_Task4.fragment");

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
	LoadingPhoto(GL_TEXTURE_2D, "container.jpg", GL_RGB);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER  /*GL_MIRRORED_REPEAT*/);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);//在使用VBO之后，解除绑定
	glBindVertexArray(0);

	shader.use();
	float mixValue = 0;
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually
	shader.setInt("ourTexture2", 1); // or with shader class
	shader.setFloat1f("mixValue", mixValue);
	unsigned int curKey = -1, lastKey = -1;
	
	while (!glfwWindowShouldClose(window)) {
		//process input
		lastKey = curKey;
		curKey = processInput(window);

		
		//rendering commands here....
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		//glBindTexture(GL_TEXTURE_2D, texture);	//这里不需要手动的进行uniform的赋值，其会自动进行
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture[1]);

		shader.use();
		if(lastKey == -1)
			switch (curKey) {
			case GLFW_KEY_UP:
				shader.setFloat1f("mixValue", ( abs(mixValue  < 0.97f) ? (mixValue += 0.1f) : (mixValue)));
				break;
			case GLFW_KEY_DOWN:
				shader.setFloat1f("mixValue", ((mixValue > 0.03f) ? (mixValue -= 0.1f) : (mixValue)));
				break;
			}
		std::cout << mixValue << std::endl;
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
//检测按键输入
unsigned int  processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ) {
		return  GLFW_KEY_UP;
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		return  GLFW_KEY_DOWN;
	}
	else {
		return  -1;
	}
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
			glTexImage2D(target, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);

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

