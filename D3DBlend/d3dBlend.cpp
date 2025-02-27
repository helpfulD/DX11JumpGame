#include "d3dUtility.h"
#include "Light.h"
#include "Gravity.h"
#include "Camera.h"
#include <math.h>

//����ȫ�ֵ�ָ��
ID3D11Device* device = NULL;//D3D11�豸�ӿ�
IDXGISwapChain* swapChain = NULL;//�������ӿ�
ID3D11DeviceContext* immediateContext = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;//��ȾĿ����ͼ  

//Effect���ȫ��ָ��
ID3D11InputLayout* vertexLayout;
ID3DX11Effect* effect;
ID3DX11EffectTechnique* technique;

//������������ϵ����
XMMATRIX world;         //��������任�ľ���
XMMATRIX view;          //���ڹ۲�任�ľ���
XMMATRIX projection;    //����ͶӰ�任�ľ���

ID3D11DepthStencilView * depthStencilView;
ID3D11Texture2D * depthStencilBuffer;

//�������ʺ͹��յ�ȫ�ֶ���
Material		boxMaterial;      //���Ӳ���
Material		floorMaterial;    //�ذ����
Material		waterMaterial;    //ˮ�����
Light			light[3];      //��Դ����
int             lightType = 0;  //��Դ����

ID3D11ShaderResourceView* textureBox;      //��������
ID3D11ShaderResourceView* textureFloor;    //�ذ�����
ID3D11ShaderResourceView* texturebrick;    //ש������
ID3D11ShaderResourceView* textureWater;    //ˮ������
ID3D11ShaderResourceView* textureWall;    //ǽ������
ID3D11ShaderResourceView* textureSky;    //�������
ID3D11ShaderResourceView* textureIron;    //��������
ID3D11ShaderResourceView* textureStar;    //�ǿ�����
ID3D11ShaderResourceView* textureTanhuang;    //��������
ID3D11ShaderResourceView* textureTanhuangding;    //���ɶ�����
ID3D11ShaderResourceView* textureBazi;    //���հ����� 
ID3D11ShaderResourceView* textureGameover;    //������������ 


ID3D11BlendState *blendStateAlpha;        //���״̬
ID3D11RasterizerState* noCullRS;          //��������״̬


// ���ù�Դ
Light dirLight, pointLight, spotLight;

Camera* camera = new Camera();    //���������
XMVECTOR Eye;                     //�ӵ�λ��

void SetLightEffect(Light light);

//����һ������ṹ����������������ͷ���������������
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

