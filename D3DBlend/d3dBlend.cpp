#include "d3dUtility.h"
#include "Light.h"
#include "Gravity.h"
#include "Camera.h"
#include <math.h>

//声明全局的指针
ID3D11Device* device = NULL;//D3D11设备接口
IDXGISwapChain* swapChain = NULL;//交换链接口
ID3D11DeviceContext* immediateContext = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;//渲染目标视图  

//Effect相关全局指针
ID3D11InputLayout* vertexLayout;
ID3DX11Effect* effect;
ID3DX11EffectTechnique* technique;

//声明三个坐标系矩阵
XMMATRIX world;         //用于世界变换的矩阵
XMMATRIX view;          //用于观察变换的矩阵
XMMATRIX projection;    //用于投影变换的矩阵

ID3D11DepthStencilView * depthStencilView;
ID3D11Texture2D * depthStencilBuffer;

//声明材质和光照的全局对象
Material		boxMaterial;      //箱子材质
Material		floorMaterial;    //地板材质
Material		waterMaterial;    //水面材质
Light			light[3];      //光源数组
int             lightType = 0;  //光源类型

ID3D11ShaderResourceView* textureBox;      //箱子纹理
ID3D11ShaderResourceView* textureFloor;    //地板纹理
ID3D11ShaderResourceView* texturebrick;    //砖面纹理
ID3D11ShaderResourceView* textureWater;    //水面纹理
ID3D11ShaderResourceView* textureWall;    //墙面纹理
ID3D11ShaderResourceView* textureSky;    //天空纹理
ID3D11ShaderResourceView* textureIron;    //铁棍纹理
ID3D11ShaderResourceView* textureStar;    //星空纹理
ID3D11ShaderResourceView* textureTanhuang;    //弹簧纹理
ID3D11ShaderResourceView* textureTanhuangding;    //弹簧顶纹理
ID3D11ShaderResourceView* textureBazi;    //最终板纹理 
ID3D11ShaderResourceView* textureGameover;    //结束界面纹理 


ID3D11BlendState *blendStateAlpha;        //混合状态
ID3D11RasterizerState* noCullRS;          //背面消隐状态


// 设置光源
Light dirLight, pointLight, spotLight;

Camera* camera = new Camera();    //摄像机对象
XMVECTOR Eye;                     //视点位置

void SetLightEffect(Light light);

//定义一个顶点结构，这个顶点包含坐标和法向量和纹理坐标
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

