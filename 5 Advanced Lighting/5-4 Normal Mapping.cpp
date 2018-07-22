#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*	注意切线空间：TBN
	T 对应 纹理的U轴
	B 对应 纹理的V轴
	N 对应 纹理的垂直方向，一般与法线相关

	TBN空间一般用于描述纹理的本地空间，用于将纹理从本地空间转换到其所在的世界空间

	要计算一个三角形片段的TBN。
	我们需要知道这个三角形的顶点位置，以及其UV坐标！然后方可计算得到切线空间。
*/

/*
	纹理帮助我们增加了很多的细节，但是当靠近看的时候，便会出现很多的漏洞，
	例如，对于墙壁来讲：
		纹理是没办法增加凹陷感的。墙壁是不可能平坦的。

	以光的视角来看这个问题：是什么使表面被视为完全平坦的表面来照亮？答案会是表面的法线向量。

	这种每个fragment使用各自的法线，替代一个面上所有fragment使用同一个法线的技术叫做法线贴图（normal mapping）
	或凹凸贴图（bump mapping）。

	像diffuse和specular一样，法线贴图也是使用2D纹理来保存。
	由于法线向量是个几何工具，而纹理通常只用于储存颜色信息，用纹理储存法线向量不是非常直接。

		纹理中的颜色向量被表示成了3D的向量：r,g,b
		我们相似的保存了法向量：x，y，z
		法线向量的范围在-1到1之间，所以我们先要将其映射到0到1的范围：
			vec3 rgb_normal = normal * 0.5 + 0.5 ; // 从 [-1,1] 转换至 [0,1]
		将法线向量转换到像这样的RGB组件之后，我们就能根据表面的形状的fragment的法线保存在2D纹理中。

		大部分的法向量贴图都是偏蓝色的，这是因为大部分的法向量都指向z轴方向：（0,0,1）--->蓝色
		颜色轻微的偏移代表了法向量的轻微偏移（偏离z粥正方向）。

*/

/*	
	应用法向量贴图的时候产生了一个问题：法向量的方向是指向z轴的！
	这在平面朝向z轴的时候没有单体，但是当平面换到一个方向的时候就出现问题了！
		
		办法一：
			记录平面所进行的变换，对法向量进行同样的变换！
		办法二：
			在一个不同的坐标空间中进行光照，在这个坐标空间中，法向量贴图永远粗略指向z轴正向；
		所有其他的光向量都被转换到相对于z轴正向的方向。这样我们就能总是使用同一个normalMap。
		这个坐标空间叫做tangent space -- 切线空间

*/

/*tangent space 切线空间
	在法线贴图中的法线向量被传递到tangent space中，在这里法线通常大致指向z轴正向。
	Tangent空间是一个位于三角形表面的空间：法线相对于单个三角形的本地参考系框架。
	可以把它考虑成法线贴图向量的本地空间。无论最终转换到什么方向，它们所有全部被定义为指向z轴方向。
	通过使用一个特定的矩阵：
		我们就可以将法向量从本地的Tangent 空间转换到世界坐标或者观察坐标中，
	即：
		使它们转向到最终的贴图表面的方向。

	Tangent空间的一大好处是：我们可以为任何类型的表面计算这样一个矩阵，使得
	我们能够恰到好处的将切线空间的z轴方向对齐到表面法线向量。

	这种矩阵称为：
		TBN矩阵：分别表示： tangent ， bitangent , normal三个单位向量！
	tangent 切线，   bitangent 副切线，   normal 法线
	
	要建构这样一个把切线空间转变到不同的坐标空间的这样一个变基矩阵，
	我们需要三个相互垂直的向量：
		它们沿一个法线贴图的表面对齐于：上、右、前；这和我们在摄像机教程中做的类似。
	
	其中：
		上向量：Up向量，也即表面的法向量。
		右向量：切向量
		前向量：副切向量
	
	切向量与副切向量的计算：
		法线贴图的切线和副切线与纹理坐标的两个方向对齐： 一个对齐U， 一个对齐V
		在纹理中任取三个点在不同的边上。
			分别为： P1(U1,V1)，P2(U2,V2)，P3(U3,V3) 三个点构成了三角形
			设E1 = P1 - P2			E2 = P2 - P3
			E1=ΔU1T+ΔV1B
			E2=ΔU2T+ΔV2B
		注意T：tangent  和 B: bitangent 都是单位向量。所以	ΔU	ΔV 表示长度，T 和 B表示方向
		所以有：
			(E1x , E1y, E1z) = ΔU1(Tx,Ty,Tz)+ΔV1(Bx,By,Bz)
			(E2x , E2y, E2z) = ΔU2(Tx,Ty,Tz)+ΔV2(Bx,By,Bz)
		| E1x  E1y  E1z | = | ΔU1 ΔV1 | | Tx  Ty  Tz |
		| E2x  E2y  E2z |	| ΔU2 ΔV2 | | Bx  By  Bz |
		把等式写成矩阵形式的好处是，解T和B会因此变得很容易。两边都乘以ΔUΔV的逆矩阵等于：
		| Tx  Ty  Tz | =	| ΔU1 ΔV1 |-1	| E1x  E1y  E1z |
		| Bx  By  Bz |		| ΔU2 ΔV2 |	| E2x  E2y  E2z |
		所以要解这两个向量，我们还是需要知道一些信息：
				三个顶点，ΔU 和 ΔV

		现在我们就可以用公式、三角形的两条边以及纹理坐标计算出切线向量T和副切线B

*/

