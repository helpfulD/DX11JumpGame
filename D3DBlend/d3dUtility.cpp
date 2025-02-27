//���������Լ�������"d3dUtility.h"ͷ�ļ�
#include "d3dUtility.h"

//D3D��ʼ��
//��������а����������֣���һ���֣�����һ�����ڣ��ڶ����֣���ʼ��D3D
//��������������
//1. HINSTANCE hInstance  ��ǰӦ�ó���ʵ���ľ��
//2. int width            ���ڿ� 
//3. int height           ���ڸ�
//4. ID3D11RenderTargetView** renderTargetView Ŀ����Ⱦ��ͼָ��
//5. ID3D11DeviceContext** immediateContext    �豸������ָ�룬�豸�����İ����豸��ʹ�û���������
//6. IDXGISwapChain** swapChain                ������ָ�룬��������������������
//7. ID3D11Device** device                     �豸��ָ�룬ÿ��D3D����������һ���豸

bool d3d::InitD3D(
	HINSTANCE hInstance,
	int width,
	int height,
	ID3D11RenderTargetView** renderTargetView,
	ID3D11DeviceContext** immediateContext,
	IDXGISwapChain** swapChain,
	ID3D11Device** device,
	ID3D11Texture2D ** depthStencilBuffer,
	ID3D11DepthStencilView ** depthStencilView)
{
	//***********��һ���֣�����һ�����ڿ�ʼ***************
	//�ⲿ�ֵĴ����ʵ��һ�еĴ������ڴ������һ�£����������ע�Ϳ��Բο�ʵ��һ
	//�������ڵ�4�����裺1 ���һ�������ࣻ2 ע�ᴰ���ࣻ3 �������ڣ�4 ������ʾ�͸��� 
	//1 ���һ��������
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"Direct3D11App";

	//2 ע�ᴰ����
	if (!RegisterClass(&wc))
	{
		::MessageBox(0, L"RegisterClass() - FAILED", 0, 0);
		return false;
	}

	//3 ��������
	HWND hwnd = 0;
	hwnd = ::CreateWindow(L"Direct3D11App",
		L"D3D11",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		0,
		0,
		hInstance,
		0);

	if (!hwnd)
	{
		::MessageBox(0, L"CreateWindow() - FAILED", 0, 0);
		return false;
	}

	//4 ������ʾ�͸���
	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);
	//***********��һ���֣�����һ�����ڽ���***************

	//***********�ڶ����֣���ʼ��D3D��ʼ***************
	//��ʼ��D3D�豸��ҪΪ���²���
	//1. �����������������DXGI_SWAP_CHAIN_DESC�ṹ
	//2. ʹ��D3D11CreateDeviceAndSwapChain����D3D�豸��ID3D11Device��
	//   �豸�����Ľӿڣ�ID3D11DeviceContext�����������ӿڣ�IDXGISwapChain��
	//3. ����Ŀ����Ⱦ��ͼ��ID3D11RenderTargetView��
	//4. �����ӿڣ�View Port��   


	//��һ���������������������DXGI_SWAP_CHAIN_DESC�ṹ
	DXGI_SWAP_CHAIN_DESC sd;                           //��������һ��DXGI_SWAP_CHAIN_DESC�Ķ���sd
	ZeroMemory(&sd, sizeof(sd));                   //��ZeroMemory��sd���г�ʼ����ZeroMemory���÷���ʵ��һ�Ĳ���֪ʶ
	sd.BufferCount = 1;                                //�������к�̨����������ͨ��Ϊ1
	sd.BufferDesc.Width = width;                       //�������еĴ��ڿ�
	sd.BufferDesc.Height = height;                     //�������еĴ��ڸ�
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //ָ��32λ���ظ�ʽ����ʾ������Alpha��8λ��������ʽ����P50
	sd.BufferDesc.RefreshRate.Numerator = 60;          //ˢ��Ƶ�ʵķ���Ϊ60
	sd.BufferDesc.RefreshRate.Denominator = 1;         //ˢ��Ƶ�ʵķ�ĸΪ1����ˢ��Ƶ��Ϊÿ��6��
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  //����������̨������÷�����CPU�Ժ�̨����ķ��� 
	sd.OutputWindow = hwnd;                            //ָ����ȾĿ�괰�ڵľ��
	sd.SampleDesc.Count = 1;                           //���ز��������ԣ������в����ö��ز�������
	sd.SampleDesc.Quality = 0;                         //����Count=1��Quality=0����ϸ����P54
	sd.Windowed = TRUE;                                //TRUEΪ����ģʽ��FALSEΪȫ��ģʽ

	//�ڶ����������豸���������Լ�����ִ��������
	//����һ������ȷ�����Դ���Featurelevel��˳��
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0, //D3D11 ��֧�ֵ�����������shader model 5
		D3D_FEATURE_LEVEL_10_1, //D3D10 ��֧�ֵ�����������shader model 4.
		D3D_FEATURE_LEVEL_10_0,
	};

	//��ȡD3D_FEATURE_LEVEL�����Ԫ�ظ���
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	//����D3D11CreateDeviceAndSwapChain�������������豸�����豸������
	//�ֱ����swapChain��device��immediateContext
	if (FAILED(D3D11CreateDeviceAndSwapChain(
		NULL,                       //ȷ����ʾ��������NULL��ʾĬ����ʾ������
		D3D_DRIVER_TYPE_HARDWARE,   //ѡ���������ͣ������ʾʹ����άӲ������
		NULL,                       //ֻ����һ����������D3D_DRIVER_TYPE_SOFTWAREʱ����ʹ���������
		0,                          //Ҳ��������ΪD3D11_CREATE_DEVICE_DEBUG��������ģʽ
		featureLevels,              //ǰ�涨���D3D_FEATURE_LEVEL����
		numFeatureLevels,           //D3D_FEATURE_LEVEL��Ԫ�ظ���
		D3D11_SDK_VERSION,          //SDK�İ汾������ΪD3D11
		&sd,                        //ǰ�涨���DXGI_SWAP_CHAIN_DESC����
		swapChain,                  //���ش����õĽ�����ָ�룬InitD3D�������ݵ�ʵ��
		device,                     //���ش����õ��豸��ָ�룬InitD3D�������ݵ�ʵ��
		NULL,                       //���ص�ǰ�豸֧�ֵ�featureLevels�����еĵ�һ������һ������ΪNULL
		immediateContext)))      //���ش����õ��豸������ָ�룬InitD3D�������ݵ�ʵ��
	{
		::MessageBox(0, L"CreateDevice - FAILED", 0, 0);  //�������ʧ�ܣ�������Ϣ��
		return false;
	}

	//��������������������ȾĿ����ͼ
	HRESULT hr = 0;         //COMҪ�����еķ������᷵��һ��HRESULT���͵Ĵ����
	ID3D11Texture2D* pBackBuffer = NULL;      //ID3D11Texture2D���͵ģ���̨����ָ��
	//����GetBuffer()�����õ���̨������󣬲�����&pBackBuffer��
	hr = (*swapChain)->GetBuffer(0,                        //����������һ������Ϊ0
		__uuidof(ID3D11Texture2D), //��������
		(LPVOID*)&pBackBuffer);   //����ָ��
