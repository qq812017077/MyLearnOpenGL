#include <glad\glad.h>
#include <glfw3.h>

//数学库
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
	built-in variables 内置变量类型
	我们之前使用了gl_Position，其是顶点着色器的裁减空间输出位置向量。
	如果想要在屏幕中渲染任何东西，设置该变量是必不可少的要求。
*/

/*vertex shader variables
---------------------------------
	gl_PointSize：
		另外的一个我们可选用的渲染图元（render primitive）是GL_POINTS。
		每一个单个的顶点都是一个图元，并且可被渲染为一个点。
		我们可以设置要渲染的点的大小，通过该OpenGL的 gl_PointSize函数。
		或者，我们可以在shader着色器中来影响这个值。
		
		GLSL定义了一个输出变量叫：gl_PointSize
		这是一个float变量，可以设置点的宽度和高度（单位是像素）。
		通过在顶点着色器中修改这个点的大小，我们可以控制每个顶点的大小。

		注意该效果默认被关闭，需要使能之后才能控制：
			glEnable(GL_PROGRAM_POINT_SIZE);  
		然后修改顶点着色器：
			eg：
				void main(){
					gl_Position = projection * view * model * vec4(aPos, 1.0);    
					gl_PointSize = gl_Position.z;    
				}
		修改点大小为z值，则离的越远，越大。 emmm , 很不合常理。
	
	注意gl_Position和gl_PointSize是输出变量，我们可以通过修改它们来影响结果。

	gl_VertexID：
		顶点着色器也给了我们一个输入变量，我们只能读取：
			gl_VertexID  ------> integer

		整形变量包含了我们正在绘制的顶点的ID。
		当使用索引渲染的时候该值包含了绘制顶点的索引，
		当不使用索引绘制的时候，该值包含了从渲染调用开始到当前已处理的顶点数。
*/

/*fragment shader variables
----------------------------
		除了最近本的 FragColor ;
	还提供了两个好用的输入变量：
		gl_FragCoord： -------只读
			(读取当前片段的窗口坐标，以及其深度)
			gl_FragCoord包含 x , y , z 三个坐标分量；
			其中：
				x,y组分是片段的空间坐标，以窗口的左下角为原点，
				当我们使用glViewport设置一个800*600的窗口时，
					x range ： 0 - 800 ； 
					y range ： 0 - 600 ；
			
		gl_FrontFacing： bool类型值。
			该变量告诉我们，当前片段是属于正向面的一部分，还是背向面的一部分。
			（OpenGL能够分辨出面是正向面还是背向面）
			注意这个和面剔除的效果。面剔除之后，这个背面就没什么用了。

	gl_FragDepth：--------------可写
		虽然gl_FragCoord可以获取到片段的窗口坐标以及深度，但是不可以对其进行修改。
		实际上，我们可以通过gl_FragDepth来修改片段的深度信息。

			eg:
				gl_FragDepth = 0.0; // this fragment now has a depth value of 0.0
				我们只需要简单的写入0.0-1.0的浮点数就好了。

		如果shader没有向该变量写入值，则其会自动的获取gl_FragCoord.z的值。
		
		！
		但要注意这不是一个好的事情，事实上，如果我们不对gl_FragCoord进行手动控制的话，
		OpenGL将会进行提前深度测试
		（它毕竟没办法提前知道我们的修改操作，所以如果shader中进行了修改，则提前深度测试会失效）
		
		//不过 在 4.2版本提供了更多新的特性，这允许我们进行两者的调和。
			layout (depth_<condition>) out float gl_FragDepth;
				Condition				Description
					any				关闭提起深度测试
					greater			openGL默认我们只写入比gl_FragCoord.z更大的值。
					less			openGL默认我们只写入比gl_FragCoord.z更小的值。
				  unchanged			要写入gl_FragDepth则只能写入gl_FragCoord.z的值
		这让OpenGL

*/

/*
	Interface blocks 接口块

	到目前为止，当我们要从顶点着色器朝片段着色器传递数据时，我们一直是通过：
		in,out 这样的匹配输入输出的方式来传递的。

	这是最简单的声明方式，但是当程序变大的时候，可能我们甚至想要传输结构体和数组！

	GLSL提供了一种叫做接口块的东西来帮助我们。
		它看起来像是结构体， 不过我们通过in 和 out 来说明其是 输入还是输出块：
		
		...
		out VS_OUT
		{
		vec2 TexCoords;
		} vs_out;

		void main(){
			...
			vs_out.TexCoords = .... ;
		}
	
		上面我们声明了一个叫做vs_out的接口块，它包含了所有的输出变量，
		当我们希望将着色器的输入或输出打包为数组时，它也会非常有用。

		之后，在片段着色器中，我们要设置对应的输入块：

		...
		in VS_OUT
		{
		vec2 TexCoords;
		} fs_in;

		void main(){
			...
			... = fs_in.TexCoords ;
		}
		注意块名应该一致:都是 VS_OUT ；但是实例名： 这里是 fs_in，可以是任何名字，
		(除了任何混淆性的名字，比附vs_out)
		
		只要两个接口块的名字一样，它们对应的输入和输出将会匹配起来。
		（这在几何着色器中非常有用）
*/


