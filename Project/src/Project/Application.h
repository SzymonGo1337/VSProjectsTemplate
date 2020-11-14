#pragma once
#include "Project/Base.h"

namespace df
{
	class Application
	{
	public:
		Application();

		int Run();

		void Pause();
		void Shutdown();

		static Application& Get();
	private:
		static Application* s_instance;

		bool m_isRunning = true;
		bool m_isPaused = false;
	};
}