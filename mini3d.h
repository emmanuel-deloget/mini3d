﻿#ifndef mini3d_h_included

#include <stdint.h>
#include <math.h>

//=====================================================================
// 数学库：此部分应该不用详解，熟悉 D3D 矩阵变换即可
//=====================================================================
typedef struct { float m[4][4]; } matrix_t;
typedef struct { float x, y, z, w; } vector_t;
typedef vector_t point_t;

static inline int CMID(int x, int min, int max) { 
	return (x < min) ? min : ((x > max) ? max : x); 
}

// 计算插值：t 为 [0, 1] 之间的数值
static inline float interp(float x1, float x2, float t) { 
	return x1 + (x2 - x1) * t; 
}

// | v |
static inline float vector_length(const vector_t * v) {
	float sq = v->x * v->x + v->y * v->y + v->z * v->z;
	return (float)sqrt(sq);
}

// z = x + y
static inline void vector_add(vector_t* z, const vector_t* x, const vector_t* y) {
	z->x = x->x + y->x;
	z->y = x->y + y->y;
	z->z = x->z + y->z;
	z->w = 1.0;
}

// z = x - y
static inline void vector_sub(vector_t* z, const vector_t* x, const vector_t* y) {
	z->x = x->x - y->x;
	z->y = x->y - y->y;
	z->z = x->z - y->z;
	z->w = 1.0;
}

// 矢量点乘
static inline float vector_dotproduct(const vector_t* x, const vector_t* y) {
	return x->x* y->x + x->y * y->y + x->z * y->z;
}

// 矢量叉乘
static inline void vector_crossproduct(vector_t* z, const vector_t* x, const vector_t* y) {
	float m1, m2, m3;
	m1 = x->y * y->z - x->z * y->y;
	m2 = x->z * y->x - x->x * y->z;
	m3 = x->x * y->y - x->y * y->x;
	z->x = m1;
	z->y = m2;
	z->z = m3;
	z->w = 1.0f;
}

// 矢量插值，t取值 [0, 1]
static inline void vector_interp(vector_t* z, const vector_t* x1, const vector_t* x2, float t) {
	z->x = interp(x1->x, x2->x, t);
	z->y = interp(x1->y, x2->y, t);
	z->z = interp(x1->z, x2->z, t);
	z->w = 1.0f;
}

void vector_normalize(vector_t* v);

void matrix_add(matrix_t* c, const matrix_t* a, const matrix_t* b);
void matrix_sub(matrix_t* c, const matrix_t* a, const matrix_t* b);
void matrix_mul(matrix_t* c, const matrix_t* a, const matrix_t* b);
void matrix_scale(matrix_t* c, const matrix_t* a, float f);
void matrix_apply(vector_t* y, const vector_t* x, const matrix_t* m);
void matrix_set_identity(matrix_t* m);
void matrix_set_zero(matrix_t* m);
void matrix_set_translate(matrix_t* m, float x, float y, float z);
void matrix_set_scale(matrix_t* m, float x, float y, float z);
void matrix_set_rotate(matrix_t* m, float x, float y, float z, float theta);
void matrix_set_lookat(matrix_t* m, const vector_t* eye, const vector_t* at, const vector_t* up);
void matrix_set_perspective(matrix_t* m, float fovy, float aspect, float zn, float zf);

//=====================================================================
// 坐标变换
//=====================================================================
typedef struct {
	matrix_t world;         // 世界坐标变换
	matrix_t view;          // 摄影机坐标变换
	matrix_t projection;    // 投影变换
	matrix_t transform;     // transform = world * view * projection
	float w, h;             // 屏幕大小
}	transform_t;

// 矩阵更新，计算 transform = world * view * projection
void transform_update(transform_t* ts);

// 初始化，设置屏幕长宽
void transform_init(transform_t* ts, int width, int height);

// 将矢量 x 进行 project 
static inline void transform_apply(const transform_t* ts, vector_t* y, const vector_t* x) {
	matrix_apply(y, x, &ts->transform);
}

// backface culling in view space
int transform_check_ccw_culling(const vector_t *p1, const vector_t *p2, const vector_t *p3);

