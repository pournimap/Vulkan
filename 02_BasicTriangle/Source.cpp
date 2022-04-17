#include <windows.h>
#include <stdio.h> // for FILE I/O

//#include <gl\glew.h>

//#include <gl/GL.h>


//#include "vmath.h"

//#pragma comment(lib,"glew32.lib")
//#pragma comment(lib,"opengl32.lib")

//using namespace vmath;
#include "vk_engine.h"
enum
{
	MALATI_ATTRIBUTE_VERTEX = 0,
	MALATI_ATTRIBUTE_COLOR,
	MALATI_ATTRIBUTE_NORMAL,
	MALATI_ATTRIBUTE_TEXTURE
};





bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;



#include "vk_engine.h"


VulkanEngine vk_engine;



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function prototype
	
	MSG msg;
	
	bool bDone = false;

	//code
	// create log file
	/*if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log File Can Not Be Created\nExitting ..."), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File Is Successfully Opened.\n");
	}*/

	
	
	vk_engine.init(hInstance, iCmdShow);

	vk_engine.resize(800, 600);
	
	//Message Loop
	while (bDone == false) //Parallel to glutMainLoop();
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// rendring function
			fopen_s(&vk_engine.gpFile, "Log.txt", "a+");
			fprintf(vk_engine.gpFile, "after run command game loop vulkan0\n");
			fclose(vk_engine.gpFile);

			vk_engine.run();


			fopen_s(&vk_engine.gpFile, "Log.txt", "a+");
			fprintf(vk_engine.gpFile, "after run command game loop vulkan\n");
			fclose(vk_engine.gpFile);

			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyIsPressed == true) //Continuation to glutLeaveMainLoop();
				{
					bDone = true;
					fopen_s(&vk_engine.gpFile, "Log.txt", "a+");
					fprintf(vk_engine.gpFile, "after run command game loop vulkan2\n");
					fclose(vk_engine.gpFile);
				}
			}

			fopen_s(&vk_engine.gpFile, "Log.txt", "a+");
			fprintf(vk_engine.gpFile, "after run command game loop vulkan1\n");
			fclose(vk_engine.gpFile);
		}
	}

	fopen_s(&vk_engine.gpFile, "Log.txt", "a+");
	fprintf(vk_engine.gpFile, "after game loop vulkan\n");
	fclose(vk_engine.gpFile);

	//all operation in draw are asynch, it means we exit from main loop, drawing and presentation may still be going on.
	//first wait for the logical device to finish operations before exiting. and destroy the window
	vkDeviceWaitIdle(vk_engine.m_device);

	vk_engine.cleanup();
	/*if (gpFile)
	{
		fprintf(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}*/

	return((int)msg.wParam);
}

//WndProc()
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//function prototype
	
	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0) //if 0, the window is active
			gbActiveWindow = true;
		else //if non-zero, the window is not active
			gbActiveWindow = false;
		break;
	case WM_ERASEBKGND:
		return(0);
	case WM_SIZE: //Parallel to glutReshapeFunc();
		//resize(LOWORD(lParam), HIWORD(lParam)); //Parallel to glutReshapeFunc(resize);
		vk_engine.resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN: //Parallel to glutKeyboardFunc();
		switch (wParam)
		{
		case VK_ESCAPE: //case 27
			if (gbEscapeKeyIsPressed == false)
				gbEscapeKeyIsPressed = true; //Parallel to glutLeaveMainLoop();
			break;
		case 0x46: //for 'f' or 'F'
			if (vk_engine.m_bFullscreen == false)
			{
				vk_engine.ToggleFullscreen();
				vk_engine.m_bFullscreen = true;
			}
			else
			{
				vk_engine.ToggleFullscreen();
				vk_engine.m_bFullscreen = false;
			}
			break;
		default:
			break;
		}
		break;
	case WM_LBUTTONDOWN:  //Parallel to glutMouseFunc();
		break;
	case WM_CLOSE: //Parallel to glutCloseFunc();

		vkDeviceWaitIdle(vk_engine.m_device); //to avoid this : vkDestroySemaphore on VkSemaphore 0x9fde6b0000000014[] that is currently in use by a command buffer.
		vk_engine.cleanup(); //Parallel to glutCloseFunc(uninitialize);
		/*if (gpFile)
		{
			fprintf(gpFile, "Log File Is Successfully Closed.\n");
			fclose(gpFile);
			gpFile = NULL;
		}*/
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

