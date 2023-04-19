#ifndef _WIN_APP_H_
#define _WIN_APP_H_

#include <Windows.h>
#include <string>
#include <D3D11.h>

#include "Timer.h"

class WinApp
{
public:
	WinApp(HINSTANCE hInst, std::wstring title = L"SIA Framework", int width = 800, int height = 600);
	virtual ~WinApp();

	//Basic inline membre functions
	HINSTANCE	AppInstance()	const				{ return m_hInstance;		}
	HWND		Window()		const				{ return m_hWnd;			}
	int			Width()			const				{ return m_clientWidth;		}
	int			Height()		const				{ return m_clientHeight;	}
	void		SetWindowTitle(std::wstring title)	{ SetWindowText(m_hWnd,title.c_str()); }

	/*
	  Functions that can be redefined by each sub-class
	  For Init() or OnResize(), it is important that subclasses should first call WinApp::Init() or WinApp::Resize().
	*/
	virtual bool	Init();							//Initialization
	virtual bool	OnResize();						//Window size changed
	virtual bool	Update(float timeDelt) = 0;		//Update for each frame
	virtual bool	Render() = 0;					//Scene rendering for each frame

	virtual LRESULT CALLBACK WinProc(HWND,UINT,WPARAM,LPARAM);		//Main messaeg processing function
	
	int		Run();		//Main game loop

	//Mouse control function
	//By default, the three functions do nothing. And can be redefined.
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:
	bool	InitWindow();		//Win32 window initialization
	bool	InitD3D();			//D3D11 initialization
	
	void	CalculateFPS();		//Frame rate update

protected:
	HINSTANCE	m_hInstance;		//Application instance
	HWND		m_hWnd;				//Window instance

	int			m_clientWidth;		//Client window size
	int			m_clientHeight;

	bool		m_isMinimized;		//Some application state parameters
	bool		m_isMaximized;
	bool		m_isPaused;	
	bool		m_isResizing;

	ID3D11Device			*m_d3dDevice;				//Basic D3D11 related parameters
	ID3D11DeviceContext		*m_deviceContext;
	IDXGISwapChain			*m_swapChain;
	ID3D11Texture2D			*m_depthStencilBuffer;
	ID3D11RenderTargetView	*m_renderTargetView;
	ID3D11DepthStencilView	*m_depthStencilView;
	D3D11_VIEWPORT			m_viewport;
	
	std::wstring	m_winTitle;			//Title of the application
	Timer			m_timer;			//Timer

private:
	//Forbid copying
	WinApp(const WinApp&);
	WinApp& operator = (const WinApp&);
};

#endif	//_WIN_APP_H_