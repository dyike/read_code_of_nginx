## Nginx 中的数据类型

### 整型类型

```c
# ngx_int_t 和 ngx_uint_t

typedef intptr_t ngx_int_t;
typedef uintptr_t ngx_uint_t;

```

在64位的机器上，`intptr_t` 和`uintptr_t` 分别是`long int`、`unsigned long int`的别名；
在32位的机器上，`intptr_t` 和`uintptr_t` 分别是`int`、`unsigned int`的别名。

在stdint.h 头文件中定义
```c
/* Types for `void *' pointers.  */
#if __WORDSIZE == 64
# ifndef __intptr_t_defined
typedef long int		intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned long int	uintptr_t;
#else
# ifndef __intptr_t_defined
typedef int			intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned int		uintptr_t;
```


### 字符串类型

`ngx_str_t`结构就是字符串

```c
typedef struct {
    size_t   len;     // 字符串有效疮毒
    u_char   *data;  // data指针指向字符串起始地址，此字符串未必会以'\0'作为结尾，必须要根据len来使用data成员
} ngx_str_t;
```

如果试图将`ngx_str_t`的data成员当做字符串来使用的情况，都可能会导致内存越界！


### 链表结构

`ngx_list_t`是链表容器

```c

// ngx_list_part_t 是结构体ngx_list_part_s的别名，描述的是链表的一个元素。
typedef struct ngx_list_part_s ngx_list_part_t;

struct ngx_list_part_s {
    void           *elts;    //指向数组的起始地址
    ngx_uint_t     nelts;    //数组中已经使用了多少个元素，nelts必须小于ngx_list_t结构中的nalloc
    ngx_list_part_t *next;   //下一个链表元素ngx_list_part_t的地址
};
typedef struct {
    ngx_list_part_t  *last;   //指向链表的最后一个数组元素 
    ngx_list_part_t  part;    //链表的首个数组元素
    size_t           size;    //限制每个数组元素占用的空间大小
    ngx_uint_t       nalloc;  //表示每个ngx_list_part_t数组的容量，一旦分配后不可更改
    ngx_pool_t       *pool;   //链表中管理内存分配的内存池对象
} ngx_list_t;
```

`ngx_list_t`不是单纯的链表，是一个存储数组的链表。每个链表元素`ngx_list_part_s`又是一个数组，连续的内存，依赖
于`ngx_list_t`中的Size和nalloc来表述数组的容量，同时又依赖于`ngx_list_part_t`成员中的nelts来表示数组当前已使用的容量。故，`ngx_list_t`是一个链表容器，而链表中的元素又是一个数组。