/*手工计算切线和副切线
	即使用上面的公式计算每个三角形的切线/副切线。
		
		只需为每个三角形计算一个切线/副切线(因为三角形整个面的平的)，它们对于每个三角形上的顶点都是一样的。
		
	要注意的是：
		大多数实现通常三角形和三角形之间都会共享顶点。这种情况下开发者通常将每个
	顶点的法线和切线/副切线等顶点属性平均化，以获得更加柔和的效果。我们的平面的三角形
	之间分享了一些顶点，但是因为两个三角形相互并行，因此并不需要将结果平均化，但无论何
	时只要你遇到这种情况记住它就是件好事。

	最后的切线和副切线向量的值应该是(1, 0, 0)和(0, 1, 0)，它们和法线(0, 0, 1)组成相互垂直的TBN矩阵。
	在平面上显示出来TBN应该是这样的：

*/

/*Tangent space normal mapping 切线空间法线贴图
	要获得法线贴图我们首先为创建一个TBN矩阵在着色器中。
	我们先将前面计算出来的切线和副切线向量传给顶点着色器，作为它的属性
	（注意每个三角形，甚至没有顶点都有独立的切线和副切线，因此应作为属性进行传递）
	像这样：
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec3 aNormal;
		layout (location = 2) in vec2 aTexCoords;
		layout (location = 3) in vec3 aTangent;
		layout (location = 4) in vec3 aBitangent; 

	1.我们首先将所有的TBN向量转换到我们想要工作的坐标系统中，在本次案例中我们将其转换到世界空间中（通过model）
	2.我们创建一个实际的TBN矩阵，直接把相应的向量应用到mat3构造器就行。
		注意如果我们想要精确的结果，我们就使用法线矩阵，这里因为不会进行平移或者缩放处理所以无特殊影响。
		（
			从技术上讲，顶点着色器中无需副切线。所有的这三个TBN向量都是相互垂直的
		所以我们可以在顶点着色器中使用T和N向量的叉乘，自己计算出副切线：vec3 B = cross(T, N);
		）
	3.TBN的使用：
		这里有两种使用方式（将法线转换到世界空间中； 或者将光照从世界空间转换到法线空间）
			1.我们直接使用TBN矩阵，这个矩阵可以把切线坐标空间的向量转换到世界坐标空间。
		因此我们把它传给片段着色器中，把通过采样得到的法线坐标左乘上TBN矩阵，转换到世界
		坐标空间中，这样所有法线和其他光照变量就在同一个坐标系中了。
			
			对于这种情况，我们从法线贴图中采集的法线向量被传递如切线空间中，而其他的光向量被传递入世界空间中。
		通过将TBN矩阵传递给fragment着色器，我们可以将采集的法向量和TBN矩阵相乘，以转换到和光向量同样的参考系空间中。

			2.我们也可以使用TBN矩阵的逆矩阵，这个矩阵可以把世界坐标空间的向量转换到切线坐标空间。
	因此我们使用这个矩阵左乘其他光照变量，把他们转换到切线空间，这样法线和其他光照变量再一次在一个坐标系中了。
	（注意：
		正交矩阵（每个轴既是单位向量同时相互垂直）的一大属性是一个正交矩阵的置换矩阵与它的逆矩阵相等
		其非常重要：
			因为逆矩阵的求得比求置换开销大；结果却是一样的
	）
			在像素着色器中我们不用对法线向量变换，但我们要把其他相关向量转换到切线空间，
		它们是lightDir和viewDir。这样每个向量还是在同一个空间（切线空间）中了：
				
			shader：
				vs_out.TBN = transpose(mat3(T, B, N));

			fragment：
				vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
				normal = normalize(normal * 2.0 - 1.0);   

				vec3 lightDir = fs_in.TBN * normalize(lightPos - fs_in.FragPos);
				vec3 viewDir  = fs_in.TBN * normalize(viewPos - fs_in.FragPos);	
		
		该方法有额外的好处：
			我们可以把所有的相关变量在顶点着色器中转换到切线空间，而不用再像素着色器中。
		这是可行的，因为lightPos和viewPos不是每个fragment运行都要改变，
		对于fs_in.FragPos，我们也可以在顶点着色器计算它的切线空间位置。
		基本上，不需要把任何向量在像素着色器中进行变换，而第一种方法中就是必须的，
		因为采样出来的法线向量对于每个像素着色器都不一样。

		所以现在不是把TBN矩阵的逆矩阵发送给像素着色器，而是将切线空间的光源位置，
		观察位置以及顶点位置发送给像素着色器。这样我们就不用在像素着色器里进行矩阵乘法了。
		这是一个极佳的优化，因为顶点着色器通常比像素着色器运行的少。

		eg:
			shader:
				out VS_OUT {
					vec3 FragPos;
					vec2 TexCoords;
					vec3 TangentLightPos;
					vec3 TangentViewPos;
					vec3 TangentFragPos;
				} vs_out;

				uniform vec3 lightPos;
				uniform vec3 viewPos;
				
				void main()
				{
				[...]
					mat3 TBN = transpose(mat3(T, B, N));
					vs_out.TangentLightPos = TBN * lightPos;
					vs_out.TangentViewPos  = TBN * viewPos;
					vs_out.TangentFragPos  = TBN * vec3(model * vec4(position, 0.0));
				}
			fragment:
				在像素着色器中我们使用这些新的输入变量来计算切线空间的光照。因为法线向量已经在切线空间中了，光照就有意义了。

*/