/*	Uniform buffer object 
		Uniform缓冲对象

	当存在多个着色器的时候，我们可能需要不断设置uniform变量，而且他们的大多数值对于每一个shader来讲都是一样的。
	uniform buffer objects：
		允许我们定义一系列在不同着色器中保持相同值的变量。
		使用该变量，我们仅需要设置一次相关变量。
		当然，我们仍然需要手动给每个着色器设置相应的uniform变量。

		注意这是一个缓冲区：
			1.我们可以通过glGenBuffers创建
			2.绑定到GL_UNIFORM_BUFFER目标上，
			3.把数据保存在缓冲区内
			（在Uniform缓冲对象中储存数据是有一些规则的）
	eg:
		layout (std140) uniform Matrices{
		mat4 projection ; 
		mat4 view ;
		}
		之后我们可以使用projection和view，而不需要任何前缀
		layout (std140)的意思是说:
			当前定义的Uniform块对它的内容使用一个特定的内存布局。
			这个语句设置了Uniform块布局(Uniform Block Layout)。
	
	********************************************
	Uniform块布局(Uniform Block Layout)
		一个uniform块的内容保存在一个缓冲区对象中，它就是一块预留的内存（没啥别的）
		因为这块内存不会保存任何关于数据类型的信息，
			所以我们需要告诉openGL：
				内存的哪一部分对应shader中的哪一个uniform变量。
		eg：
			layout (std140) uniform ExampleBlock
			{
				float value;
				vec3  vector;
				mat4  matrix;
				float values[3];
				bool  boolean;
				int   integer;
			};  
		我们想要知道每一个变量的大小以及偏移量，所以我们可以把他们按照各自的顺序保存在缓冲区中。
			
			在openGL中每一个元素的大小都被清楚得规定说明，并且直接与C++的数据类型对应。
				vectors	：
				matrices：	这两个对应了浮点数数组类型。
			openGL没有清楚说明的是，变量的间距。
				这允许硬件按照自己所认为合适的方式去放置变量：
					（比如一些硬件会将vec3 和float变量连在一起。
				但不是所有的硬件如此，有些可能会把vec3填充成一个4个float的数组。
			
			布局方式：
				1.共享布局	
					默认情况下，GLSL使用一种被称为共享布局的内存布局方法。
			共享是因为一旦偏移量被硬件定义，它们将在多个程序间一致共享，

			使用共享布局时，GLSL是可以为了优化而对uniform变量的位置进行变动的，只要变量的顺序保持不变。
			因为我们不知道每个变量的偏移量，所以我们不知道该如何准确填充我们的Uniform缓冲。
				2.std140
					std140布局显式为每一个变量类型的内存布局，通过一系列规则来决定他们的相对偏移量。
			由于这是显式提及的，我们可以手动计算出每个变量的偏移量。
					每个变量都有一个基准对齐量（base Alignment），它等于一个变量所占据的空间（包含了填充量Padding）
			其是使用std140的布局规则进行计算的。然后，对于每一个变量，我们会计算它的对齐偏移量，
			该对齐偏移量是一个变量自缓冲块开始的字节偏移量，一个变量的对齐字节偏移必须是基准对齐量的倍数。
				
				下面会列出大部分通用规则：
					在GLSL中的大部分变量都被定义为四字节量，每4个字节将会用一个N来表示
						
						类型								布局规则

				标量，比如int和bool					每个标量的基准对齐量为N。
						向量					2N或者4N。这意味着vec3的基准对齐量为4N。
				  标量或向量的数组				 每个元素的基准对齐量与vec4的相同。
						矩阵			储存为列向量的数组，每个向量的基准对齐量与vec4的相同。
					结构体				等于所有元素根据规则计算后的大小，但会填充到vec4大小的倍数。
		
			如对于上面的例子来讲：
			layout (std140) uniform ExampleBlock
				{
											基准对齐量			//对齐偏移量
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
						注意最后的int 位置在148.整个uniform的缓冲区为152
				};
			!所以一个关键点就是 vec4，对应4N，无论是数组元素还是向量，矩阵的列，都是vec4

		根据std140布局的规则，我们就能使用像是glBufferSubData的函数将变量数据按照偏移量填充进缓冲中了
		虽然stb140不是最高效的，但是至少保证了每个程序下的这个uniform缓冲区是一致的。


		所以，在定义uniform块的时候声明布局为 std140，我们告诉openGL使用std140布局。
		除此之外，还有两个布局供选择，他们都需要我们填充前查询偏移量：
				1.shared	共享；
				2.packed	紧凑；
					packed类型布局不能保证在每个程序中都保持不变，
					因为它允许编译器去将uniform变量从Uniform块中优化掉，这在每个着色器中都可能是不同的。
*/

