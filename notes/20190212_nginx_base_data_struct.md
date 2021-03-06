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
    size_t   len;     // 字符串有效长度
    u_char   *data;  // data指针指向字符串起始地址，此字符串未必会以'\0'作为结尾，必须要根据len来使用data成员
} ngx_str_t;

typedef struct {
    ngx_str_t   key;
    ngx_str_t   value;
} ngx_keyval_t;

typedef struct {
    unsigned    len:28;

    unsigned    valid:1;
    unsigned    no_cacheable:1;
    unsigned    not_found:1;
    unsigned    escape:1;

    u_char     *data;
} ngx_variable_value_t;

```

如果试图将`ngx_str_t`的data成员当做字符串来使用的情况，都可能会导致内存越界！




### table数据结构

`ngx_table_elt_t`数据结构，就是一个key—value对

```c
typedef struct {
    ngx_uint_t     hash;
    ngx_str_t      key;
    ngx_str_t      value;
    u_char         *lowcase_key;   // 指向是的全小写的key字符串
} ngx_table_elt_t;
```

`ngx_table_elt_t`为http头部量身定制，其中key存储头部名称(如content-length)，value存储对应的(如“1024”)
`lowcase_key`是为了忽略http头部名称的大小写。


### 缓冲区数据结构

`ngx_buf_t`是处理大数据的关键数据结构，用于内存数据和磁盘数据。

```c
typedef struct ngx_buf_s  ngx_buf_t;

struct ngx_buf_s {
    /* pos通常是用来告诉使用者本次应该从pos这个位置开始处理内存中的数据，这样设置是因为 */
    /* 同一个ngx_buf_t可能被多次反复处理，当然pos的含义是由使用它的模块定义 */
    u_char          *pos;
    /* last表示有效的内容到此位置，pos与last之前的内存是希望nginx处理的内容 */
    /* 处理文件时，file_pos与file_last的含义与处理内存时的pos与last相同 */
    u_char          *last;
    off_t            file_pos;   // 将要处理的文件的位置
    off_t            file_last;  // 截止的文件位置

    u_char          *start;         /* start of buffer */
    u_char          *end;           /* end of buffer */
    ngx_buf_tag_t    tag;
    ngx_file_t      *file;
    ngx_buf_t       *shadow;


    /* the buf's content could be changed */
    /* 临时标志位，为1表示数据在内存中且这段内存可以修改 */
    unsigned         temporary:1;

    /*
     * the buf's content is in a memory cache or in a read only memory
     * and must not be changed
     */
    unsigned         memory:1;

    /* the buf's content is mmap()ed and must not be changed */
    unsigned         mmap:1;

    unsigned         recycled:1;
    unsigned         in_file:1;
    unsigned         flush:1;
    /* Nginx进程，所有的操作几乎都是异步的，有些框架代码在sync为1时，可能会有阻塞的方式进行 */
    /* I/O操作，他的意义视使用它的模块决定 */
    unsigned         sync:1;
    unsigned         last_buf:1;
    unsigned         last_in_chain:1;

    unsigned         last_shadow:1;
    unsigned         temp_file:1;

    /* STUB */ int   num;
};
```

### chain数据结构
`ngx_chain_t`与`ngx_buf_t`配合使用的链表数据结构

```c
typedef struct ngx_buf_s  ngx_buf_t;
struct ngx_chain_s {
    ngx_buf_t    *buf;  // 指向当前的ngx_buf_t缓冲区
    ngx_chain_t  *next;  // 用来指向下一个ngx_chain_t
};
```

在向用户发送HTTP包体，就需要传入`ngx_chain_t`链表对象。如果是最后一个`ngx_chain_t`，则需将next置为NULL，否则永远不会发送成功，而且这个请求一直不会结束。