/*Complex objects
	对于复杂物体，大多数情况切线和副切线的计算还是通过使用模型加载器实现的！
	Assimp有个很有用的配置，在我们加载模型的时候调用aiProcess_CalcTangentSpace。
	当aiProcess_CalcTangentSpace应用到Assimp的ReadFile函数时，Assimp会为每个加载
	的顶点计算出柔和的切线和副切线向量，它所使用的方法和我们本教程使用的类似。

	const aiScene* scene = importer.ReadFile(
	path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
	);
	可以通过下面的代码用Assimp获取计算出来的切线空间：
		vector.x = mesh->mTangents[i].x;
	然后还必须更新模型加载器，用以从带纹理模型中加载法线贴图。
	wavefront的模型格式（.obj）导出的法线贴图有点不一样，Assimp的
	aiTextureType_NORMAL并不会加载它的法线贴图，而aiTextureType_HEIGHT却能，所以我们经常这样加载它们：

			vector<Texture> specularMaps = this->loadMaterialTextures(
			material, aiTextureType_HEIGHT, "texture_normal"
			);
	了解aiProcess_CalcTangentSpace并不能总是很好的工作也很重要。
	计算切线是需要根据纹理坐标的，有些模型制作者使用一些纹理小技巧比如镜像一个模型上的纹理表面时
	也镜像了另一半的纹理坐标；这样当不考虑这个镜像的特别操作的时候（Assimp就不考虑）结果就不对了。

*/