/*
	使用Uniform缓冲
		1.创建一个uniform缓冲区
			unsigned int uboExampleBlock;
			glGenBuffers(1,&uboExampleBlock);
			glBindBuffer(GL_UNIFORM_BUFFER , uboExampleBlock);
			glBufferData(GL_UNIFORM_BUFFER , 152 , NULL , GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER,0);
		2.将shader和缓冲区绑定到Binding Point上。
		（考虑，如何才能让OpenGL知道哪个shader下的哪个Uniform缓冲对应的是哪个Uniform块呢
			在OpenGL上下文中，定义了一些绑定点(Binding Point)，
				可以将一个Uniform缓冲链接至它，并将shader下的Uniform缓冲链接到同一个绑定点。
		）
			PS：Binding point从0开始递增，类似于数组索引。

			（1）获取对应着色器下的特定缓冲区索引
				//获取Uniform块索引：Uniform块索引(Uniform Block Index)是着色器中已定义Uniform块的位置值索引
			unsigned int lights_index = glGetUniformBlockIndex(shaderA.ID, "Lights"); 
			（2）将特定着色器的特定缓冲区索引绑定到特定位置
			glUniformBlockBinding(shaderA.ID, lights_index, 2);	//将Uniform块绑定到一个特定的绑定点
				
				NOTE：感觉很有必要合并成1个函数啊。。
			（3）将目标缓冲区绑定到特定绑定点下，
				glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboExampleBlock);
				//or
				glBindBufferRange(GL_UNIFORM_BUFFER, 2, uboExampleBlock, 0, 152);
			（4）所有链接完成，接下来进行数据的复制。
			eg：要想更新uniform变量boolean，我们可以用以下方式更新Uniform缓冲对象：
				glBindBuffer(GL_UNIFORM_BUFFER, uboExampleBlock);
				int b = true; // GLSL中的bool是4字节的，所以我们将它存为一个integer
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

	//注册环境
	glfwMakeContextCurrent(window);
	//初始化回调
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
鼠标向左移动，x值减小；向右移动，x值增大
鼠标向上移动，y值减小；向下移动，y值增大
*/
bool firstMouse = true;
static float lastX = 400.0f, lastY = 300.0f;
void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
	std::cout << xPos << "   " << yPos << std::endl;
	if (firstMouse) // this bool variable is initially set to true
	{
		//第一次调用时的xPos,yPos和lastX,lastY相差巨大，需要进行一次调整
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	//std::cout << "xPos:  " << xPos << "yPos:  " << yPos << std::endl;
	float sensitivity = 0.05f;	//敏感度
	float xOffset = xPos - lastX;	//向左移动为负，向右移动为正。
	float yOffset = lastY - yPos;	//向上移动为正，向下移动为负。

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
		//使用空格来开关镜面反射效果
		if (openEffect) openEffect = false;
		else openEffect = true;
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);

	//相机按键控制
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
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	glEnable(GL_PROGRAM_POINT_SIZE);//使能点大小控制。

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
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	Shader shader_green("Shader4-8.vertex_ubo", "Shader4-8.fragment_ubo1");
	Shader shader_red("Shader4-8.vertex_ubo", "Shader4-8.fragment_ubo2");
	Shader shader_blue("Shader4-8.vertex_ubo", "Shader4-8.fragment_ubo3");
	Shader shader_littleBlue("Shader4-8.vertex_ubo", "Shader4-8.fragment_ubo4");
	camera.MovementSpeed = 1.5f;

	/*
		1.创建一个uniform缓冲区
		2.将shader和缓冲区绑定到Binding Point上。
		（1）获取对应着色器下的特定缓冲区索引
		
		（2）将特定着色器的特定缓冲区索引绑定到特定位置
		
		（3）将目标缓冲区绑定到特定绑定点下，
		
		（4）所有链接完成，接下来进行数据的复制。
	*/

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	unsigned int uboMatrices;
	glGenBuffers(1,&uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER , uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 16*4*2, NULL , GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	//将对应shader下的uniform 块绑定到0号上
	MyBindUniform(shader_blue.ID,"Matrices",0);
	MyBindUniform(shader_red.ID, "Matrices", 0);
	MyBindUniform(shader_green.ID, "Matrices", 0);
	MyBindUniform(shader_littleBlue.ID, "Matrices", 0);
	//绑定Uniform 缓冲对象到绑定点
	glBindBufferRange(GL_UNIFORM_BUFFER , 0 , uboMatrices , 0 , 16*4*2 );
	//数据复制
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
		model = glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f));  // 移动到左上角
		shader_blue.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shader_red.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f));  // 移动到右上角
		shader_red.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shader_green.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-0.75f, -0.75f, 0.0f));  // 移动到左上角
		shader_green.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shader_littleBlue.use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.75f, -0.75f, 0.0f));  // 移动到左上角
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

	//仍然和普通的纹理一样，我们需要指定其环绕和过滤方法。
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	/*
	注意我们将三个维度的环绕方式都设置为了GL_CLAMP_TO_EDGE，是因为
	在两个面之间的纹理坐标可能并不能触及哪个面（由于硬件限制），所以通过使用GL_CLAMP_TO_EDGE
	每当我们采集边缘的时候，OpenGL将总是返回他们的边缘值。

	这里的GL_TEXTURE_WRAP_R设置了纹理的R坐标（对应了纹理的第三个维度），

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