//�ж�GetBuffer�Ƿ���óɹ�
	if (FAILED(hr))
	{
		::MessageBox(0, L"GetBuffer - FAILED", 0, 0); //�������ʧ�ܣ�������Ϣ��
		return false;
	}

	//����CreateRenderTargetView��������ȾĿ����ͼ�����������renderTargetView��
	hr = (*device)->CreateRenderTargetView(pBackBuffer,            //���洴���õĺ�̨����
		NULL,                   //����ΪNULL�õ�Ĭ�ϵ���ȾĿ����ͼ
		renderTargetView);     //���ش����õ���ȾĿ����ͼ��InitD3D�������ݵ�ʵ��
	pBackBuffer->Release();   //�ͷź�̨����
	//�ж�CreateRenderTargetView�Ƿ���óɹ�
	if (FAILED(hr))
	{
		::MessageBox(0, L"CreateRender - FAILED", 0, 0);  //�������ʧ�ܣ�������Ϣ��
		return false;
	}
	//-----------------------���ӵĲ���--------------------------
	D3D11_TEXTURE2D_DESC dsDesc;
	//
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.Width = 800;
	dsDesc.Height = 600;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;

	//������Ȼ���ģ��
	hr = (*device)->CreateTexture2D(&dsDesc, 0, depthStencilBuffer);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Create depth stencil buffer failed!", L"ERROR", MB_OK);
		return false;
	}
	//������Ȼ�����ͼ
	hr = (*device)->CreateDepthStencilView(*depthStencilBuffer, 0, depthStencilView);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Create depth stencil view failed!", L"ERROR", MB_OK);
		return false;
	}
	//
	(*immediateContext)->OMSetRenderTargets(1,  //�󶨵�Ŀ����ͼ����
		renderTargetView,					//��ȾĿ����ͼ
		*depthStencilView);							//�����ģ��

	//����ȾĿ����ͼ�󶨵���Ⱦ����  
	/*(*immediateContext)->OMSetRenderTargets(1,                   //�󶨵�Ŀ����ͼ�ĸ���
		renderTargetView,    //��ȾĿ����ͼ��InitD3D�������ݵ�ʵ�� 
		NULL);              //����ΪNULL��ʾ�������ģ��*/

//���Ĳ��������ӿڴ�С��D3D11Ĭ�ϲ��������ӿڣ��˲�������ֶ�����  
	D3D11_VIEWPORT vp;    //����һ���ӿڵĶ���
	vp.Width = width;     //�ӿڵĿ�
	vp.Height = height;   //�ӿڵĸ�
	vp.MinDepth = 0.0f;   //���ֵ�����ޣ�**�������ֵ��[0, 1]��������ֵ��0
	vp.MaxDepth = 1.0f;   //���ֵ�����ޣ�����ֵ��1
	vp.TopLeftX = 0;      //�ӿ����Ͻǵĺ�����
	vp.TopLeftY = 0;      //�ӿ����Ͻǵ�������

	//�����ӿ�
	(*immediateContext)->RSSetViewports(1,     //�ӿڵĸ���
		&vp); //���洴�����ӿڶ���

	return true;
	//***********�ڶ����֣���ʼ��D3D����***************
}


//��Ϣѭ������֮ǰ"Hello World"������Run()��ͬ���Ĺ���
//bool (*ptr_display)(float timeDelta)��ʾ����һ������ָ����Ϊ����
//���������һ��float���͵Ĳ�������һ��bool���͵ķ���
int d3d::EnterMsgLoop(bool(*ptr_display)(float timeDelta))
{
	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));                  //��ʼ���ڴ�

	static float lastTime = (float)timeGetTime();     //��һ�λ�ȡ��ǰʱ��

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			float currTime = (float)timeGetTime();            //�ڶ��λ�ȡ��ǰʱ��
			float timeDelta = (currTime - lastTime)*0.001f;    //��ȡ����ʱ��֮���ʱ���

			ptr_display(timeDelta);    //������ʾ���������ں���ʵ��ͼ�εı仯������ת��ʱ���õ�

			lastTime = currTime;
		}
	}
	return msg.wParam;
}