//**************����Ϊ��ܺ���******************
bool Setup()
{
	//������Ҫ����5����Ҫ����
	//��һ�������ⲿ�ļ�������fx�ļ���ͼ���ļ���
	//�ڶ�������������Ⱦ״̬
	//�������������벼��
	//���Ĳ��������㻺��
	//���岽���ò��ʺ͹���
	//*************��һ�������ⲿ�ļ�������fx�ļ���ͼ���ļ���****************************
	HRESULT hr = S_OK;              //����HRESULT�Ķ������ڼ�¼���������Ƿ�ɹ�
	ID3DBlob* pTechBlob = NULL;     //����ID3DBlob�Ķ������ڴ�Ŵ��ļ���ȡ����Ϣ
	//������֮ǰ������.fx�ļ���ȡ��ɫ�������Ϣ
	hr = D3DX11CompileFromFile(L"Shader.fx", NULL, NULL, NULL, "fx_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pTechBlob, NULL, NULL);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"fx�ļ�����ʧ��", L"Error", MB_OK); //�����ȡʧ�ܣ�����������Ϣ
		return hr;
	}
	//����D3DX11CreateEffectFromMemory����ID3DEffect����
	hr = D3DX11CreateEffectFromMemory(pTechBlob->GetBufferPointer(),
		pTechBlob->GetBufferSize(), 0, device, &effect);

	if (FAILED(hr))
	{
		::MessageBox(NULL, L"����Effectʧ��", L"Error", MB_OK);  //����ʧ�ܣ�����������Ϣ
		return hr;
	}
	//���ⲿͼ���ļ���������
	//��������
	D3DX11CreateShaderResourceViewFromFile(device, L"BOX.BMP", NULL, NULL, &textureBox, NULL);
	//���������
	D3DX11CreateShaderResourceViewFromFile(device, L"land.BMP", NULL, NULL, &textureFloor, NULL);
	//ˮ������
	D3DX11CreateShaderResourceViewFromFile(device, L"water.dds", NULL, NULL, &textureWater, NULL);
	//ש������
	D3DX11CreateShaderResourceViewFromFile(device, L"brick.dds", NULL, NULL, &texturebrick, NULL);
	//ǽ������
	D3DX11CreateShaderResourceViewFromFile(device, L"wall.BMP", NULL, NULL, &textureWall, NULL);
	//�������
	D3DX11CreateShaderResourceViewFromFile(device, L"sky.BMP", NULL, NULL, &textureSky, NULL);
	//��������
	D3DX11CreateShaderResourceViewFromFile(device, L"iron.BMP", NULL, NULL, &textureIron, NULL);
	//�ǿ�����
	D3DX11CreateShaderResourceViewFromFile(device, L"star.BMP", NULL, NULL, &textureStar, NULL);
	//��������
	D3DX11CreateShaderResourceViewFromFile(device, L"tanhuang.BMP", NULL, NULL, &textureTanhuang, NULL); 
	//���ɶ�����
	D3DX11CreateShaderResourceViewFromFile(device, L"tanhuangding.BMP", NULL, NULL, &textureTanhuangding, NULL); 
	//���հ�����
	D3DX11CreateShaderResourceViewFromFile(device, L"bazi.BMP", NULL, NULL, &textureBazi, NULL); 
	//������������
	D3DX11CreateShaderResourceViewFromFile(device, L"gameover.BMP", NULL, NULL, &textureGameover, NULL); 
	//*************��һ�������ⲿ�ļ�������fx�ļ���ͼ���ļ���****************************

	//*************�ڶ�������������Ⱦ״̬************************************************
	//�ȴ���һ�����״̬������
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc)); //�������
	blendDesc.AlphaToCoverageEnable = false;     //�ر�AlphaToCoverage���ز�������
	blendDesc.IndependentBlendEnable = false;    //����Զ��RenderTargetʹ�ò�ͬ�Ļ��״̬
	//ֻ���RenderTarget[0]���û��ƻ��״̬������1-7
	blendDesc.RenderTarget[0].BlendEnable = true;                   //�������
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;     //����Դ����
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//����Ŀ������
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;         //��ϲ���
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;      //Դ��ϰٷֱ�����
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;    //Ŀ���ϰٷֱ�����
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;    //��ϰٷֱȵĲ���
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;  //д����
	//����ID3D11BlendState�ӿ�
	device->CreateBlendState(&blendDesc, &blendStateAlpha);

	//�رձ�������
	D3D11_RASTERIZER_DESC ncDesc;
	ZeroMemory(&ncDesc, sizeof(ncDesc));  //�������
	ncDesc.CullMode = D3D11_CULL_NONE;   //�޳��ض�����������Σ����ﲻ�޳�����ȫ������
	ncDesc.FillMode = D3D11_FILL_SOLID;  //���ģʽ������Ϊ�������������
	ncDesc.FrontCounterClockwise = false;//�Ƿ�������ʱ��������������Ϊ����
	ncDesc.DepthClipEnable = true;       //������Ȳü�
	//����һ���رձ���������״̬������Ҫ�õ�ʱ������ø��豸������
	if (FAILED(device->CreateRasterizerState(&ncDesc, &noCullRS)))
	{
		MessageBox(NULL, L"Create 'NoCull' rasterizer state failed!", L"Error", MB_OK);
		return false;
	}
	//*************�ڶ�������������Ⱦ״̬************************************************

	//*************�������������벼��****************************************************
	//��GetTechniqueByName��ȡID3DX11EffectTechnique�Ķ���
	//������Ĭ�ϵ�technique��Effect
	technique = effect->GetTechniqueByName("TexTech");                //Ĭ��Technique

	//D3DX11_PASS_DESC�ṹ��������һ��Effect Pass
	D3DX11_PASS_DESC PassDesc;
	//����GetPassByIndex��ȡEffect Pass
	//������GetDesc��ȡEffect Pass��������������PassDesc������
	technique->GetPassByIndex(0)->GetDesc(&PassDesc);

	//�������������벼��
	//�������Ƕ���һ��D3D11_INPUT_ELEMENT_DESC���飬
	//�������Ƕ���Ķ���ṹ����λ������ͷ��������������������������Ԫ��
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	//layoutԪ�ظ���
	UINT numElements = ARRAYSIZE(layout);
	//����CreateInputLayout�������벼��
	hr = device->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertexLayout);
	//�������ɵ����벼�ֵ��豸��������
	immediateContext->IASetInputLayout(vertexLayout);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"����Input Layoutʧ��", L"Error", MB_OK);
		return hr;
	}
	//*************�������������벼��****************************************************

	//*************���Ĳ��������㻺��****************************************************
	//������Ҫ�������ӣ����ӣ��Լ�ˮ��Ķ���
	Vertex vertices[] =
	{
		//------------------------------------���ӵĶ���------------------------------------
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
		//�ذ�
		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 4.0f) },

		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 4.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 4.0f) },

		//ǰ��ǽ
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, -1.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		//����ǽ
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		//�����ǽ
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, -1.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		//�Ҳ���ǽ
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
			
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, -1.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },

		//���
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		//��һ��̨��
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

		//�ڶ���̨��
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

		//������̨��
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

		//̨��ǰ��ǽ
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

		//̨�׺��ǽ
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

		//̨�׵ı���
		{ XMFLOAT3(10.0f, 5.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, -1.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 3.0f) },
		{ XMFLOAT3(10.0f, -1.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 3.0f) },

		{ XMFLOAT3(10.0f, -1.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 3.0f) },
		{ XMFLOAT3(10.0f, 5.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 5.0f, -3.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(3.0f, 0.0f) },

		//��һ������
			//����
		{ XMFLOAT3(20.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -4.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(4.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -4.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -4.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 5.0f) },
			//����
		{ XMFLOAT3(20.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -4.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(4.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -4.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -4.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 5.0f) },
			//ǰ��
		{ XMFLOAT3(14.0f, 7.0f, 4.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -4.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(16.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 7.0f, -4.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(16.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -4.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//����
		{ XMFLOAT3(20.0f, 7.0f, 4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, 4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(16.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 6.0f, 4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(16.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -4.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//�����
		{ XMFLOAT3(14.0f, 7.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(6.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -4.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },
			//�Ҳ���
		{ XMFLOAT3(14.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(6.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, 4.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },


		//�ڶ�������
			//����
		{ XMFLOAT3(20.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -16.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(4.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -16.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -16.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 5.0f) },
			//����
		{ XMFLOAT3(20.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -16.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(4.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(4.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -16.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -16.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 5.0f) },
			//ǰ��
		{ XMFLOAT3(14.0f, 7.0f, -8.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 7.0f, -16.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(16.0f, 0.0f) },

		{ XMFLOAT3(14.0f, 7.0f, -16.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(16.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -16.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//����
		{ XMFLOAT3(20.0f, 7.0f, -8.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -8.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -16.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(16.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -16.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -8.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(16.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -16.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//�Ҳ���
		{ XMFLOAT3(14.0f, 7.0f, -16.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -16.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -16.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -16.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -16.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(6.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -16.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },
			//�����
		{ XMFLOAT3(14.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(20.0f, 7.0f, -8.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(20.0f, 7.0f, -8.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(14.0f, 6.0f, -8.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(6.0f, 0.0f) },
		{ XMFLOAT3(20.0f, 6.0f, -8.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },


		//�ڶ�������Ӷ�ľ��
			//����
		{ XMFLOAT3(17.5f,7.0f,-19.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,7.0f,-19.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(32.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-19.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(32.0f, 0.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//����
		{ XMFLOAT3(17.5f,6.5f,-19.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,6.5f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(32.0f, 0.0f) },

		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(32.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//ǰ��
		{ XMFLOAT3(17.0f,7.0f,-19.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(32.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(32.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-35.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//����
		{ XMFLOAT3(17.5f,7.0f,-19.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-19.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(32.0f, 0.0f) },

		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,6.5f,-19.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(32.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//�Ҳ���
		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(6.0f, 0.0f) },

		{ XMFLOAT3(17.5f,7.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,6.5f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },
			//�����
		{ XMFLOAT3(17.0f,7.0f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.5f,7.0f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(17.5f,7.0f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,6.5f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(17.5f,6.5f,-19.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },


			//�ٽ�б��ľ��
				//����
		{ XMFLOAT3(17.0f,7.0f,-34.5f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.32f,17.0f,-34.5f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 34.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-0.32f,17.0f,-34.5f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.32f,17.0f,-35.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 34.0f) },
			//ǰ��
		{ XMFLOAT3(-0.32f,17.0f,-34.5f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.32f,17.0f,-35.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-0.32f,17.0f,-35.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-35.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//����
		{ XMFLOAT3(16.75f,6.57f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 34.0f) },
		{ XMFLOAT3(16.75f,6.57f,-34.5f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-35.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-34.5f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 34.0f) },
			//����
		{ XMFLOAT3(17.0f,7.0f,-34.5f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(16.75f,6.57f,-35.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-34.5f), XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//�����
		{ XMFLOAT3(-0.32f,17.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,7.0f,-35.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(40.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-35.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(40.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-34.5f), XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },
			//�Ҳ���
		{ XMFLOAT3(-0.32f,17.0f,-34.5f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(17.0f,7.0f,-34.5f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(40.0f, 0.0f) },

		{ XMFLOAT3(17.0f,7.0f,-34.5f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.57f,16.57f,-34.5f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(40.0f, 0.0f) },
		{ XMFLOAT3(16.75f,6.57f,-35.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },

		//С����
			//����
		{ XMFLOAT3(-3.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-32.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-5.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,17.0f,-32.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-32.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//����
		{ XMFLOAT3(-3.0f,16.0f,-30.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-32.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-32.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-32.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//ǰ��
		{ XMFLOAT3(-5.0f,17.0f,-30.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-5.0f,17.0f,-32.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-5.0f,17.0f,-32.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-32.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//����
		{ XMFLOAT3(-3.0f,17.0f,-30.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-30.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-32.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-3.0f,17.0f,-32.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-30.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-32.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//�����
		{ XMFLOAT3(-5.0f,17.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-3.0f,17.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-32.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },
			//�Ҳ���
		{ XMFLOAT3(-5.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-3.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(2.0f, 0.0f) },

		{ XMFLOAT3(-3.0f,17.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-5.0f,16.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(2.0f, 0.0f) },
		{ XMFLOAT3(-3.0f,16.0f,-30.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },


			//���ս����� 
			//���� 
		{ XMFLOAT3(-10.0f,20.0f,5.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,20.0f,5.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,20.0f,-5.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(0.0f,20.0f,5.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.0f,20.0f,-5.0f),XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,20.0f,-5.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//����
		{ XMFLOAT3(-10.0f,19.0f,5.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,19.0f,-5.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,-5.0f),XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,-5.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),XMFLOAT2(0.0f, 1.0f) },
			//ǰ��
		{ XMFLOAT3(0.0f,20.0f,5.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.0f,20.0f,-5.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(10.0f, 0.0f) },

		{ XMFLOAT3(0.0f,20.0f,-5.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,-5.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//����
		{ XMFLOAT3(-10.0f,20.0f,5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,20.0f,-5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(10.0f, 0.0f) },

		{ XMFLOAT3(-10.0f,20.0f,-5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,19.0f,5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,-5.0f),XMFLOAT3(1.0f, 0.0f, 0.0f),XMFLOAT2(0.0f, 0.0f) },
			//�����
		{ XMFLOAT3(0.0f,20.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,20.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(10.0f, 0.0f) },

		{ XMFLOAT3(-10.0f,20.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.0f,19.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,-5.0f),XMFLOAT3(0.0f, 0.0f, 1.0f),XMFLOAT2(0.0f, 0.0f) },
			//�Ҳ���
		{ XMFLOAT3(0.0f,20.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-10.0f,20.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(10.0f, 0.0f) },

		{ XMFLOAT3(-10.0f,20.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.0f,19.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f,19.0f,5.0f),XMFLOAT3(0.0f, 0.0f, -1.0f),XMFLOAT2(0.0f, 0.0f) },

			//�ǿ�
			//ǰ��ǽ
		{ XMFLOAT3(-50.0f, 120.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 120.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.625f) },

		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0, 0.625f) },
		{ XMFLOAT3(50.0f, 120.0f, -50.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.625f) },

			//����ǽ
		{ XMFLOAT3(-50.0f, 120.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 120.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.625f) },
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0, 0.0f) },

		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0, 0.0f) },
		{ XMFLOAT3(50.0f, 120.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.625f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0, 0.625f) },

			//�����ǽ
		{ XMFLOAT3(-50.0f, 120.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.625f) },
		{ XMFLOAT3(-50.0f, 40.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.0f) },

		{ XMFLOAT3(-50.0f, 120.0f, -50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-50.0f, 120.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.625f) },
		{ XMFLOAT3(-50.0f, 40.0f, 50.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.625f) },

			//�Ҳ���ǽ
		{ XMFLOAT3(50.0f, 120.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.625f) },

		{ XMFLOAT3(50.0f, 120.0f, -50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(50.0f, 40.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0, 0.625f) },
		{ XMFLOAT3(50.0f, 120.0f, 50.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.625f) },

			//���
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

		/*/------------------------------------ˮ��Ķ���------------------------------------------
		{ XMFLOAT3(-10.0f, 0.8f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 0.8f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(-10.0f, 0.8f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },

		{ XMFLOAT3(-10.0f, 0.8f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 10.0f) },
		{ XMFLOAT3(10.0f, 0.8f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 0.0f) },
		{ XMFLOAT3(10.0f, 0.8f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(10.0f, 10.0f) },*/
	};
	UINT vertexCount = ARRAYSIZE(vertices);
	//�������㻺�棬����ͬʵ��4һ��
	//��������һ��D3D11_BUFFER_DESC�Ķ���bd
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * vertexCount;	//ע�⣺�������ﶨ����24����������Ҫ����24
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;		//ע�⣺�����ʾ�������Ƕ��㻺��
	bd.CPUAccessFlags = 0;

	//����һ��D3D11_SUBRESOURCE_DATA�������ڳ�ʼ������Դ
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;         //������Ҫ��ʼ�������ݣ���������ݾ��Ƕ�������
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	//����һ��ID3D11Buffer������Ϊ���㻺��
	ID3D11Buffer* vertexBuffer;
	//����CreateBuffer�������㻺��
	hr = device->CreateBuffer(&bd, &InitData, &vertexBuffer);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"����VertexBufferʧ��", L"Error", MB_OK);
		return hr;
	}

	UINT stride = sizeof(Vertex);                 //��ȡVertex�Ĵ�С��Ϊ���
	UINT offset = 0;                              //����ƫ����Ϊ0
	//���ö��㻺�棬�����Ľ��ͼ�ʵ��4
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//ָ��ͼԪ���ͣ�D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST��ʾͼԪΪ������
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//*************���Ĳ��������㻺��****************************************************

	//*************���岽���ò��ʺ͹���**************************************************
	//���ӵذ弰ǽ�Ĳ���
	floorMaterial.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	floorMaterial.diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	floorMaterial.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	floorMaterial.power = 5.0f;
	//���Ӳ���
	boxMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	boxMaterial.diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	boxMaterial.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	boxMaterial.power = 5.0f;
	/*/ˮ�����
	waterMaterial.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	waterMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.6f);//ˮ���alphaֵΪ0.6������͸����Ϊ40%
	waterMaterial.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
	waterMaterial.power = 5.0f;*/

	// �����ֻ��Ҫ���ã�����3�ֹ���ǿ��
	dirLight.type = 0;
	dirLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f);
	dirLight.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);   //ǰ��λ�ֱ��ʾ���������ǿ��
	dirLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //ͬ��
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //ͬ��

	// ���Դ��Ҫ���ã�λ�á�3�й���ǿ�ȡ�3��˥������
	pointLight.type = 1;
	pointLight.position = XMFLOAT4(0.0f, 5.0f, 0.0f, 1.0f); //��Դλ��
	pointLight.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);   //ǰ��λ�ֱ��ʾ���������ǿ��
	pointLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //ͬ��
	pointLight.specular = XMFLOAT4(1.0f, 1.0f,1.0f, 1.0f);  //ͬ��
	pointLight.attenuation0 = 0;      //����˥������
	pointLight.attenuation1 = 0.1f;   //һ��˥������
	pointLight.attenuation2 = 0;      //����˥������

	// �۹����Ҫ����Light�ṹ�����еĳ�Ա
	spotLight.type = 2;
	spotLight.position = XMFLOAT4(0.0f, 10.0f, 0.0f, 1.0f); //��Դλ��
	spotLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f); //���շ���
	spotLight.ambient = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);   //ǰ��λ�ֱ��ʾ���������ǿ��
	spotLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);   //ͬ��
	spotLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  //ͬ��
	spotLight.attenuation0 = 0;    //����˥������
	spotLight.attenuation1 = 0.1f; //һ��˥������
	spotLight.attenuation2 = 0;    //����˥������
	spotLight.alpha = XM_1DIV2PI;   //��׶�Ƕ�
	spotLight.beta = XM_PI;    //��׶�Ƕ�
	spotLight.fallOff = 0.5f;      //˥��ϵ����һ��Ϊ1.0

	light[0] = dirLight;
	light[1] = pointLight;
	light[2] = spotLight;
	//*************���岽���ò��ʺ͹���**************************************************
	

	return true;
}

void Cleanup()
{
	//�ͷ�ȫ��ָ��
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
		//********************��һ���֣�����3������ϵ�����յ��ⲿ����****************************
		//-------------------------------�������-------------------------------
		static float yNow = 0.0f;		//��ǰy����
		static bool onTheGround = true;	//�ж��Ƿ���������ߵ�����
		static float angleX = 0.0f;		//��ǰ��Y����ת�Ƕ�	
		static float angleY = 0.0f;		//��ǰ��XZ��ת�Ƕ�	
		static float keytime = 0.0f;	//�����ɿ�����Ծ  ���Ҽ�¼����ʱ��
		bool spaceUp = true;			//�����ɿ��ո����Ծ
		bool musicKey = false;			//���ֿ���
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };//����һ����������ɫ��Ϣ��4��Ԫ�طֱ��ʾ�죬�̣����Լ�alpha
		immediateContext->ClearRenderTargetView(renderTargetView, ClearColor);		//�����ǰ�󶨵����ģ����ͼ
		immediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		float BlendFactor[] = { 0,0,0,0 };		//��ָ��������ӣ�һ�㲻����������������������ָ��Ϊʹ��blend factor

		pointLight.position = XMFLOAT4(transX, transY+3.0f, transZ, 1.0f); //��Դλ��
		spotLight.position = XMFLOAT4(transX, transY+3.0f, transZ, 1.0f); //��Դλ��
		light[1] = pointLight;
		light[2] = spotLight;

		//-------------------------------ͨ�����ת�������-------------------------------
		Eye = XMVectorSet(transX, transY, transZ, 0.0f);//���λ��
		camera->SetEye(Eye);   //�����ӵ�λ��
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
		//ʹ�����Ǳ�����+-45��
		camera->Yaw(dx  * timeDelta);
		if (angleY< XM_PIDIV4&&angleY > -XM_PIDIV4) {
			camera->Pitch(dy * timeDelta);
		}
		else {
			angleY = 0.0f; 
		} 
		SetCursorPos(400,300);				//�������λ�ò��� 
		ShowCursor(false);					//�������Ϊ���ɼ� 
		//ʹ��ת�Ǳ�����0~2��
		if (angleX > XM_2PI) { 
			angleX -= XM_2PI; 
		}
		if (angleX < 0) { 
			angleX += XM_2PI;  
		} 
		  


		
		camera->Apply();//���ɹ۲����
		world = XMMatrixIdentity();//��ʼ���������
		projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 800.0f / 600.0f, 0.01f, 100.0f);//����ͶӰ����
		//������任����ĳ��������еľ�����������õ�Effect�����
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
		XMMATRIX ViewMATRIX = XMLoadFloat4x4(&camera->GetView());
		effect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)&ViewMATRIX);    //���ù۲�����ϵ
		effect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)&projection); //����ͶӰ����ϵ
		effect->GetVariableByName("EyePosition")->AsMatrix()->SetMatrix((float*)&Eye); //�����ӵ�
		//��Դ�ĳ��������еĹ�Դ��Ϣ���õ�Effect�����
		SetLightEffect(light[lightType]);
		//********************��һ���֣�����3������ϵ�����յ��ⲿ����****************************


		//*****************************�ڶ����֣�ʵ�ָ�������************************************
		//-------------------------------���̿�������ͷ�ƶ�-------------------------------
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

		//-------------------------------��Ծ-------------------------------
		if (::GetAsyncKeyState(VK_SPACE) & 0x8000f && onTheGround){ //���¿ո�
			keytime += 0.1f*timeDelta;
			spaceUp = false;
		}
		if (keytime != 0 && spaceUp) {							   //�ɿ��ո����Ծ
			keytime = 0.0f;
			Speed = 9.0f;
			musicKey = true;
			flag = true;
			onTheGround = false;
		}

		//-------------------------------����yNow,ʹ�����뿪��ǰƽ̨ʱ��������-------------------------------
		if (!(transX > -10.0f&&transX<-0.0f&&transZ > -5.0f&&transZ < 5.0f) && yNow >= 18.0f) {//���հ�
			yNow = 18.0f;
			flag = true;
		}
		if (!(transX > -5.0f&&transX<-3.0f&&transZ > -32.0f&&transZ < -30.0f) && yNow >= 16.0f) {//С����
			yNow = 16.0f;
			flag = true;
		}
		if (!(transX > -0.68f&&transX<17.0f&&transZ > -35.5f&&transZ < -34.0f) && yNow >= 8.0f + (17.32f - transX) * 0.57735f) {//�ڶ���ľ��
			yNow = 8.0f + (16.32f - transX);
			flag = true;
		}
		if (!(transX > 17.0f&&transX<17.5f&&transZ > -36.0f&&transZ < -18.0f) && yNow > 8.0f) {//��ľ��
			yNow = 6.0f;
			flag = true;
		}
		if (!(transX > 13.0f&&transX<21.0f&&transZ > -17.0f&&transZ < -7.0f) && yNow > 8.0f) {//�ڶ���ľ��
			yNow = 6.0f;
			flag = true;
		}
		if (!(transX > 13.0f&&transX<21.0f&&transZ > -5.0f&&transZ < 5.0f) && yNow > 8.0f) {//��һ��ľ��
			yNow = 6.0f;
			flag = true;
		}
		if (!(transX > 7.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f) && yNow >= 6.0f) {//������̨��
			yNow = 4.0f;
			flag = true;
		}
		if (!(transX > 5.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f) && yNow >= 4.0f) {//�ڶ���̨��
			yNow = 2.0f;
			flag = true;
		}
		if (!(transX > 3.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f) && yNow >= 2.0f) {//��һ��̨��
			yNow = 0.0f; 
			flag = true;
		}


		if (transX > 13.0f&&transX<21.0f&&transZ > -5.0f&&transZ < 5.0f&&transY >= 8.0f) {//�����ڵ�һ��ľ����ʱ
			yNow = 8.0f;
		}
		if (transX > 13.0f&&transX<21.0f&&transZ > -17.0f&&transZ < -7.0f&&transY >= 8.0f) {//�����ڵڶ���ľ����ʱ
			yNow = 8.0f;
		}
		if (transX > 17.0f&&transX<17.5f&&transZ > -36.0f&&transZ < -18.0f&&transY >= 8.0f) {//�����ڶ�ľ����ʱ
			yNow = 8.0f;
		}
		if (transX > -0.68f&&transX<17.0f&&transZ > -35.5f&&transZ < -34.0f&&transY >= 8.0f) {//������б��ľ����ʱ
			yNow = 8.0f + (17.32f - transX) * 0.57735f;
		}
		if (transX > -5.0f&&transX<-3.0f&&transZ > -32.0f&&transZ < -30.0f&&transY >= 16.0f) {//С����
			yNow = 18.0f;
			Speed = 40.0f;
			onTheGround = false;
			flag = true;
		}
		if (transX > -10.0f&&transX<-0.0f&&transZ > -5.0f&&transZ < 5.0f&&transY >= 21.0f) {//���հ�
			yNow = 21.0f;
		}
		if (transY > 40.0f) {
			lightType = 0;
		}

		//-------------------------------����-------------------------------
		if (flag) {


			if (musicKey) {
				PlaySound(L"jump.wav", NULL, SND_FILENAME | SND_ASYNC);
				musicKey = false;
			}
			Gravity(timeDelta);
			//���½���С��yNow,��ֹͣ�½�
			if (transY < yNow){
				transY = yNow;
				flag = false;
				onTheGround = true;
			}
			//�䵽�ϰ�����
			if (transX > 3.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f&&transY >= 2.0f) {//��һ��̨��
				yNow = 2.0f;
			}
			if (transX > 5.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f&&transY >= 4.0f) {//�ڶ���̨��
				yNow = 4.0f;
			}
			if (transX > 7.0f&&transX<11.0f&&transZ > -4.0f&&transZ < 4.0f&&transY >= 6.0f) {//������̨��
				yNow = 6.0f;
			}
			if (transX > 13.0f&&transX<21.0f&&transZ > -5.0f&&transZ < 5.0f&&transY >= 8.0f) {//��һ��ľ��
				yNow = 8.0f;
			}
			if (transX > 13.0f&&transX<21.0f&&transZ > -17.0f&&transZ < -7.0f&&transY >= 8.0f) {//�ڶ���ľ��
				yNow = 8.0f;
			}
			if (transX > 17.0f&&transX<17.5f&&transZ > -36.0f&&transZ < -18.0f&&transY >= 8.0f) {//��ľ��
				yNow = 8.0f;
			}
			if (transX > -0.32f&&transX<17.0f&&transZ > -35.5f&&transZ < -34.0f&&transY >= 8.0f) {//��ľ��
				yNow = 8.0f + (17.32f - transX) * 0.57735f;
			}
			if (transX > -5.0f&&transX<-3.0f&&transZ > -32.0f&&transZ < -30.0f&&transY >= 16.0f) {//С����
				yNow = 18.0f;
			}
			if (transX > -10.0f&&transX<-0.0f&&transZ > -5.0f&&transZ < 5.0f&&transY >= 21.0f) {//���հ�
				yNow = 21.0f;
				//����ʤ�����
				lightType = 0;
				world = XMMatrixIdentity();
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
				immediateContext->RSSetState(noCullRS);                 //�رձ���������ʹ��ͷ���ϰ��ﱳ��Ҳ�ܿ����ϰ���
				effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureGameover);
				technique->GetPassByIndex(0)->Apply(0, immediateContext);
				immediateContext->Draw(6, 396);   //�������հ壬�ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
				immediateContext->RSSetState(0);                       //�ָ���������
			}
		}

		//-------------------------------��������ϰ���-------------------------------
		if (yNow == 0.0f) {//��һ��̨��
			if (transZ > -4.0f&&transZ < 4.0f) {
				if (transX > 3.0f&&transX < 3.2f) transX = 3.0f;
				if (transX > 10.8f&&transX < 11.0f) transX = 11.0f;
			}
			if (transX > 3.0f&&transX < 11.0f) {
				if (transZ > -4.0f&&transZ < -3.8f) transZ = -4.0f;
				if (transZ > 3.8f&&transZ < 4.0f) transZ = 4.0f;
			}
		}
		else if (yNow == 2.0f) {//�ڶ���̨��
			if (transZ > -4.0f&&transZ < 4.0f) {
				if (transX > 5.0f&&transX < 5.2f) transX = 5.0f;
				if (transX > 10.8f&&transX < 11.0f) transX = 11.0f;
			}
			if (transX > 5.0f&&transX < 11.0f) {
				if (transZ > -4.0f&&transZ < -3.8f) transZ = -4.0f;
				if (transZ > 3.8f&&transZ < 4.0f) transZ = 4.0f;
			}
		}
		else if (yNow == 4.0f) {//������̨��
			if (transZ > -4.0f&&transZ < 4.0f) {
				if (transX > 7.0f&&transX < 7.2f) transX = 7.0f;
				if (transX > 10.8f&&transX < 11.0f) transX = 11.0f;
			}
			if (transX > 7.0f&&transX < 11.0f) {
				if (transZ > -4.0f&&transZ < -3.8f) transZ = -4.0f;
				if (transZ > 3.8f&&transZ < 4.0f) transZ = 4.0f;
			}
		}
		else if (yNow == 6.0f) {//�ӵ�����̨�������һ��ľ��
			if (transZ > -5.0f&&transZ < 5.0f) {
				if (transX > 13.0f&&transX < 13.2f) transX = 13.0f;
				if (transX > 20.8f&&transX < 21.0f) transX = 21.0f;
			}
			if (transX > 13.0f&&transX < 21.0f) {
				if (transZ > -5.0f&&transZ < -4.8f) transZ = -5.0f;
				if (transZ > 4.8f&&transZ < 5.0f) transZ = 5.0f;
			}
		}
		else if (yNow == 8.0f) {//�ӵ�һ��ľ������ڶ���ľ��
			if (transZ > -17.0f&&transZ < -7.0f) {
				if (transX > 13.0f&&transX < 13.2f) transX = 13.0f;
				if (transX > 20.8f&&transX < 21.0f) transX = 21.0f;
			}
			if (transX > 13.0f&&transX < 21.0f) {
				if (transZ > -7.0f&&transZ < -6.8f) transZ = -7.0f;
				if (transZ > -17.0f&&transZ < -16.8f) transZ = -17.0f;
			}
		}
		else if (yNow == 8.0f) {//�ӵڶ���ľ�������ľ��
			if (transZ > -36.0f&&transZ < -18.0f) {
				if (transX > 17.0f&&transX < 17.2f) transX = 17.0f;
				if (transX > 17.5f&&transX < 17.7f) transX = 17.5f;
			}
			if (transX > 13.0f&&transX < 21.0f) {
				if (transZ > -36.0f&&transZ < -35.8f) transZ = -36.0f;
				if (transZ > -18.0f&&transZ < -17.8f) transZ = -18.0f;
			}
		}
		else if (yNow >= 8.0f) {//�Ӷ�ľ������б��ľ��
			if (transZ > -34.0f&&transZ < -35.5f) {
				if (transX > 17.0f&&transX < 17.2f) transX = 17.0f;
				if (transX > -0.32f&&transX < -0.3f) transX = -0.32f;
			}
			if (transX > -0.32f&&transX < 17.0f) {
				if (transZ > -34.2f&&transZ < -34.0f) transZ = -34.0f;
				if (transZ > -35.5&&transZ < -35.3f) transZ = -35.5f;
			}
		}
		else if (yNow >= 16.0f) {//��б��ľ������С����
			if (transZ > -32.0f&&transZ < -30.0f) {
				if (transX > -5.0f&&transX < -4.8f) transX = -5.0f;
				if (transX > -3.2f&&transX < -3.0f) transX = -3.0f;
			}
			if (transX > -5.0f&&transX < -3.0f) {
				if (transZ > -32.0f&&transZ < -31.8f) transZ = -32.0f;
				if (transZ > -30.2&&transZ < -30.0f) transZ = -30.0f;
			}
		}
		else if (yNow >= 18.0f) {//��С���嵽�յ�
			if (transZ > -5.0f&&transZ < 5.0f) {
				if (transX > -10.0f&&transX < -9.8f) transX = -10.0f;
				if (transX > -0.2f&&transX < 0.0f) transX = 0.0f;
			}
			if (transX > -10.0f&&transX < 0.0f) {
				if (transZ > -5.0f&&transZ < -4.8f) transZ = -5.0f;
				if (transZ > 4.8&&transZ < 5.0f) transZ = 5.0f;
			}
		}
		//*******************************�ڶ����֣�ʵ�ָ�������**************************************



		//********************�������֣����Ƹ�������*************************************************
		D3DX11_TECHNIQUE_DESC techDesc;
		technique->GetDesc(&techDesc);    //��ȡtechnique������
		//���Ƶذ弰ǽ��
		//���õذ弰ǽ�ڵĲ�����Ϣ
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(floorMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(floorMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(floorMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(floorMaterial.power);
		//���õذ弰ǽ�ڵ�����
		world = XMMatrixIdentity();
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
		immediateContext->RSSetState(noCullRS);                 //�رձ���������ʹ��ͷ��ǽ����Ҳ�ܿ���ǽ
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureFloor);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 36);   //���Ƶذ壬�ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureWall);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(24, 42);   //����ǽ�ڣ��ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureSky);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(6, 66);   //������գ��ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureStar);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(30, 366);   //�����ǿգ��ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		immediateContext->RSSetState(0);                       //�ָ���������
		//���Ƶ�ǰ����(����)
		world = XMMatrixTranslation(transX, transY, transZ);
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
		effect->GetVariableByName("MatAmbient")->AsVector()->SetFloatVector((float*)&(boxMaterial.ambient));
		effect->GetVariableByName("MatDiffuse")->AsVector()->SetFloatVector((float*)&(boxMaterial.diffuse));
		effect->GetVariableByName("MatSpecular")->AsVector()->SetFloatVector((float*)&(boxMaterial.specular));
		effect->GetVariableByName("MatPower")->AsScalar()->SetFloat(boxMaterial.power);
		//���õ�ǰ����(����)������
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBox);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(36, 0);   //���Ƶ�ǰ����(����)

		//�����ϰ���
		world = XMMatrixIdentity();
		effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
		immediateContext->RSSetState(noCullRS);                 //�رձ���������ʹ��ͷ���ϰ��ﱳ��Ҳ�ܿ����ϰ���
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(texturebrick);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(150, 72);   //�����ϰ���ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		//                     ��
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureIron);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(72, 222);   //�����ϰ���ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureTanhuangding);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(12, 294);   //���Ƶ��ɶ����ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureTanhuang);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(24, 306);   //���Ƶ��ɲ��棬�ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureBazi);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(12, 330);   //�������հ壬�ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		effect->GetVariableByName("Texture")->AsShaderResource()->SetResource(textureIron);
		technique->GetPassByIndex(0)->Apply(0, immediateContext);
		immediateContext->Draw(24, 342);   //�������հ壬�ڶ�������ʾ�Ӷ��������36������0��ʼ���㣩���㿪ʼ����
		immediateContext->RSSetState(0);                       //�ָ���������
		//********************�������֣����Ƹ�������*************************************************
		swapChain->Present(0, 0);
	}
	return true;
}
//**************��ܺ���******************


//
// �ص�����
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
		if (wParam == VK_F1)  //��F1������Դ���͸�Ϊ�����
			lightType = 0;
		if (wParam == VK_F2)  //��F2������Դ���͸�Ϊ���Դ
			lightType = 1;
		if (wParam == VK_F3)  //��F3������Դ���͸�Ϊ�۹�ƹ�Դ
			lightType = 2;
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// ������WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{

	//��ʼ��
	//**ע��**:������������IDirect3DDevice9ָ�룬��������Ϊ��������InitD3D����
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

//��Դ�ĳ����������õ�Effect�����
//���ڹ������ñȽϸ��ӣ�������һ����������������
void SetLightEffect(Light light)
{
	//���Ƚ��������ͣ�������ǿ�ȣ������ǿ�ȣ������ǿ�����õ�Effect��
	effect->GetVariableByName("type")->AsScalar()->SetInt(light.type);
	effect->GetVariableByName("LightAmbient")->AsVector()->SetFloatVector((float*)&(light.ambient));
	effect->GetVariableByName("LightDiffuse")->AsVector()->SetFloatVector((float*)&(light.diffuse));
	effect->GetVariableByName("LightSpecular")->AsVector()->SetFloatVector((float*)&(light.specular));

	//������ݹ������͵Ĳ�ͬ���ò�ͬ������
	if (light.type == 0)  //�����
	{
		//�����ֻ��Ҫ������������Լ���
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));
		//��������Tectnique���õ�Effect
		technique = effect->GetTechniqueByName("T_DirLight");
	}
	else if (light.type == 1)  //���Դ
	{
		//���Դ��Ҫ��λ�á���������˥�����ӡ�����һ��˥�����ӡ���������˥�����ӡ�
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		//�����Դ��Tectnique���õ�Effect
		technique = effect->GetTechniqueByName("T_PointLight");
	}
	else if (light.type == 2) //�۹�ƹ�Դ
	{
		//���Դ��Ҫ�����򡱣������򡱣�������˥�����ӡ�����һ��˥�����ӡ���������˥�����ӡ�
		//����׶�Ƕȡ�������׶�Ƕȡ������۹��˥��ϵ����
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));

		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		effect->GetVariableByName("LightAlpha")->AsScalar()->SetFloat(light.alpha);
		effect->GetVariableByName("LightBeta")->AsScalar()->SetFloat(light.beta);
		effect->GetVariableByName("LightFallOff")->AsScalar()->SetFloat(light.fallOff);

		//���۹�ƹ�Դ��Tectnique���õ�Effect
		technique = effect->GetTechniqueByName("T_SpotLight");
	}
}




