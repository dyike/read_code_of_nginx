# 动态数组结构——ngx_array_t

## 概要

数组是C语言中最常用的数据类型之一，按固定大小分割。但是C语言中的数组是不能动态扩展，所以Nginx做了一个封装，成为类似于C++ STL中vector的数据类型。
优点：
* 访问速度快
* 元素个数可以动态扩展
* 内存池统一管理内存

文件在`src/core/ngx_array.h`和`src/core/ngx_array.c`两个文件中。

## 定义结构

```c
// 比如cookies是以ngx_array_t数组存储的
// struct ngx_array_t
// nginx 数组结构 {{{
typedef struct {
    void        *elts;		// 数组起始位置,可以是ngx_keyval_t  ngx_str_t  ngx_bufs_t ngx_hash_key_t等
    ngx_uint_t   nelts;		// 数组元素个数
    size_t       size;		// 单个元素大小
    ngx_uint_t   nalloc;	// 空间能够容纳元素个数
    ngx_pool_t  *pool;		// 内存池,赋值见ngx_init_cycle，为cycle的时候分配的pool空间
} ngx_array_t; // }}}
```

## 操作函数

```c
// 动态数组创建
ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
// 动态数组销毁
void ngx_array_destroy(ngx_array_t *a);
// 在动态数组尾部插入一个元素
void *ngx_array_push(ngx_array_t *a);
// 在动态数组尾部插入n个元素
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);


// 数组结构初始化（ngx_array_create 中调用）
static ngx_inline ngx_int_t
ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;
    // 为数组分配初始空间
    array->elts = ngx_palloc(pool, n * size);
    if (array->elts == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}
```


### 创建

首先在内存池中为 ngx_array_t 结构分配了空间，然后调用了 ngx_array_init 函数初始化 ngx_array_t 结构的各个域，并未实际的存储区域分配空间

```c
ngx_array_t *
ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size)
{
    ngx_array_t *a;

    a = ngx_palloc(p, sizeof(ngx_array_t));
    if (a == NULL) {
        return NULL;
    }

    if (ngx_array_init(a, p, n, size) != NGX_OK) {
        return NULL;
    }

    return a;
}

```
### 销毁