//**************以下为框架函数******************
bool Setup()
{
	//这里主要包含5个主要步骤
	//第一步载入外部文件（包括fx文件及图像文件）
	//第二步创建各种渲染状态
	//第三步创建输入布局
	//第四步创建顶点缓存
	//第五步设置材质和光照
	//*************第一步载入外部文件（包括fx文件及图像文件）****************************
	HRESULT hr = S_OK;              //声明HRESULT的对象用于记录函数调用是否成功
	ID3DBlob* pTechBlob = NULL;     //声明ID3DBlob的对象用于存放从文件读取的信息
	//从我们之前建立的.fx文件读取着色器相关信息
	hr = D3DX11CompileFromFile(L"Shader.fx", NULL, NULL, NULL, "fx_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pTechBlob, NULL, NULL);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"fx文件载入失败", L"Error", MB_OK); //如果读取失败，弹出错误信息
		return hr;
	}
	//调用D3DX11CreateEffectFromMemory创建ID3DEffect对象
	hr = D3DX11CreateEffectFromMemory(pTechBlob->GetBufferPointer(),
		pTechBlob->GetBufferSize(), 0, device, &effect);

	if (FAILED(hr))
	{
		::MessageBox(NULL, L"创建Effect失败", L"Error", MB_OK);  //创建失败，弹出错误信息
		return hr;
	}
	//从外部图像文件载入纹理
	//箱子纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"BOX.BMP", NULL, NULL, &textureBox, NULL);
	//地面的纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"land.BMP", NULL, NULL, &textureFloor, NULL);
	//水面纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"water.dds", NULL, NULL, &textureWater, NULL);
	//砖块纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"brick.dds", NULL, NULL, &texturebrick, NULL);
	//墙面纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"wall.BMP", NULL, NULL, &textureWall, NULL);
	//天空纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"sky.BMP", NULL, NULL, &textureSky, NULL);
	//铁棍纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"iron.BMP", NULL, NULL, &textureIron, NULL);
	//星空纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"star.BMP", NULL, NULL, &textureStar, NULL);
	//弹簧纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"tanhuang.BMP", NULL, NULL, &textureTanhuang, NULL); 
	//弹簧顶纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"tanhuangding.BMP", NULL, NULL, &textureTanhuangding, NULL); 
	//最终板纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"bazi.BMP", NULL, NULL, &textureBazi, NULL); 
	//结束界面纹理
	D3DX11CreateShaderResourceViewFromFile(device, L"gameover.BMP", NULL, NULL, &textureGameover, NULL); 
	//*************第一步载入外部文件（包括fx文件及图像文件）****************************

	//*************第二步创建各种渲染状态************************************************
	//先创建一个混合状态的描述
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc)); //清零操作
	blendDesc.AlphaToCoverageEnable = false;     //关闭AlphaToCoverage多重采样技术
	blendDesc.IndependentBlendEnable = false;    //不针对多个RenderTarget使用不同的混合状态
	//只针对RenderTarget[0]设置绘制混合状态，忽略1-7
	blendDesc.RenderTarget[0].BlendEnable = true;                   //开启混合
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;     //设置源因子
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//设置目标因子
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;         //混合操作
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;      //源混合百分比因子
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;    //目标混合百分比因子
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;    //混合百分比的操作
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;  //写掩码
	//创建ID3D11BlendState接口
	device->CreateBlendState(&blendDesc, &blendStateAlpha);

	//关闭背面消隐
	D3D11_RASTERIZER_DESC ncDesc;
	ZeroMemory(&ncDesc, sizeof(ncDesc));  //清零操作
	ncDesc.CullMode = D3D11_CULL_NONE;   //剔除特定朝向的三角形，这里不剔除，即全部绘制
	ncDesc.FillMode = D3D11_FILL_SOLID;  //填充模式，这里为利用三角形填充
	ncDesc.FrontCounterClockwise = false;//是否设置逆时针绕续的三角形为正面
	ncDesc.DepthClipEnable = true;       //开启深度裁剪
	//创建一个关闭背面消隐的状态，在需要用的时候才设置给设备上下文
	if (FAILED(device->CreateRasterizerState(&ncDesc, &noCullRS)))
	{
		MessageBox(NULL, L"Create 'NoCull' rasterizer state failed!", L"Error", MB_OK);
		return false;
	}
	//*************第二步创建各种渲染状态************************************************

	//*************第三步创建输入布局****************************************************
	//用GetTechniqueByName获取ID3DX11EffectTechnique的对象
	//先设置默认的technique到Effect
	technique = effect->GetTechniqueByName("TexTech");                //默认Technique

	//D3DX11_PASS_DESC结构用于描述一个Effect Pass
	D3DX11_PASS_DESC PassDesc;
	//利用GetPassByIndex获取Effect Pass
	//再利用GetDesc获取Effect Pass的描述，并存如PassDesc对象中
	technique->GetPassByIndex(0)->GetDesc(&PassDesc);

	//创建并设置输入布局
	//这里我们定义一个D3D11_INPUT_ELEMENT_DESC数组，
	//由于我们定义的顶点结构包括位置坐标和法向量，所以这个数组里有两个元素
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	//layout元素个数
	UINT numElements = ARRAYSIZE(layout);
	//调用CreateInputLayout创建输入布局
	hr = device->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertexLayout);
	//设置生成的输入布局到设备上下文中
	immediateContext->IASetInputLayout(vertexLayout);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"创建Input Layout失败", L"Error", MB_OK);
		return hr;
	}
	//*************第三步创建输入布局****************************************************

	//*************第四步创建顶点缓存****************************************************
	//这里需要定义箱子，池子，以及水面的顶点
	Vertex vertices[] =
	{
		//------------------------------------箱子的顶点------------------------------------
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		//地板
		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 4.0f) },

		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 4.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 4.0f) },

		//前面墙
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		//后面墙
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		//左侧面墙
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		//右侧面墙
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
			
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },

		//天空
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		//第一级台阶
		{ XMFLOAT3(4.0f, 1.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(4.0f, 1.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(4.0f, -1.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(4.0f, -1.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(4.0f, 1.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(4.0f, -1.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 1.0f) },

		{ XMFLOAT3(6.0f, 1.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(6.0f, 1.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(4.0f, 1.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(4.0f, 1.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(6.0f, 1.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(4.0f, 1.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(3.0f, 1.0f) },

		//第二级台阶
		{ XMFLOAT3(6.0f, 3.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(6.0f, 3.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(6.0f, 1.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(6.0f, 1.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(6.0f, 3.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(6.0f, 1.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 3.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 3.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(6.0f, 3.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(6.0f, 3.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(8.0f, 3.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(6.0f, 3.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(3.0f, 1.0f) },

		//第三级台阶
		{ XMFLOAT3(8.0f, 5.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 5.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 3.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 3.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(8.0f, 5.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 3.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 1.0f) },

		{ XMFLOAT3(10.0f, 5.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 5.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 5.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(8.0f, 5.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(10.0f, 5.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },
		{ XMFLOAT3(8.0f, 5.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(3.0f, 1.0f) },

		//台阶前的墙
		{ XMFLOAT3(4.0f, 1.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(3.0f, 1.0f) },
		{ XMFLOAT3(4.0f, -1.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(10.0f, -1.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(3.0f, 1.0f) },
		{ XMFLOAT3(4.0f, 1.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(3.0f, 0.0f) },

		{ XMFLOAT3(6.0f, 3.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 1.0f) },
		{ XMFLOAT3(6.0f, 1.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(10.0f, 1.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 1.0f) },
		{ XMFLOAT3(6.0f, 3.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 3.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(8.0f, 5.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 3.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(8.0f, 3.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(10.0f, 3.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(8.0f, 5.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 5.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },

		//台阶后的墙
		{ XMFLOAT3(4.0f, 1.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(3.0f, 1.0f) },
		{ XMFLOAT3(4.0f, -1.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(10.0f, -1.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(3.0f, 1.0f) },
		{ XMFLOAT3(4.0f, 1.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(3.0f, 0.0f) },

		{ XMFLOAT3(6.0f, 3.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 1.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 1.0f) },
		{ XMFLOAT3(6.0f, 1.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(10.0f, 1.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 1.0f) },
		{ XMFLOAT3(6.0f, 3.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 3.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(8.0f, 5.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 3.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(8.0f, 3.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(10.0f, 3.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(8.0f, 5.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 5.0f, 3.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		//台阶的背面
		{ XMFLOAT3(10.0f, 5.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 3.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 3.0f) },

		{ XMFLOAT3(10.0f, -1.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 3.0f) },
		{ XMFLOAT3(10.0f, 5.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 5.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },

		//第一个跳板
			//上面
		{ XMFLOAT3(20.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -4.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(4.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -4.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -4.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 5.0f) },
			//底面
		{ XMFLOAT3(20.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -4.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(4.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -4.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -4.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 5.0f) },
			//前面
		{ XMFLOAT3(14.0f, 7.0f, 4.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -4.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(16.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 7.0f, -4.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(16.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -4.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//背面
		{ XMFLOAT3(20.0f, 7.0f, 4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, 4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(16.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 6.0f, 4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(16.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//左侧面
		{ XMFLOAT3(14.0f, 7.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(6.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },
			//右侧面
		{ XMFLOAT3(14.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(6.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },


		//第二个跳板
			//上面
		{ XMFLOAT3(20.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -16.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(4.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -16.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -16.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 5.0f) },
			//底面
		{ XMFLOAT3(20.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -16.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(4.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -16.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -16.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 5.0f) },
			//前面
		{ XMFLOAT3(14.0f, 7.0f, -8.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -16.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(16.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 7.0f, -16.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(16.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -16.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//背面
		{ XMFLOAT3(20.0f, 7.0f, -8.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -8.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -16.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(16.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -16.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -8.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(16.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -16.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//右侧面
		{ XMFLOAT3(14.0f, 7.0f, -16.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -16.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -16.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -16.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -16.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(6.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -16.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },
			//左侧面
		{ XMFLOAT3(14.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -8.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(6.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -8.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },


		//第二块板链接独木桥
			//上面
		{ XMFLOAT3(17.5f,7.0f,-19.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,7.0f,-19.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(32.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-19.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(32.0f, 0.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//底面
		{ XMFLOAT3(17.5f,6.5f,-19.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,6.5f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(32.0f, 0.0f) },

		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(32.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//前面
		{ XMFLOAT3(17.0f,7.0f,-19.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(32.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(32.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-35.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//背面
		{ XMFLOAT3(17.5f,7.0f,-19.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-19.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(32.0f, 0.0f) },

		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,6.5f,-19.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(32.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//右侧面
		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,6.5f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },
			//左侧面
		{ XMFLOAT3(17.0f,7.0f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,7.0f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(17.5f,7.0f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },


			//再接斜独木桥
				//上面
		{ XMFLOAT3(17.0f,7.0f,-34.5f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.32f,17.0f,-34.5f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 34.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-0.32f,17.0f,-34.5f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.32f,17.0f,-35.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 34.0f) },
			//前面
		{ XMFLOAT3(-0.32f,17.0f,-34.5f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.32f,17.0f,-35.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-0.32f,17.0f,-35.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-35.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//底面
		{ XMFLOAT3(16.75f,6.57f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 34.0f) },
		{ XMFLOAT3(16.75f,6.57f,-34.5f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-34.5f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 34.0f) },
			//背面
		{ XMFLOAT3(17.0f,7.0f,-34.5f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(16.75f,6.57f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-34.5f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//左侧面
		{ XMFLOAT3(-0.32f,17.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(40.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(40.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-34.5f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },
			//右侧面
		{ XMFLOAT3(-0.32f,17.0f,-34.5f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,7.0f,-34.5f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(40.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-34.5f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(40.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-35.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },

		//小跳板
			//上面
		{ XMFLOAT3(-3.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-32.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-5.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,17.0f,-32.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-32.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//底面
		{ XMFLOAT3(-3.0f,16.0f,-30.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-32.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-32.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-32.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//前面
		{ XMFLOAT3(-5.0f,17.0f,-30.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-5.0f,17.0f,-32.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-5.0f,17.0f,-32.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-32.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//背面
		{ XMFLOAT3(-3.0f,17.0f,-30.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-30.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-32.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-3.0f,17.0f,-32.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-30.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-32.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//左侧面
		{ XMFLOAT3(-5.0f,17.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-3.0f,17.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },
			//右侧面
		{ XMFLOAT3(-5.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-3.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },


			//最终结束板 
			//上面 
		{ XMFLOAT3(-10.0f,20.0f,5.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,20.0f,5.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,20.0f,-5.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(0.0f,20.0f,5.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.0f,20.0f,-5.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,20.0f,-5.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//底面
		{ XMFLOAT3(-10.0f,19.0f,5.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,19.0f,-5.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,-5.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,-5.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//前面
		{ XMFLOAT3(0.0f,20.0f,5.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.0f,20.0f,-5.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(10.0f, 0.0f) },

		{ XMFLOAT3(0.0f,20.0f,-5.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,-5.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//背面
		{ XMFLOAT3(-10.0f,20.0f,5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,20.0f,-5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(10.0f, 0.0f) },

		{ XMFLOAT3(-10.0f,20.0f,-5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,19.0f,5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,-5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//左侧面
		{ XMFLOAT3(0.0f,20.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,20.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(10.0f, 0.0f) },

		{ XMFLOAT3(-10.0f,20.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.0f,19.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },
			//右侧面
		{ XMFLOAT3(0.0f,20.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,20.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(10.0f, 0.0f) },

		{ XMFLOAT3(-10.0f,20.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },

			//星空
			//前面墙
		{ XMFLOAT3(-50.0f, 120.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 120.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.625f) },

		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0, 0.625f) },
		{ XMFLOAT3(50.0f, 120.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.625f) },

			//后面墙
		{ XMFLOAT3(-50.0f, 120.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 120.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.625f) },
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0, 0.0f) },

		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0, 0.0f) },
		{ XMFLOAT3(50.0f, 120.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.625f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0, 0.625f) },

			//左侧面墙
		{ XMFLOAT3(-50.0f, 120.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.625f) },
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.0f) },

		{ XMFLOAT3(-50.0f, 120.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 120.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.625f) },
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.625f) },

			//右侧面墙
		{ XMFLOAT3(50.0f, 120.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.625f) },

		{ XMFLOAT3(50.0f, 120.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.625f) },
		{ XMFLOAT3(50.0f, 120.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.625f) },

			//天空
		{ XMFLOAT3(-50.0f, 120.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 120.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 120.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-50.0f, 120.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 120.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 120.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

			//gameover
		{ XMFLOAT3(0.0f,30.0f,7.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.0f,20.0f,7.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,30.0f,7.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(-10.0f,30.0f,7.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,20.0f,7.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,20.0f,7.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 1.0f) },

		/*/------------------------------------水面的顶点------------------------------------------
		{ XMFLOAT3(-10.0f, 0.8f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 0.8f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, 0.8f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },

		{ XMFLOAT3(-10.0f, 0.8f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(10.0f, 0.8f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 0.8f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 10.0f) },*/
	};
	UINT vertexCount = ARRAYSIZE(vertices);
	//创建顶点缓存，方法同实验4一样
	//首先声明一个D3D11_BUFFER_DESC的对象bd
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * vertexCount;	//注意：由于这里定义了24个顶点所以要乘以24
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;		//注意：这里表示创建的是顶点缓存
	bd.CPUAccessFlags = 0;

	//声明一个D3D11_SUBRESOURCE_DATA数据用于初始化子资源
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;         //设置需要初始化的数据，这里的数据就是顶点数组
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	//声明一个ID3D11Buffer对象作为顶点缓存
	ID3D11Buffer* vertexBuffer;
	//调用CreateBuffer创建顶点缓存
	hr = device->CreateBuffer(&bd, &InitData, &vertexBuffer);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"创建VertexBuffer失败", L"Error", MB_OK);
		return hr;
	}

	UINT stride = sizeof(Vertex);                 //获取Vertex的大小作为跨度
	UINT offset = 0;                              //设置偏移量为0
	//设置顶点缓存，参数的解释见实验4
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//指定图元类型，D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST表示图元为三角形
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//*************第四步创建顶点缓存****************************************************

	//*************第五步设置材质和光照**************************************************
	//池子地板及墙的材质
	floorMaterial.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	floorMaterial.diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	floorMaterial.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	floorMaterial.power = 5.0f;
	//箱子材质
	boxMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	boxMaterial.diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	boxMaterial.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	boxMaterial.power = 5.0f;
	/*/水面材质
	waterMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	waterMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.6f);//水面的alpha值为0.6，即其透明度为40%
	waterMaterial.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
	waterMaterial.power = 5.0f;*/

	// 方向光只需要设置：方向、3种光照强度
	dirLight.type = 0;
	dirLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f);
	dirLight.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);   //前三位分别表示红绿蓝光的强度
	dirLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //同上
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //同上

	// 点光源需要设置：位置、3中光照强度、3个衰减因子
	pointLight.type = 1;
	pointLight.position = XMFLOAT4(0.0f, 5.0f, 0.0f, 1.0f); //光源位置
	pointLight.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);   //前三位分别表示红绿蓝光的强度
	pointLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //同上
	pointLight.specular = XMFLOAT4(1.0f, 1.0f,1.0f, 1.0f);  //同上
	pointLight.attenuation0 = 0;      //常量衰减因子
	pointLight.attenuation1 = 0.1f;   //一次衰减因子
	pointLight.attenuation2 = 0;      //二次衰减因子

	// 聚光灯需要设置Light结构中所有的成员
	spotLight.type = 2;
	spotLight.position = XMFLOAT4(0.0f, 10.0f, 0.0f, 1.0f); //光源位置
	spotLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f); //光照方向
	spotLight.ambient = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);   //前三位分别表示红绿蓝光的强度
	spotLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //同上
	spotLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //同上
	spotLight.attenuation0 = 0;    //常量衰减因子
	spotLight.attenuation1 = 0.1f; //一次衰减因子
	spotLight.attenuation2 = 0;    //二次衰减因子
	spotLight.alpha = XM_1DIV2PI;   //内锥角度
	spotLight.beta = XM_PI;    //外锥角度
	spotLight.fallOff = 0.5f;      //衰减系数，一般为1.0

	light[0] = dirLight;
	light[1] = pointLight;
	light[2] = spotLight;
	//*************第五步设置材质和光照**************************************************
	

	return true;
}

void Cleanup()
{
	//释放全局指针
	if (renderTargetView) renderTargetView->Release();
	if (immediateContext) immediateContext->Release();
	if (swapChain) swapChain->Release();
	if (device) device->Release();

	if (vertexLayout) vertexLayout->Release();
	if (effect) effect->Release();

	if (depthStencilView) depthStencilView->Release();
	if (textureBox) textureBox->Release();
	if (textureFloor) textureFloor->Release();
	if (textureWater) textureWater->Release();

	if (blendStateAlpha) blendStateAlpha->Release();
	if (noCullRS) noCullRS->Release();

}

void Gravity(float timeDelta)
{

	transY += timeDelta * Speed + 0.5*gravity*timeDelta*timeDelta;
	Speed += gravity * timeDelta;

}

bool Display(float timeDelta)
{
	if (device)
	{
		//********************第一部分：设置3个坐标系及光照的外部变量****************************
		//-------------------------------定义变量-------------------------------
		static float yNow = 0.0f;		//当前y坐标
		static bool onTheGround = true;	//判断是否在物体或者地面上
		static float angleX = 0.0f;		//当前绕Y轴旋转角度	
		static float angleY = 0.0f;		//当前绕XZ旋转角度	
		static float keytime = 0.0f;	//控制松开后跳跃  并且记录按键时间
		bool spaceUp = true;			//控制松开空格后跳跃
		bool musicKey = false;			//音乐控制
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };//声明一个数组存放颜色信息，4个元素分别表示红，绿，蓝以及alpha
		immediateContext->ClearRenderTargetView(renderTargetView, ClearColor);		//清除当前绑定的深度模板视图
		immediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		float BlendFactor[] = { 0,0,0,0 };		//先指定混合因子，一般不用它，除非在上面混合因子指定为使用blend factor

		pointLight.position = XMFLOAT4(transX, transY+3.0f, transZ, 1.0f); //光源位置
		spotLight.position = XMFLOAT4(transX, transY+3.0f, transZ, 1.0f); //光源位置
		light[1] = pointLight;
		light[2] = spotLight;

		//-------------------------------通过鼠标转动摄像机-------------------------------
		Eye = XMVectorSet(transX, transY, transZ, 0.0f);//相机位置
		camera->SetEye(Eye);   //设置视点位置
		POINT pt;
		GetCursorPos(&pt);
		POINT oldpt;
		int dx = 0;
		int dy = 0;
		oldpt.x = 400;
		oldpt.y = 300;
		dx = pt.x - oldpt.x;
		dy = pt.y - oldpt.y;
		angleX += dx  * timeDelta;
		angleY += dy * timeDelta;
		//使俯仰角保持在+-45°
		camera->Yaw(dx  * timeDelta);
		if (angleY< XM_PIDIV4&&angleY > -XM_PIDIV4) {
			camera->Pitch(dy * timeDelta);
		}
		else {
			angleY = 0.0f; 
		} 
		SetCursorPos(400,300);				//设置鼠标位置不变 
		ShowCursor(false);					//将鼠标设为不可见 
		//使旋转角保持在0~2π
		if (angleX > XM_2PI) { 
			angleX -= XM_2PI; 
		}
		if (angleX < 0) { 
			angleX += XM_2PI;  
		} 
		  


		
		camera->Apply();//生成观察矩阵
		world = XMMatrixIdentity();//初始化世界矩阵
		projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 800.0f / 600.0f, 0.01f, 100.0f);//设置投影矩阵
		//将坐标变换矩阵的常量缓存中的矩阵和坐标设置到Effect框架中
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
		XMMATRIX ViewMATRIX = XMLoadFloat4x4(&camera->GetView());
		effect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)&ViewMATRIX);    //设置观察坐标系
		effect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)&projection); //设置投影坐标系
		effect->GetVariableByName("EyePosition")->AsMatrix()->SetMatrix((float*)&Eye); //设置视点
		//光源的常量缓存中的光源信息设置到Effect框架中
		SetLightEffect(light[lightType]);
		//********************第一部分：设置3个坐标系及光照的外部变量****************************


		//*****************************第二部分：实现各个功能************************************
		//-------------------------------键盘控制摄像头移动-------------------------------
		if (::GetAsyncKeyState('A') & 0x8000f) {
			if ((angleX >= -XM_PIDIV2&&angleX <= 0)) {
				transX -= sin(XM_PIDIV2 + angleX)*5.0f* timeDelta;
				transZ -= cos(XM_PIDIV2 + angleX)*5.0f* timeDelta;
			}
			else{
				transZ += sin(angleX)*5.0f* timeDelta;
				transX -= cos(angleX)*5.0f* timeDelta;
			}
		}
		if (::GetAsyncKeyState('D') & 0x8000f) {
			if ((angleX >= -XM_PIDIV2&&angleX <= 0)) {
				transX += sin(XM_PIDIV2 + angleX)*5.0f* timeDelta;
				transZ += cos(XM_PIDIV2 + angleX)*5.0f* timeDelta;
			}
			else {
				transX += cos(angleX)*5.0f* timeDelta;
				transZ -= sin(angleX)*5.0f* timeDelta;
			}
		}
		if (::GetAsyncKeyState('W') & 0x8000f) {
			transZ += cos(angleX)*5.0f* timeDelta;
			transX += sin(angleX)*5.0f* timeDelta;
		}
		if (::GetAsyncKeyState('S') & 0x8000f) {
			transZ -= cos(angleX)*5.0f* timeDelta;
			transX -= sin(angleX)*5.0f* timeDelta;
		}
		if (::GetAsyncKeyState('R') & 0x8000f) {
			transZ = 0;
			transX = 0;
			transY = 0;
			lightType = 0;
		}

		//-------------------------------跳跃-------------------------------
		if (::GetAsyncKeyState(VK_SPACE) & 0x8000f && onTheGround){ //按下空格
			keytime += 0.1f*timeDelta;
			spaceUp = false;
		}
		if (keytime != 0 && spaceUp) {							   //松开空格后跳跃
			keytime = 0.0f;
			Speed = 9.0f;
			musicKey = true;
			flag = true;
			onTheGround = false;
		}

		//-------------------------------控制yNow,使箱子离开当前平台时可以落下-------------------------------
		if (!(transX > -10.0f&&transX<-0.0f&&transZ > -5.0f&&transZ < 5.0f) && yNow >= 18.0f) {//最终板
			yNow = 18.0f;
			flag = true;
		}
		if (!(transX > -5.0f&&transX<-3.0f&&transZ > -32.0f&&transZ < -30.0f) && yNow >= 16.0f) {//小跳板
			yNow = 16.0f;
			flag = true;
		}
		if (!(transX > -0.68f&&transX<17.0f&&transZ > -35.5f&&transZ < -34.0f) && yNow >= 8.0f + (17.32f - transX) * 0.57735f) {//第二个木板
			yNow = 8.0f + (16.32f - transX);
			flag = true;
		}
		if (!(transX > 17.0f&&transX<17.5f&&transZ > -36.0f&&transZ < -18.0f) && yNow > 8.0f) {//独木桥
			yNow = 6.0f;
			flag = true;
		}
		if (!(transX > 13.0f&&transX<21.0f&&transZ > -17.0f&&transZ < -7.0f) && yNow > 8.0f) {//第二个木板
			yNow = 6.0f;
			flag = true;
		}
		if (!(transX > 13.0f&&transX<21.0f&&transZ > -5.0f&&transZ < 5.0f) && yNow > 8.0f) {//第一个木板
			yNow = 6.0f;
			flag = true;
		}
		if (!(transX > 7.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f) && yNow >= 6.0f) {//第三级台阶
			yNow = 4.0f;
			flag = true;
		}
		if (!(transX > 5.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f) && yNow >= 4.0f) {//第二级台阶
			yNow = 2.0f;
			flag = true;
		}
		if (!(transX > 3.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f) && yNow >= 2.0f) {//第一级台阶
			yNow = 0.0f; 
			flag = true;
		}


		if (transX > 13.0f&&transX<21.0f&&transZ > -5.0f&&transZ < 5.0f&&transY >= 8.0f) {//控制在第一个木板上时
			yNow = 8.0f;
		}
		if (transX > 13.0f&&transX<21.0f&&transZ > -17.0f&&transZ < -7.0f&&transY >= 8.0f) {//控制在第二个木板上时
			yNow = 8.0f;
		}
		if (transX > 17.0f&&transX<17.5f&&transZ > -36.0f&&transZ < -18.0f&&transY >= 8.0f) {//控制在独木桥上时
			yNow = 8.0f;
		}
		if (transX > -0.68f&&transX<17.0f&&transZ > -35.5f&&transZ < -34.0f&&transY >= 8.0f) {//控制在斜独木桥上时
			yNow = 8.0f + (17.32f - transX) * 0.57735f;
		}
		if (transX > -5.0f&&transX<-3.0f&&transZ > -32.0f&&transZ < -30.0f&&transY >= 16.0f) {//小跳板
			yNow = 18.0f;
			Speed = 40.0f;
			onTheGround = false;
			flag = true;
		}
		if (transX > -10.0f&&transX<-0.0f&&transZ > -5.0f&&transZ < 5.0f&&transY >= 21.0f) {//最终板
			yNow = 21.0f;
		}
		if (transY > 40.0f) {
			lightType = 0;
		}

		//-------------------------------下落-------------------------------
		if (flag) {


			if (musicKey) {
				PlaySound(L"jump.wav", NULL, SND_FILENAME | SND_ASYNC);
				musicKey = false;
			}
			Gravity(timeDelta);
			//若下降至小于yNow,则停止下降
			if (transY < yNow){
				transY = yNow;
				flag = false;
				onTheGround = true;
			}
			//落到障碍物上
			if (transX > 3.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f&&transY >= 2.0f) {//第一级台阶
				yNow = 2.0f;
			}
			if (transX > 5.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f&&transY >= 4.0f) {//第二级台阶
				yNow = 4.0f;
			}
			if (transX > 7.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f&&transY >= 6.0f) {//第三级台阶
				yNow = 6.0f;
			}
			if (transX > 13.0f&&transX<21.0f&&transZ > -5.0f&&transZ < 5.0f&&transY >= 8.0f) {//第一个木板
				yNow = 8.0f;
			}
			if (transX > 13.0f&&transX<21.0f&&transZ > -17.0f&&transZ < -7.0f&&transY >= 8.0f) {//第二个木板
				yNow = 8.0f;
			}
			if (transX > 17.0f&&transX<17.5f&&transZ > -36.0f&&transZ < -18.0f&&transY >= 8.0f) {//独木桥
				yNow = 8.0f;
			}
			if (transX > -0.32f&&transX<17.0f&&transZ > -35.5f&&transZ < -34.0f&&transY >= 8.0f) {//独木桥
				yNow = 8.0f + (17.32f - transX) * 0.57735f;
			}
			if (transX > -5.0f&&transX<-3.0f&&transZ > -32.0f&&transZ < -30.0f&&transY >= 16.0f) {//小跳板
				yNow = 18.0f;
			}
			if (transX > -10.0f&&transX<-0.0f&&transZ > -5.0f&&transZ < 5.0f&&transY >= 21.0f) {//最终板
				yNow = 21.0f;
				//绘制胜利面板
				lightType = 0;
				world = XMMatrixIdentity();
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
				immediateContext->RSSetState(noCullRS);                 //关闭背面消隐，使镜头在障碍物背面也能看到障碍物
				effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureGameover);
				technique->GetPassByIndex(0)->Apply(0, immediateContext);
				immediateContext->Draw(6, 396);   //绘制最终板，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
				immediateContext->RSSetState(0);                       //恢复背面消隐
			}
		}

		//-------------------------------避免进入障碍物-------------------------------
		if (yNow == 0.0f) {//第一级台阶
			if (transZ > -4.0f&&transZ < 4.0f) {
				if (transX > 3.0f&&transX < 3.2f) transX = 3.0f;
				if (transX > 10.8f&&transX < 11.0f) transX = 11.0f;
			}
			if (transX > 3.0f&&transX < 11.0f) {
				if (transZ > -4.0f&&transZ < -3.8f) transZ = -4.0f;
				if (transZ > 3.8f&&transZ < 4.0f) transZ = 4.0f;
			}
		}
		else if (yNow == 2.0f) {//第二级台阶
			if (transZ > -4.0f&&transZ < 4.0f) {
				if (transX > 5.0f&&transX < 5.2f) transX = 5.0f;
				if (transX > 10.8f&&transX < 11.0f) transX = 11.0f;
			}
			if (transX > 5.0f&&transX < 11.0f) {
				if (transZ > -4.0f&&transZ < -3.8f) transZ = -4.0f;
				if (transZ > 3.8f&&transZ < 4.0f) transZ = 4.0f;
			}
		}
		else if (yNow == 4.0f) {//第三级台阶
			if (transZ > -4.0f&&transZ < 4.0f) {
				if (transX > 7.0f&&transX < 7.2f) transX = 7.0f;
				if (transX > 10.8f&&transX < 11.0f) transX = 11.0f;
			}
			if (transX > 7.0f&&transX < 11.0f) {
				if (transZ > -4.0f&&transZ < -3.8f) transZ = -4.0f;
				if (transZ > 3.8f&&transZ < 4.0f) transZ = 4.0f;
			}
		}
		else if (yNow == 6.0f) {//从第三级台阶跳向第一个木板
			if (transZ > -5.0f&&transZ < 5.0f) {
				if (transX > 13.0f&&transX < 13.2f) transX = 13.0f;
				if (transX > 20.8f&&transX < 21.0f) transX = 21.0f;
			}
			if (transX > 13.0f&&transX < 21.0f) {
				if (transZ > -5.0f&&transZ < -4.8f) transZ = -5.0f;
				if (transZ > 4.8f&&transZ < 5.0f) transZ = 5.0f;
			}
		}
		else if (yNow == 8.0f) {//从第一个木板跳向第二个木板
			if (transZ > -17.0f&&transZ < -7.0f) {
				if (transX > 13.0f&&transX < 13.2f) transX = 13.0f;
				if (transX > 20.8f&&transX < 21.0f) transX = 21.0f;
			}
			if (transX > 13.0f&&transX < 21.0f) {
				if (transZ > -7.0f&&transZ < -6.8f) transZ = -7.0f;
				if (transZ > -17.0f&&transZ < -16.8f) transZ = -17.0f;
			}
		}
		else if (yNow == 8.0f) {//从第二个木板跳向独木桥
			if (transZ > -36.0f&&transZ < -18.0f) {
				if (transX > 17.0f&&transX < 17.2f) transX = 17.0f;
				if (transX > 17.5f&&transX < 17.7f) transX = 17.5f;
			}
			if (transX > 13.0f&&transX < 21.0f) {
				if (transZ > -36.0f&&transZ < -35.8f) transZ = -36.0f;
				if (transZ > -18.0f&&transZ < -17.8f) transZ = -18.0f;
			}
		}
		else if (yNow >= 8.0f) {//从独木桥跳向斜独木桥
			if (transZ > -34.0f&&transZ < -35.5f) {
				if (transX > 17.0f&&transX < 17.2f) transX = 17.0f;
				if (transX > -0.32f&&transX < -0.3f) transX = -0.32f;
			}
			if (transX > -0.32f&&transX < 17.0f) {
				if (transZ > -34.2f&&transZ < -34.0f) transZ = -34.0f;
				if (transZ > -35.5&&transZ < -35.3f) transZ = -35.5f;
			}
		}
		else if (yNow >= 16.0f) {//从斜独木桥跳向小跳板
			if (transZ > -32.0f&&transZ < -30.0f) {
				if (transX > -5.0f&&transX < -4.8f) transX = -5.0f;
				if (transX > -3.2f&&transX < -3.0f) transX = -3.0f;
			}
			if (transX > -5.0f&&transX < -3.0f) {
				if (transZ > -32.0f&&transZ < -31.8f) transZ = -32.0f;
				if (transZ > -30.2&&transZ < -30.0f) transZ = -30.0f;
			}
		}
		else if (yNow >= 18.0f) {//从小跳板到终点
			if (transZ > -5.0f&&transZ < 5.0f) {
				if (transX > -10.0f&&transX < -9.8f) transX = -10.0f;
				if (transX > -0.2f&&transX < 0.0f) transX = 0.0f;
			}
			if (transX > -10.0f&&transX < 0.0f) {
				if (transZ > -5.0f&&transZ < -4.8f) transZ = -5.0f;
				if (transZ > 4.8&&transZ < 5.0f) transZ = 5.0f;
			}
		}
		//*******************************第二部分：实现各个功能**************************************



		//********************第三部分：绘制各个物体*************************************************
		D3DX11_TECHNIQUE_DESC techDesc;
		technique->GetDesc(&techDesc);    //获取technique的描述
		//绘制地板及墙壁
		//设置地板及墙壁的材质信息
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(floorMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(floorMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(floorMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(floorMaterial.power);
		//设置地板及墙壁的纹理
		world = XMMatrixIdentity();
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
		immediateContext->RSSetState(noCullRS);                 //关闭背面消隐，使镜头在墙背面也能看到墙
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureFloor);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 36);   //绘制地板，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureWall);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(24, 42);   //绘制墙壁，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureSky);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 66);   //绘制天空，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureStar);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(30, 366);   //绘制星空，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		immediateContext->RSSetState(0);                       //恢复背面消隐
		//绘制当前人物(箱子)
		world = XMMatrixTranslation(transX, transY, transZ);
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(boxMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(boxMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(boxMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(boxMaterial.power);
		//设置当前人物(箱子)的纹理
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBox);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(36, 0);   //绘制当前人物(箱子)

		//绘制障碍物
		world = XMMatrixIdentity();
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
		immediateContext->RSSetState(noCullRS);                 //关闭背面消隐，使镜头在障碍物背面也能看到障碍物
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(texturebrick);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(150, 72);   //绘制障碍物，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		//                     ↑
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureIron);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(72, 222);   //绘制障碍物，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureTanhuangding);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(12, 294);   //绘制弹簧顶，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureTanhuang);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(24, 306);   //绘制弹簧侧面，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBazi);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(12, 330);   //绘制最终板，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureIron);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(24, 342);   //绘制最终板，第二参数表示从顶点数组第36个（从0开始计算）顶点开始绘制
		immediateContext->RSSetState(0);                       //恢复背面消隐
		//********************第三部分：绘制各个物体*************************************************
		swapChain->Present(0, 0);
	}
	return true;
}
//**************框架函数******************


//
// 回调函数
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			::DestroyWindow(hwnd);
		if (wParam == VK_F1)  //按F1键将光源类型改为方向光
			lightType = 0;
		if (wParam == VK_F2)  //按F2键将光源类型改为点光源
			lightType = 1;
		if (wParam == VK_F3)  //按F3键将光源类型改为聚光灯光源
			lightType = 2;
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// 主函数WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{

	//初始化
	//**注意**:最上面声明的IDirect3DDevice9指针，在这里作为参数传给InitD3D函数
	if (!d3d::InitD3D(hinstance,
		800,
		600,
		&renderTargetView,
		&immediateContext,
		&swapChain,
		&device,
		&depthStencilBuffer,
		&depthStencilView))// [out]The created device.
	{
		::MessageBox(0, L"InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, L"Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	return 0;
}

//光源的常量缓存设置到Effect框架中
//由于光照设置比较复杂，所以以一个函数来进行设置
void SetLightEffect(Light light)
{
	//首先将光照类型，环境光强度，漫射光强度，镜面光强度设置到Effect中
	effect->GetVariableByName("type")->AsScalar()->SetInt(light.type);
	effect->GetVariableByName("LightAmbient")->AsVector()->SetFloatVector((float*)&(light.ambient));
	effect->GetVariableByName("LightDiffuse")->AsVector()->SetFloatVector((float*)&(light.diffuse));
	effect->GetVariableByName("LightSpecular")->AsVector()->SetFloatVector((float*)&(light.specular));

	//下面根据光照类型的不同设置不同的属性
	if (light.type == 0)  //方向光
	{
		//方向光只需要“方向”这个属性即可
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));
		//将方向光的Tectnique设置到Effect
		technique = effect->GetTechniqueByName("T_DirLight");
	}
	else if (light.type == 1)  //点光源
	{
		//点光源需要“位置”，“常量衰变因子”，“一次衰变因子”，“二次衰变因子”
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		//将点光源的Tectnique设置到Effect
		technique = effect->GetTechniqueByName("T_PointLight");
	}
	else if (light.type == 2) //聚光灯光源
	{
		//点光源需要“方向”，“方向”，“常量衰变因子”，“一次衰变因子”，“二次衰变因子”
		//“内锥角度”，“外锥角度”，“聚光灯衰减系数”
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));

		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		effect->GetVariableByName("LightAlpha")->AsScalar()->SetFloat(light.alpha);
		effect->GetVariableByName("LightBeta")->AsScalar()->SetFloat(light.beta);
		effect->GetVariableByName("LightFallOff")->AsScalar()->SetFloat(light.fallOff);

		//将聚光灯光源的Tectnique设置到Effect
		technique = effect->GetTechniqueByName("T_SpotLight");
	}
}




