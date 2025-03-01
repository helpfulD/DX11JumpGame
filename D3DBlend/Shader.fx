//////////////////////////////////////////////////////////////////////////
// 定义常量常量缓存
//////////////////////////////////////////////////////////////////////////
//坐标变换矩阵的常量缓存
cbuffer MatrixBuffer
{
	matrix World;         //世界坐标变换矩阵
	matrix View;          //观察坐标变换矩阵
	matrix Projection;    //投影坐标变换矩阵
	float4 EyePosition;   //视点位置
};

//材质信息的常量缓存
cbuffer MaterialBuffer
{
	float4 MatAmbient;    //材质对环境光的反射率
	float4 MatDiffuse;    //材质对漫反射光的反射率
	float4 MatSpecular;   //材质对镜面光的反射率
	float  MatPower;      //材质的镜面光反射系数
}

//光源的常量缓存
cbuffer LightBuffer
{
	int    type;          //光源类型
	float4 LightPosition; //光源位置
	float4 LightDirection;//光源方向
	float4 LightAmbient;  //环境光强度
	float4 LightDiffuse;  //漫反射光强度
	float4 LightSpecular; //镜面光强度
	float  LightAtt0;     //常量衰减因子
	float  LightAtt1;     //一次衰减因子
	float  LightAtt2;     //二次衰减因子
	float  LightAlpha;    //聚光灯内锥角度
	float  LightBeta;     //聚光灯外锥角度
	float  LightFallOff;  //聚光灯衰减系数
}

Texture2D Texture;       //纹理变量

SamplerState Sampler     //定义采样器
{
	Filter = MIN_MAG_MIP_LINEAR;   //采用线性过滤
	AddressU = WRAP;              //寻址模式为WRAP
	AddressV = WRAP;              //寻址模式为WRAP
};

//////////////////////////////////////////////////////////////////////////
//定义输入结构
//////////////////////////////////////////////////////////////////////////
//顶点着色器的输入结构
struct VS_INPUT
{
	float4 Pos : POSITION;   //位置
	float3 Norm : NORMAL;    //法向量
	float2 Tex : TEXCOORD0;  //纹理
};

//顶点着色器的输出结构
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;    //位置
	float2 Tex : TEXCOORD0;      //纹理
	float3 Norm : TEXCOORD1;     //法向量
	float4 ViewDirection : TEXCOORD2;   //视点方向
	float4 LightVector : TEXCOORD3;		//对点光源和聚光灯有效，
										//前3各分量记录“光照向量”
										//最后一个分量记录光照距离
};

//////////////////////////////////////////////////////////////////////////
// 顶点着色器
//////////////////////////////////////////////////////////////////////////
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;              //声明一个VS_OUTPUT对象

	output.Pos = mul(input.Pos, World);         //在input坐标上进行世界变换
	output.Pos = mul(output.Pos, View);         //进行观察变换
	output.Pos = mul(output.Pos, Projection);   //进行投影变换

	output.Norm = mul(input.Norm, (float3x3)World);  //获得output的方向量
	output.Norm = normalize(output.Norm);              //对法向量进行归一化

	float4 worldPosition = mul(input.Pos, World);          //获取顶点的世界坐标
	output.ViewDirection = EyePosition - worldPosition;    //获取视点方向
	output.ViewDirection = normalize(output.ViewDirection);//将视点方向归一化

	output.LightVector = LightPosition - worldPosition;    //获取光照方向
	output.LightVector = normalize(output.LightVector);    //将光照方向归一化
	output.LightVector.w = length(LightPosition - worldPosition);  //获取光照距离

	output.Tex = input.Tex;       //纹理设置

	return output;
}

//////////////////////////////////////////////////////////////////////////
// 像素着色器
//////////////////////////////////////////////////////////////////////////
//默认像素着色器
float4 PS(VS_OUTPUT input) : SV_Target
{
	return Texture.Sample(Sampler, input.Tex);    //返回纹理
}

