#include "windowsApplication.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	ImportVulkanFunctionPointers();

	WindowsApplication app;

	try
	{
		app.hInstance = hInstance;
		app.Start("Triangle");
	}
	catch (const char * message)
	{
		app.HandleError(message);
	}

	return 0;
}

void WindowsApplication::Start(const char * pName)
{
	LoggerCreateInfo loggerCreateInfo = {};
	loggerCreateInfo.pName = pName;
	pLogger = ConsoleLogger::Create(&loggerCreateInfo);

	Win32CreateInfo createInfo = {};
	createInfo.pLogger = pLogger;
	createInfo.pName = pName;
	createInfo.fullscreen = false;
	createInfo.width = 1028;
	createInfo.height = 720;
	createInfo.hInstance = hInstance;
	createInfo.windowResized = [=](unsigned int width, unsigned int height)
	{
		pLogger->Info("Window resized callback!");
	};
	createInfo.windowActivation = [=](bool active)
	{
		if (active)
		{
			pLogger->Info("Window activated");
		}
		else
		{
			pLogger->Info("Window deactivated");
		}
	};
	pWindow = Win32Window::Create(&createInfo);
	
	try
	{
		DeviceManagerCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.enableValidation = true;
		deviceCreateInfo.pLogger = pLogger;
		deviceCreateInfo.surfaceExtension = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		pDevice = DeviceManager::Create(&deviceCreateInfo);

		Win32SurfaceCreateInfo surfaceCreateInfo = {};
		surfaceCreateInfo.hInstance = hInstance;
		surfaceCreateInfo.hwnd = (HWND)pWindow->GetHandle();
		surfaceCreateInfo.pDevice = pDevice;
		surfaceCreateInfo.pLogger = pLogger;
		pSurface = Win32Surface::Create(&surfaceCreateInfo);

		SimpleRendererCreateInfo rendererCreateInfo = {};
		rendererCreateInfo.pLogger = pLogger;
		rendererCreateInfo.pDevice = pDevice;
		rendererCreateInfo.pSurface = pSurface;
		pRenderer = SimpleRenderer::Create(&rendererCreateInfo);
	}
	catch (const char * message)
	{
		HandleError(message);
	}
	
	pInputsManager = new Win32InputManager;

	std::thread updateThread([&] {
		SceneCreateInfo sceneCreateInfo = {};
		sceneCreateInfo.pInputManager = pInputsManager;
		sceneCreateInfo.pWindow = pWindow;
		sceneManager.AddScene(&sceneCreateInfo);

		while (!pWindow->shutdown)
		{
			sceneManager.Update();
		}
	});

	pWindow->StartProcessingMessages();
	updateThread.join();
}

void WindowsApplication::HandleError(const char * pMessage)
{
	pLogger->Error(pMessage);
}