/*最后一件事
	当在更大的网格上计算切线向量的时候，它们往往有很大数量的共享顶点，
	当法线贴图应用到这些表面时将切线向量平均化通常能获得更好更平滑的结果。
	这样做有个问题，就是TBN向量可能会不能互相垂直，这意味着TBN矩阵不再是正交矩阵了。
	法线贴图可能会稍稍偏移，但这仍然可以改进。

	通过格拉姆-施密特正交化过程（Gram-Schmidt process）的数学技巧，对TBN向量进行重正交化，
	这样每个向量就又会重新垂直了。在顶点着色器中我们这样做：
		vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
		vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
		// re-orthogonalize T with respect to N
		T = normalize(T - dot(T, N) * N);
		// then retrieve perpendicular vector B with the cross product of T and N
		vec3 B = cross(T, N);

		mat3 TBN = mat3(T, B, N)

*/
/*
--函数------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);


unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT);
//Brick Wall Normal Test
void BrickWall_NormalTest(GLFWwindow * window);

void TangentSpace(GLFWwindow * window);
/*
--变量------------------------------
*/
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

glm::vec3 cameraPos(-1.0f, 0.0f, 2.0f);
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

	//注册环境
	glfwMakeContextCurrent(window);
	//初始化回调
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//关闭鼠光标
	glfwSetCursorPosCallback(window, mouse_callback);				//打开鼠标回调
	glfwSetScrollCallback(window, scroll_callback);

	//glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}
	TangentSpace(window);
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
		//使用B来控制切换着色方案
		if (functoinExchange) {
			functoinExchange = false;
		}
		else {
			functoinExchange = true;
		}
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);
	Exchange_lastStatus = glfwGetKey(window, GLFW_KEY_B);
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

float planeVertices[] = {
	// x-z 平面
	// positions          // texture Coords		//Normal
	/*-4.0f, -1.0f, -4.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		5.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	4.0f, -1.0f, -4.0f,		5.0f, 5.0f,			0.0f,  1.0f, 0.0f,

	-4.0f, -1.0f, -4.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	-4.0f, -1.0f,  4.0f,	0.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		5.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	*/
	// x-y 平面
	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	-2.0f, -1.0f,	2.0f, 2.0f,			0.0f,  0.0f, 1.0f,
													    	  
	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	-2.0f,	 2.0f, -1.0f,	0.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f
};