// 平行光源像素着色器
// 这里以PS_INPUT的对象作为参数，返回一个float4的变量作为颜色
// 注意：这里的颜色并不是着色产生，而是由光照和材质相互作用产生的颜色
float4 PSDirectionalLight(VS_OUTPUT input) : SV_Target
{
	float4 finalColor;           //声明颜色向量，这个颜色是最终的颜色
	float4 ambientColor, diffuseColor, specularColor; //声明环境光，漫反射，镜面光颜色

	//光照方向,和光线照射方向相反
	float3 lightVector = -LightDirection.xyz;  //可以用.xyz的方式去float4对象的前3位向量
	lightVector = normalize(lightVector);      //归一化光照向量
	//用材质环境光反射率和环境光强度相乘得到环境光颜色
	ambientColor = MatAmbient * LightAmbient;
	//将顶点法向量和光照方向进行点乘得到漫反射光因子
	float diffuseFactor = dot(lightVector, input.Norm);
	if (diffuseFactor > 0.0f)  //漫反射光因子>0表示不是背光面
	{
		//用材质的漫反射光的反射率和漫反射光的光照强度以及反漫射光因子相乘得到漫反射光颜色
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;

		//根据光照方向和顶点法向量计算反射方向
		float3 reflection = reflect(-lightVector, input.Norm);
		//根据反射方向，视点方向以及材质的镜面光反射系数来计算镜面反射因子
		//pow(b, n)返回b的n次方
		float specularFactor = pow(max(dot(reflection, input.ViewDirection.xyz), 0.0f), MatPower);
		//材质的镜面反射率，镜面光强度以及镜面反射因子相乘得到镜面光颜色
		specularColor = MatSpecular * LightSpecular * specularFactor;
	}
	//最终颜色由境光，漫反射，镜面光颜色三者相加得到
	//saturate表示饱和处理，结果大于1就变成1，小于0就变成0，以保证结果在0-1之间
	//finalColor = saturate(ambientColor + diffuseColor + specularColor);
	float4 texColor = float4(1.f,1.f,1.f,1.f);
	texColor = Texture.Sample(Sampler, input.Tex);             //获取纹理颜色
	finalColor = saturate(ambientColor + diffuseColor + specularColor);  //获取光照颜色
	finalColor = texColor * finalColor;                       //纹理颜色与光照颜色相乘得到最后颜色
	finalColor.a = texColor.a * MatDiffuse.a;                 //获取alpha值即透明度
	return finalColor;
}

// 点光源像素着色器
// 计算方法和方向光不同，主要区别在于最终颜色的计算公式上
float4 PSPointLight(VS_OUTPUT input) : SV_Target
{
	float4 finalColor;         //声明颜色向量，这个颜色是最终的颜色
	float4 ambientColor, diffuseColor, specularColor;  //声明环境光，漫反射，镜面光颜色

	//光照向量等于顶点的光照向量
	float3 lightVector = input.LightVector.xyz;
	lightVector = normalize(lightVector);   //归一化
	//用材质环境光反射率和环境光强度相乘得到环境光颜色
	ambientColor = MatAmbient * LightAmbient;
	//将顶点法向量和光照方向进行点乘得到漫反射光因子
	float diffuseFactor = dot(lightVector, input.Norm);
	if (diffuseFactor > 0.0f)  //漫反射光因子>0表示不是背光面
	{
		//用材质的漫反射光的反射率和漫反射光的光照强度以及反漫射光因子相乘得到漫反射光颜色
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;
		//根据光照方向和顶点法向量计算反射方向
		float3 reflection = reflect(-lightVector, input.Norm);
		//根据反射方向，视点方向以及材质的镜面光反射系数来计算镜面反射因子
		float specularFactor = pow(max(dot(reflection, input.ViewDirection.xyz), 0.0f), MatPower);
		//材质的镜面反射率，镜面光强度以及镜面反射因子相乘得到镜面光颜色
		specularColor = MatSpecular * LightSpecular * specularFactor;
	}

	float d = input.LightVector.w;   //光照距离
	//距离衰减因子
	float att = LightAtt0 + LightAtt1 * d + LightAtt2 * d * d;
	//最终颜色由境光，漫反射，镜面光颜色三者相加后再除以距离衰减因子得到
	finalColor = saturate(ambientColor + diffuseColor + specularColor) / att;  //获取光照颜色
	float4 texColor = Texture.Sample(Sampler, input.Tex);   //获取纹理颜色
	finalColor = finalColor * texColor;                     //纹理颜色与光照颜色相乘得到最后颜色
	finalColor.a = texColor.a * MatDiffuse.a;               //获取alpha值即透明度
	return finalColor;
}

