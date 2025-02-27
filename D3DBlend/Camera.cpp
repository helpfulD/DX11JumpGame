#include "Camera.h"

Camera::Camera()  //构造函数，初始化Camera类的成员变量
{
	flag = Rotate | Translate;    //指定标识符
	rotateX = 0.0f;               //绕X轴旋转的角度
	rotateY = 0.0f;                //绕Y轴旋转的角度
	rotateZ = 0.0f;                //绕Z轴旋转的角度
	move = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);  //初始化移动位置
	eye = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);   //初始化视点位置
	//初始化相机的旋转矩阵
	//这里需要将XMMATRIX对象转变成XMStoreFloat4x4对象
	XMMATRIX cameraRotationMATRIX = XMMatrixIdentity();
	XMStoreFloat4x4(&cameraRotation, cameraRotationMATRIX);
	//初始化相机的观察矩阵
	//同样需要将XMMATRIX对象转变成XMStoreFloat4x4对象
	XMMATRIX viewMATRIX = XMMatrixIdentity();
	XMStoreFloat4x4(&view, viewMATRIX);
}

//仰俯
void Camera::Pitch(float angle)
{
	rotateX += angle;
	flag |= Rotate;
}

//偏转
void Camera::Yaw(float angle)
{
	rotateY += angle;
	flag |= Rotate;
}

//设置视点位置
void Camera::SetEye(const XMVECTOR& position)
{
	eye = position;
	flag |= Translate;
}

//获得当前视点位置
XMVECTOR& Camera::GetEye()
{
	return eye;
}

//获得当前摄像机的观察点
XMVECTOR& Camera::GetAt()
{
	return at;
}

//获得当前摄像机的正方向
XMVECTOR& Camera::GetUp()
{
	return up;
}

//向前移动摄像机，value取负值则向后移动
void Camera::MoveForwardBy(float value)
{
	move = XMVectorSetZ(move, value);
	flag |= Translate;
}

//向右移动摄像机，value取负值则向左旋转
void Camera::MoveRightBy(float value)
{
	move = XMVectorSetX(move, value);
	flag |= Translate;
}

//向上移动摄像机，value取负值则向下移动
void Camera::MoveUpBy(float value)
{
	move = XMVectorSetY(move, value);
	flag |= Translate;
}

//应用当前设置生成观察变换矩阵
void Camera::Apply()
{
	XMMATRIX cameraRotationMATRIX;    //声明XMMATRIX一个对象用于矩阵变换
	if (flag != None)
	{
		if ((flag & Rotate) != 0)  //如果包含旋转标识
		{
			//将Rotate取否，即1110，再和flag进行与运算
			//如果flag=0011则运算结果为0010
			flag &= ~Rotate;
			//在三个坐标系上进行旋转变换
			cameraRotationMATRIX = XMMatrixRotationX(rotateX) * XMMatrixRotationY(rotateY) * XMMatrixRotationZ(rotateZ);
			XMStoreFloat4x4(&cameraRotation, cameraRotationMATRIX); //将生成的XMMATRIX存放到成员cameraRotation
		}

		if ((flag & Translate) != 0)//如果包含平移标识
		{
			//将Translate取否，即1110，再和flag进行与运算
			//如果flag=0010则运算结果为0000
			flag &= ~Translate;
			//从成员cameraRotation获得旋转矩阵
			cameraRotationMATRIX = XMLoadFloat4x4(&cameraRotation);
			//重新计算视点位置
			eye += XMVector4Transform(move, cameraRotationMATRIX);
			move = XMVectorZero(); //重置移动向量
		}
		//从成员cameraRotation获得旋转矩阵
		cameraRotationMATRIX = XMLoadFloat4x4(&cameraRotation);
		//重新计算观察点
		at = eye + XMVector4Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), cameraRotationMATRIX);
		//重新计算摄像机正方向
		up = XMVector4Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), cameraRotationMATRIX);
		//重新计算观察矩阵
		XMMATRIX viewMATRIX = XMMatrixLookAtLH(eye, at, up);
		//将新生成的观察矩阵存入成员view中
		XMStoreFloat4x4(&view, viewMATRIX);
	}
}

//获得观察坐标系变换矩阵
XMFLOAT4X4& Camera::GetView()
{
	return view;
}
