#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "d3dUtility.h"

class Camera
{
	enum TransformFlag                    //变换标识符
	{
		None = 0,
		Rotate = (1 << 0),      //<<为位移操作符，即向左位移0位，即0001
		Translate = (1 << 1)    //0001向左位移一位，变成0010，
	};
public:
	Camera();

	void Pitch(float angle);                //仰俯
	void Yaw(float angle);                  //偏转

	void SetEye(const XMVECTOR& position);  //设置视点位置

	XMVECTOR& GetEye();          //获得当前视点位置
	XMVECTOR& GetAt();           //获得当前摄像机的观察点
	XMVECTOR& GetUp();           //获得当前摄像机的正方向

	void MoveForwardBy(float value);  //向前移动摄像机，value取负值则向后移动
	void MoveRightBy(float value);    //向右移动摄像机，value取负值则向左旋转
	void MoveUpBy(float value);       //向上移动摄像机，value取负值则向下移动

	void Apply();                     //应用当前设置生成观察变换矩阵

	XMFLOAT4X4& GetView();      //获得观察坐标系变换矩阵

	UINT flag;
	float rotateX;       //绕X轴的旋转角度
	float rotateY;       //绕Y轴的旋转角度
	float rotateZ;       //绕Z轴的旋转角度

	XMVECTOR move;              //移动向量
	XMFLOAT4X4 cameraRotation;  //相机旋转矩阵
	XMVECTOR eye;               //视点位置
	XMVECTOR up;                //摄像机正方向
	XMVECTOR at;                //观察位置
	XMFLOAT4X4 view;            //观察矩阵
};

#endif