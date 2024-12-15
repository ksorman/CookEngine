#include <GL.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stdio.h>
#include <stdlib.h>

using glm::mat4;
using glm::vec3;

static const char* shaderCodeVertex = R"(
#version 460 core
layout(std140, binding = 0) uniform PerFrameData
{
	uniform mat4 MVP;
	uniform int isWireframe;
};
layout (location=0) out vec3 color;
const vec3 pos[8] = vec3[8](
	vec3(-1.0,-0.0, 1.0),
	vec3( 1.0, 2.0, 1.0),
	vec3( 1.0, 1.0, 1.0),
	vec3( 2.0, 1.0, 1.0),

	vec3( 1.0, 1.0,-1.0),
	vec3( 1.0, 0.0,-1.0),
	vec3( 1.0, 1.0,-1.0),
	vec3( 1.0, 1.0,-1.0)
);
const vec3 col[8] = vec3[8](
	vec3( 1.0, 0.0, 1.0),
	vec3( 1.0, 0.0, 1.0),
	vec3( 1.0, 0.0, 1.0),
	vec3( 1.0, 0.0, 1.0),

	vec3( 0.0, 0.0, 0.0),
	vec3( 0.0, 0.0, 0.0),
	vec3( 0.0, 0.0, 0.0),
	vec3( 0.0, 0.0, 0.0)
);
const int indices[36] = int[36](
	// front
	0, 1, 2, 2, 3, 0,
	// right
	1, 5, 6, 6, 2, 1,
	// back
	7, 6, 5, 5, 4, 7,
	// left
	4, 0, 3, 3, 7, 4,
	// bottom
	4, 5, 1, 1, 0, 4,
	// top
	3, 2, 6, 6, 7, 3
);
void main()
{
	int idx = indices[gl_VertexID];
	gl_Position = MVP * vec4(pos[idx], 1.0);
	color = isWireframe > 0 ? vec3(0.0) : col[idx];
}
)";

static const char* shaderCodeFragment = R"(
#version 460 core
layout (location=0) in vec3 color;
layout (location=0) out vec4 out_FragColor;
void main()
{
	out_FragColor = vec4(color, 1.0);
};
)";

struct PerFrameData
{
	mat4 mvp;
	int isWireframe;
};

int main(void)
{
	glfwSetErrorCallback(
		[](int error, const char* description)
		{
			fprintf(stderr, "Error: %s\n", description);
		}
	);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1024, 768, "Simple example", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}


	glfwSetKeyCallback(
		window,
		[](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	);

	glfwMakeContextCurrent(window);

	GL4API api;

	GetAPI4(&api, [](const char* func) -> void* { return (void*)glfwGetProcAddress(func); });
	InjectAPITracer4(&api);


	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1);

	const GLuint shaderVertex = api.glCreateShader(GL_VERTEX_SHADER);
	api.glShaderSource(shaderVertex, 1, &shaderCodeVertex, nullptr);
	api.glCompileShader(shaderVertex);

	const GLuint shaderFragment = api.glCreateShader(GL_FRAGMENT_SHADER);
	api.glShaderSource(shaderFragment, 1, &shaderCodeFragment, nullptr);
	api.glCompileShader(shaderFragment);

	const GLuint program = api.glCreateProgram();
	api.glAttachShader(program, shaderVertex);
	api.glAttachShader(program, shaderFragment);
	api.glLinkProgram(program);
	api.glUseProgram(program);

	GLuint vao;
	api.glCreateVertexArrays(1, &vao);
	api.glBindVertexArray(vao);

	const GLsizeiptr kBufferSize = sizeof(PerFrameData);

	GLuint perFrameDataBuffer;
	api.glCreateBuffers(1, &perFrameDataBuffer);
	api.glNamedBufferStorage(perFrameDataBuffer, kBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
	api.glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuffer, 0, kBufferSize);

	api.glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	api.glEnable(GL_DEPTH_TEST);
	api.glEnable(GL_POLYGON_OFFSET_LINE);
	api.glPolygonOffset(-1.0f, -1.0f);

	while (!glfwWindowShouldClose(window))
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		const float ratio = width / (float)height;

		api.glViewport(0, 0, width, height);
		api.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const mat4 m = glm::rotate(glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, -3.5f)), (float)glfwGetTime(), vec3(1.0f, 1.0f, 1.0f));
		const mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);

		PerFrameData perFrameData = { .mvp = p * m, .isWireframe = false };

		api.glNamedBufferSubData(perFrameDataBuffer, 0, kBufferSize, &perFrameData);

		api.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		api.glDrawArrays(GL_TRIANGLES, 0, 36);

		perFrameData.isWireframe = true;
		api.glNamedBufferSubData(perFrameDataBuffer, 0, kBufferSize, &perFrameData);

		api.glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		api.glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	api.glDeleteBuffers(1, &perFrameDataBuffer);
	api.glDeleteProgram(program);
	api.glDeleteShader(shaderFragment);
	api.glDeleteShader(shaderVertex);
	api.glDeleteVertexArrays(1, &vao);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
