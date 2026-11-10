
#include <math.h> 
#include <stdio.h> 
#include "uint_float_convert.h"

/**
************************************************************************
* @brief:      	float_to_uint: 浮点数转换为无符号整数函数
* @param[in]:   x_float:	待转换的浮点数
* @param[in]:   x_min:		范围最小值
* @param[in]:   x_max:		范围最大值
* @param[in]:   bits: 		目标无符号整数的位数
* @retval:     	无符号整数结果
* @details:    	将给定的浮点数 x 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个指定位数的无符号整数
************************************************************************
**/
int float_to_uint(float x_float, float x_min, float x_max, int bits)
{
	/* Converts a float to an unsigned int, given range and number of bits */
	float span = x_max - x_min;
	float offset = x_min;
	return (int) ((x_float-offset)*((float)((1<<bits)-1))/span);
}
/**
************************************************************************
* @brief:      	uint_to_float: 无符号整数转换为浮点数函数
* @param[in]:   x_int: 待转换的无符号整数
* @param[in]:   x_min: 范围最小值
* @param[in]:   x_max: 范围最大值
* @param[in]:   bits:  无符号整数的位数
* @retval:     	浮点数结果
* @details:    	将给定的无符号整数 x_int 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个浮点数
************************************************************************
**/
float uint_to_float(unsigned int x_int, float x_min, float x_max, int bits)
{
	/* converts unsigned int to float, given range and number of bits */
	float span = x_max - x_min;
	float offset = x_min;
	return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}

/**
 ************************************************************************
 * @brief:      	float2string: 将浮点数转换为字符串
 * @param[in]:   value: 待转换的浮点数
 * @param[out]:  str: 存储转换结果的字符串
 * @retval:     	str: 返回转换后的字符串
 * @details:    	将浮点数转换为带有三位小数的字符串
 ************************************************************************
 **/
char* float2string(float value, char *str)
{
    // 1. 提取整数部分
    int Head = (int)value;
    
    // 2. 提取小数部分：用 fabsf 强制取绝对值，杜绝第二个负号
    // 加 0.5f 是为了四舍五入，防止浮点精度丢失导致 0.999 变成 998
    int Point = (int)(fabsf(value - Head) * 1000.0f + 0.5f); 

    // 3. 处理 -0.xxx 这种特殊情况（整数部分是0，但整体是负数）
    if (value < 0.0f && Head == 0) {
        // %03d 表示强制输出 3 位，不足 3 位在前面补 0 (解决吞零问题)
        sprintf(str, "-0.%03d", Point);
    } else {
        sprintf(str, "%d.%03d", Head, Point);
    }
    
    return str;
}