#ifndef GRAVITY_H_H
#define GRAVITY_H_H

#include <xnamath.h>

const float gravity = -15.0f;//�������ٶ�

float transX = 0, transZ = 0, transY = 0;//���������᷽���ϵ�λ����

//���ٶ�
float Speed = 0.0f;

bool flag = false;

void Gravity(float timeDleta);	//���������˶�

#endif


