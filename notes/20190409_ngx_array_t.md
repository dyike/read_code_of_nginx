# 动态数组结构——ngx_array_t

## 概要

数组是C语言中最常用的数据类型之一，按固定大小分割。但是C语言中的数组是不能动态扩展，所以Nginx做了一个封装，成为类似于C++ STL中vector的数据类型。
优点：
* 访问速度快
* 元素个数可以动态扩展
* 内存池统一管理内存

文件在`src/core/ngx_array.h`和`src/core/ngx_array.c`两个文件中。

## 定义结构
![ngx_array_t数组结构](https://raw.githubusercontent.com/dyike/read_code_of_nginx/master/images/ngx_array_t.jpg)
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

```


### 创建

首先分配数组头，然后分配数组数据区，两次分配均在传入的内存池(pool指向内存池)中进行，然后简单初始化数组头并返回数组头起始位置。

```c
ngx_array_t *
ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size)
{
    ngx_array_t *a;
    // 分配动态数组头部
    a = ngx_palloc(p, sizeof(ngx_array_t));
    if (a == NULL) {
        return NULL;
    }
    // 分配容量为n的动态数组数据区，并将其初始化
    if (ngx_array_init(a, p, n, size) != NGX_OK) {
        return NULL;
    }

    return a;
}

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
### 销毁

销毁数组的操作实现如下，包括销毁数组数据区和数组头。销毁动作实际上就是修改内存池的 last 指针，即数组的内存被内存池回收，并没有调用 free 等释放内存的操作。

```c
void
ngx_array_destroy(ngx_array_t *a)
{
    ngx_pool_t  *p;

    p = a->pool;
    // 移动内存池的last指针，释放数组所有元素所占据的内存
    if ((u_char *) a->elts + a->size * a->nalloc == p->d.last) {
        p->d.last -= a->size * a->nalloc;
    }
    // 释放数组首指针所占据的内存
    if ((u_char *) a + sizeof(ngx_array_t) == p->d.last) {
        p->d.last = (u_char *) a;
    }
}
```

### 添加元素

数组添加元素的操作有两个，ngx_array_push 和ngx_array_push_n，分别添加一个和多个元素。

实际的添加操作并不在这两个函数中完成，只是在这两个函数中申请元素所需的内存空间，并返回指向该内存空间的首地址，在利用指针赋值的形式添加元素。

```c
void *
ngx_array_push(ngx_array_t *a)
{
    void        *elt, *new;
    size_t       size;
    ngx_pool_t  *p;
    // 判断数组是否已满
    if (a->nelts == a->nalloc) {
        // 计算数组所有元素占据的内存大小
        size = a->size * a->nalloc;
        p = a->pool;
        /* 若当前内存池的内存空间至少可容纳一个元素大小 */
        if ((u_char *) a->elts + size == p->d.last
            && p->d.last + a->size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */
            p->d.last += a->size;
            a->nalloc++;

        } else {
            /* allocate a new array */
            /* 新的数组内存为当前数组大小的 2 倍 */
            new = ngx_palloc(p, 2 * size);
            if (new == NULL) {
                return NULL;
            }
            /* 首先把现有数组的所有元素复制到新的数组中 */
            ngx_memcpy(new, a->elts, size);
            a->elts = new;
            a->nalloc *= 2;
        }
    }

    elt = (u_char *) a->elts + a->size * a->nelts;
    a->nelts++;

    return elt;
}
// 数组增加n个元素
void *
ngx_array_push_n(ngx_array_t *a, ngx_uint_t n)
{
    void        *elt, *new;
    size_t       size;
    ngx_uint_t   nalloc;
    ngx_pool_t  *p;

    size = n * a->size;

    if (a->nelts + n > a->nalloc) {

        /* the array is full */

        p = a->pool;

        if ((u_char *) a->elts + a->size * a->nalloc == p->d.last
            && p->d.last + size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += size;
            a->nalloc += n;

        } else {
            /* allocate a new array */

            nalloc = 2 * ((n >= a->nalloc) ? n : a->nalloc);

            new = ngx_palloc(p, nalloc * a->size);
            if (new == NULL) {
                return NULL;
            }

            ngx_memcpy(new, a->elts, a->nelts * a->size);
            a->elts = new;
            a->nalloc = nalloc;
        }
    }

    elt = (u_char *) a->elts + a->size * a->nelts;
    a->nelts += n;

    return elt;
}
```

### 测试脚本

[源码](../nginx-1.15.8/src/test/ngx_array_t)