void BrickWall_NormalTest(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader shader("Shader5-4.vertex", "Shader5-4.fragment");
	//Texture
	unsigned int brickwallTexture = loadTexture("brickwall.jpg");
	unsigned int normalTexture = loadTexture("brickwall_normal.jpg");
	//VAO , VBO
	//plane
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	//light
	glm::vec3 lightPosition(0.0f, 0.0f, 3.0f);

	unsigned int lightVAO, lightVBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Shader参数设置
	shader.use();
	
	shader.setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.setVec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
	shader.setVec3("light.specular", glm::vec3(0.6f, 0.6f, 0.6f));
	shader.setFloat1f("light.constant", 1.0f);
	shader.setFloat1f("light.linear", 0.09f);
	shader.setFloat1f("light.quadratic", 0.032f);
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setInt("material.normalMap",2);
	shader.setFloat1f("material.shininess", 16.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickwallTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brickwallTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;
		//Draw
		lightPosition.z = (sin(glfwGetTime()) + 1.0f) / 2.0 ;
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//plane
		shader.use();
		model = glm::mat4();
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		shader.setVec3("light.position", lightPosition);
		shader.setMartix("model", model);
		shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setVec3("viewerPos", camera.Position);
		shader.setBool("openEffect", openEffect);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//light
		light_Shader.use();
		model = glm::mat4();
		model = glm::translate(model, lightPosition);
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		light_Shader.setMartix("model", model);
		light_Shader.setMartix("view", view);
		light_Shader.setMartix("projection", projection);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void TangentSpace(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader shader("Shader5-4.vertex", "Shader5-4.fragment");
	//Texture
	unsigned int brickwallTexture = loadTexture("brickwall.jpg");
	unsigned int normalTexture = loadTexture("brickwall_normal.jpg");
	
	
	//切线空间的处理
	// positions----平面的四个顶点				   1(0,1)__________4(1,1)
	glm::vec3 pos1(-2.0, 2.0, 0.0);		//左上		|		   |
	glm::vec3 pos2(-2.0, -2.0, 0.0);		//左下		|		   |
	glm::vec3 pos3(2.0, -2.0, 0.0);		//右下		|		   |
	glm::vec3 pos4(2.0, 2.0, 0.0);		//右上	    |__________|
										// texture coordinates								2(0,0)	   3(1,0)
	glm::vec2 uv1(0.0, 2.0);
	glm::vec2 uv2(0.0, 0.0);
	glm::vec2 uv3(2.0, 0.0);
	glm::vec2 uv4(2.0, 2.0);
	// normal vector
	glm::vec3 normal(0.0, 0.0, 1.0);

	//step1:第一个三角形的边 和 ΔUV
	glm::vec3 edge1 = pos2 - pos1;	//1 -> 2
	glm::vec3 edge2 = pos3 - pos1;	//1 -> 3
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;
	//注意这里的边是有方向的，但是方向如何关系不大，只要Delta与之对应就好了。
	/*公式：
	| Tx  Ty  Tz | =	| ΔU1 ΔV1 |-1	| E1x  E1y  E1z |
	| Bx  By  Bz |		| ΔU2 ΔV2 |	| E2x  E2y  E2z |
	*/
	//计算方法1：

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	glm::vec3 tangent1;
	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent1 = glm::normalize(tangent1);

	glm::vec3 bitangent1;
	bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent1 = glm::normalize(bitangent1);

	//计算方法2
	/*glm::mat2x2 uv(deltaUV1, deltaUV2);
	glm::mat2x3 edge(edge1, edge2);
	glm::mat2x3 result = edge * glm::inverse(uv) ;
	tangent = result[0];
	bitangent = result[1];*/

	//step2:第二个三角形的边 和 ΔUV ---实际上，这里的不进行计算也行，因为整个平面的切线和
	edge1 = pos3 - pos1;	//1 -> 3
	edge2 = pos1 - pos4;	//4 -> 1
	deltaUV1 = uv3 - uv1;
	deltaUV2 = uv1 - uv4;

	f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	glm::vec3 tangent2;
	tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent2 = glm::normalize(tangent1);

	glm::vec3 bitangent2;
	bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent2 = glm::normalize(bitangent1);

	GLfloat quadVertices[] = {
		// Positions            // normal						// TexCoords  // Tangent                         // Bitangent
		pos1.x, pos1.y, pos1.z, normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos2.x, pos2.y, pos2.z, normal.x, normal.y, normal.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos3.x, pos3.y, pos3.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

		pos1.x, pos1.y, pos1.z, normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos3.x, pos3.y, pos3.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos4.x, pos4.y, pos4.z, normal.x, normal.y, normal.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
	};
	//VAO , VBO
	//plane
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 14 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(11 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	//light
	glm::vec3 lightPosition(0.0f, 0.0f, 3.0f);

	unsigned int lightVAO, lightVBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Shader参数设置
	shader.use();

	shader.setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.setVec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
	shader.setVec3("light.specular", glm::vec3(0.6f, 0.6f, 0.6f));
	shader.setFloat1f("light.constant", 1.0f);
	shader.setFloat1f("light.linear", 0.09f);
	shader.setFloat1f("light.quadratic", 0.032f);
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setInt("material.normalMap", 2);
	shader.setFloat1f("material.shininess", 16.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickwallTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brickwallTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;
		//Draw
		lightPosition.z = (sin(glfwGetTime()) + 1.0f) / 2.0 + 0.5f;
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//plane
		shader.use();
		model = glm::mat4();
		model = glm::rotate(model, glm::radians(60.0f * (float)sin(glfwGetTime())), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		shader.setVec3("light.position", lightPosition);
		shader.setMartix("model", model);
		shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setVec3("viewerPos", camera.Position);
		shader.setBool("openEffect", openEffect);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//light
		light_Shader.use();
		model = glm::mat4();
		model = glm::translate(model, lightPosition);
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		light_Shader.setMartix("model", model);
		light_Shader.setMartix("view", view);
		light_Shader.setMartix("projection", projection);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

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