// 检查齐次坐标同 cvv 的边界用于视锥裁剪
int transform_check_cvv(const vector_t* v);

// 归一化，得到屏幕坐标
void transform_homogenize(const transform_t* ts, vector_t* y, const vector_t* x);

//=====================================================================
// 几何计算：顶点、扫描线、边缘、矩形、步长计算
//=====================================================================
typedef struct { float r, g, b; } color_t;
typedef struct { float u, v; } texcoord_t;
typedef struct { point_t pos; texcoord_t tc; color_t color; float rhw; } vertex_t;

typedef struct { vertex_t v, v1, v2; } edge_t;
typedef struct { float top, bottom; edge_t left, right; } trapezoid_t;
typedef struct { vertex_t v, step; int x, y, w; } scanline_t;

void vertex_rhw_init(vertex_t* v);
void vertex_interp(vertex_t* y, const vertex_t* x1, const vertex_t* x2, float t);
void vertex_division(vertex_t* y, const vertex_t* x1, const vertex_t* x2, float w);
void vertex_add(vertex_t* y, const vertex_t* x);

// 根据三角形生成 0-2 个梯形，并且返回合法梯形的数量
int trapezoid_init_triangle(trapezoid_t* trap, const vertex_t* p1,
	const vertex_t* p2, const vertex_t* p3);

// 按照 Y 坐标计算出左右两条边纵坐标等于 Y 的顶点
void trapezoid_edge_interp(trapezoid_t* trap, float y);

// 根据左右两边的端点，初始化计算出扫描线的起点和步长
void trapezoid_init_scan_line(const trapezoid_t* trap, scanline_t* scanline, int y);

//=====================================================================
// 渲染设备
//=====================================================================
typedef struct {
	transform_t transform;      // 坐标变换器
	int width;                  // 窗口宽度
	int height;                 // 窗口高度
	uint32_t** framebuffer;      // 像素缓存：framebuffer[y] 代表第 y行
	float** zbuffer;            // 深度缓存：zbuffer[y] 为第 y行指针
	uint32_t** texture;          // 纹理：同样是每行索引
	int tex_width;              // 纹理宽度
	int tex_height;             // 纹理高度
	float max_u;                // 纹理最大宽度：tex_width - 1
	float max_v;                // 纹理最大高度：tex_height - 1
	int render_state;           // 渲染状态
	uint32_t background;         // 背景颜色
	uint32_t foreground;         // 线框颜色
}	device_t;

#define RENDER_STATE_WIREFRAME      1		// 渲染线框
#define RENDER_STATE_TEXTURE        2		// 渲染纹理
#define RENDER_STATE_COLOR          4		// 渲染颜色
#define RENDER_STATE_CCW_CULLING    8		// cull backfaces

// 设备初始化，fb为外部帧缓存，非 NULL 将引用外部帧缓存（每行 4字节对齐）
void device_init(device_t* device, int width, int height, void* fb);

// 删除设备
void device_destroy(device_t* device);

// 设置当前纹理
void device_set_texture(device_t* device, void* bits, long pitch, int w, int h);

// 清空 framebuffer 和 zbuffer
void device_clear(device_t* device, int mode);

// 画点
static inline void device_pixel(device_t* device, int x, int y, uint32_t color) {
	if (((uint32_t)x) < (uint32_t)device->width && ((uint32_t)y) < (uint32_t)device->height) {
		device->framebuffer[y][x] = color;
	}
}

// 绘制线段
void device_draw_line(device_t* device, int x1, int y1, int x2, int y2, uint32_t c);

// 根据坐标读取纹理
uint32_t device_texture_read(const device_t* device, float u, float v);

//=====================================================================
// 渲染实现
//=====================================================================

// 绘制扫描线
void device_draw_scanline(device_t* device, scanline_t* scanline);

// 主渲染函数
void device_render_trap(device_t* device, trapezoid_t* trap);

// 根据 render_state 绘制原始三角形
void device_draw_primitive(device_t* device, const vertex_t* v1,
	const vertex_t* v2, const vertex_t* v3);



#endif // !mini3d_h_included
