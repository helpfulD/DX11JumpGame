#ifndef GRAVITY_H_H
#define GRAVITY_H_H

#include <xnamath.h>

const float gravity = -15.0f;//重力加速度

float transX = 0, transZ = 0, transY = 0;//三个坐标轴方向上的位移量

//初速度
float Speed = 0.0f;

bool flag = false;

void Gravity(float timeDleta);	//自由落体运动

#endif