// 聚光灯像素着色器
float4 PSSpotLight(VS_OUTPUT input) : SV_Target
{
	float4 finalColor;        //声明颜色向量，这个颜色是最终的颜色
	float4 ambientColor, diffuseColor, specularColor;  //声明环境光，漫反射，镜面光颜色

	//光照向量等于顶点的光照向量
	float3 lightVector = input.LightVector.xyz;
	lightVector = normalize(lightVector); //归一化
	float d = input.LightVector.w;        //光照距离
	float3 lightDirection = LightDirection.xyz;  //光照方向
	lightDirection = normalize(lightDirection);  //归一化

	//判断光照区域
	float cosTheta = dot(-lightVector, lightDirection);
	//如果照射点位于圆锥体照射区域之外，则不产生光照
	if (cosTheta < cos(LightBeta / 2))
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
	//用材质环境光反射率和环境光强度相乘得到环境光颜色
	ambientColor = MatAmbient * LightAmbient;
	//将顶点法向量和光照方向进行点乘得到漫反射光因子
	float diffuseFactor = dot(lightVector, input.Norm);
	if (diffuseFactor > 0.0f)    //漫反射光因子>0表示不是背光面
	{
		//用材质的漫反射光的反射率和漫反射光的光照强度以及反漫射光因子相乘得到漫反射光颜色
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;
		//根据光照方向和顶点法向量计算反射方向
		float3 reflection = reflect(-lightVector, input.Norm);
		//根据反射方向，视点方向以及材质的镜面光反射系数来计算镜面反射因子
		float specularFactor = pow(max(dot(reflection, input.ViewDirection.xyz), 0.0f), MatPower);
		//材质的镜面反射率，镜面光强度以及镜面反射因子相乘得到镜面光颜色
		specularColor = MatSpecular * LightSpecular * specularFactor;
	}

	//距离衰减因子
	float att = LightAtt0 + LightAtt1 * d + LightAtt2 * d * d;
	if (cosTheta > cos(LightAlpha / 2)) //当照射点位于内锥体内
	{
		finalColor = saturate(ambientColor + diffuseColor + specularColor) / att;
	}
	else if (cosTheta >= cos(LightBeta / 2) && cosTheta <= cos(LightAlpha / 2))//当照射点位于内锥体和外锥体之间
	{
		//外锥体衰减因子
		float spotFactor = pow((cosTheta - cos(LightBeta / 2)) / (cos(LightAlpha / 2) - cos(LightBeta / 2)), 1);
		finalColor = spotFactor * saturate(ambientColor + diffuseColor + specularColor) / att;
	}
	float4 texColor = Texture.Sample(Sampler, input.Tex);  //获取纹理颜色
	finalColor = finalColor * texColor;                    //纹理颜色与光照颜色相乘得到最后颜色
	finalColor.a = texColor.a * MatDiffuse.a;              //获取alpha值即透明度
	return finalColor;
}
//////////////////////////////////////////////////////////////////////////
// 定义Technique
//////////////////////////////////////////////////////////////////////////
technique11 TexTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

//方向光Technique
technique11 T_DirLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PSDirectionalLight()));
	}
}

//点光源Technique
technique11 T_PointLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PSPointLight()));
	}
}

//聚光灯光源Technique
technique11 T_SpotLight
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PSSpotLight()));
	}
}