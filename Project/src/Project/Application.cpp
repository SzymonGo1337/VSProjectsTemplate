#include "dfpch.h"
#include "Application.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Project/Utils/Renderer/VertexBuffer.h"

namespace df
{
	float verts[] = {
		0.0f, 0.5f, 0.0f, 0.0f, 0.5f,
		0.5f, 0.0f, 0.0f, 0.5f, 0.0f,
		-0.5f, 0.0f, 0.0f, -0.5f, 0.0f
	};

	const char* vert = "#version 440\n"
						"layout(location = 0) in vec3 a_Pos;\n"
						"layout(location = 1) in vec2 a_TexCoord;\n"

						"out vec2 v_TexCoord;\n"

						"uniform mat4 u_ViewProjection = mat4(1.0);\n"
						"uniform mat4 u_Transform = mat4(1.0);\n"

						"void main() {\n"
							"v_TexCoord = a_Pos.xy;\n"
							"gl_Position = u_ViewProjection * u_Transform * vec4(a_Pos, 1.0);\n"
						"}";

	const char* frag =	"#version 440\n"
						"uniform vec4 u_Color = vec4(1.0, 1.0, 1.0, 1.0);\n"
						"uniform sampler2D u_Texture;\n"

						"out vec4 Color;\n"

						"in vec2 v_TexCoord;\n"


						"void main() {\n"
							"Color = vec4(v_TexCoord, 0, 1); //u_Color; //* texture(u_Texture, v_TexCoord);\n"
						"}";

	void framebufferCallback(GLFWwindow*, int width, int height) {
		glViewport(0, 0, width, height);
	}

	void OpenGLMessageCallback(uint, uint, uint, uint severity, int, const char* message, const void*) {
		//switch (severity) {
		//	case GL_DEBUG_SEVERITY_HIGH:         //RAT_LOG_CRITICAL(message);/* RAT_DEBUGBREAK();*/ return;
		//	case GL_DEBUG_SEVERITY_MEDIUM:       RAT_LOG_ERROR(message); return;
		//	case GL_DEBUG_SEVERITY_LOW:          RAT_LOG_WARN(message); return;
		//	case GL_DEBUG_SEVERITY_NOTIFICATION: RAT_LOG_TRACE(message); return;
		//}

		//RAT_ASSERT("Unknown severity level!");

		std::cout << message;
	}

	Application* Application::s_instance{nullptr};

	Application::Application() {
		s_instance = this;
	}

	int Application::Run() {
		
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		#ifdef __APPLE__
    		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		#endif

		GLFWwindow* window = glfwCreateWindow(1280, 720, "DownFallEngine", nullptr, nullptr);

		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, framebufferCallback);

		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glfwSetWindowUserPointer(window, s_instance);
		glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
			Application* app = (Application*)glfwGetWindowUserPointer(window);
			app->Shutdown();
		});

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);


		int success;
		char infoLog[512];

		int vertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertShader, 1, &vert, NULL);
		glCompileShader(vertShader);

		glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
			std::cout << "Vertex Shader Error Go Brrrr: " << infoLog << "\n";
		}

		int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &frag, NULL);
		glCompileShader(fragShader);

		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
			std::cout << "Fragment Shader Error Go Brrrr: " << infoLog << "\n";
		}

		uint shader = glCreateProgram();
		glAttachShader(shader, vertShader);
		glAttachShader(shader, fragShader);
		glLinkProgram(shader);

		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cout << "Link Shader Error Go Brrrr: " << infoLog << "\n";
		}
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		VertexBuffer vb(verts, sizeof(verts));
		vb.SetLayout({VertexAttribType::Float3, VertexAttribType::Float2});

		while (m_isRunning) {
			glfwPollEvents();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

			glUseProgram(shader);

			vb.Bind();
			glDrawArrays(GL_TRIANGLES, 0, 3);

			glfwSwapBuffers(window);
		}

		glfwDestroyWindow(window);
		glfwTerminate();
		return 0;
	}

	void Application::Pause() { m_isPaused = true; }

	void Application::Shutdown() { m_isRunning = false; }

	Application& Application::Get() {
		return *s_instance;
	}
}