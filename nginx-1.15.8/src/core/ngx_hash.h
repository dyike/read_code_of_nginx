
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HASH_H_INCLUDED_
#define _NGX_HASH_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/*
ngx_hash_elt_t中u_char name[1]的设计，如果在name很短的情况下，name和ushort的字节对齐可能只用占到一个字节，这样就比方一个uchar*的指针少占用一个字节。
*/
typedef struct {
    void             *value;
    u_short           len;    // key的长度
    u_char            name[1];  // key,小写字母
} ngx_hash_elt_t;   // ngx_hash_elt_t是ngx_hash_t->buckets[i]桶中的具体成员

// 在创建hash桶的时候赋值
typedef struct {
    ngx_hash_elt_t  **buckets; // hash桶(有size个桶)，指向各个桶的头部指针，也就是bucket[]数组，bucket[i]有指向每个桶中的第一个ngx_hash_elt_t成员。
    ngx_uint_t        size;  // hash桶个数，注意是桶的个数，不是每个桶中的成员个数，见ngx_hash_init
} ngx_hash_t;


// 这个结构主要是用于包含通配符的hash的结构，相比ngx_hash_t结构就多了一个value指针。value是用来存放某个已经达到末尾的统配url对应的value。
// 如果通配符url没有达到末尾，这个字段为NULL。
//
// ngx_hash_wildcard_t专用于表示牵制或后置通配符的哈希表，如：前置*.test.com，后置:www.test.* ，它只是对ngx_hash_t的简单封装
//
typedef struct {
    ngx_hash_t        hash;
    void             *value;
} ngx_hash_wildcard_t;


// hash表中元素ngx_hash_elt_t 预添加哈希散列元素结构 ngx_hash_key_t
// 赋值参考ngx_hash_add_key
typedef struct {
    ngx_str_t         key;  // key，为nginx的字符串结构
    ngx_uint_t        key_hash;  // 由该key计算出的hash值(通过hash函数如ngx_hash_key_lc())
    void             *value; // 该key对应的值，组成一个键-值对<key,value>,在通配符hash中也可能指向下一个通配符hash,见ngx_hash_wildcard_init
} ngx_hash_key_t;   // ngx_hash_init中names数组存入hash桶前，其结构是ngx_hash_key_t形式，在往hash桶里面存数据的时候，会把ngx_hash_key_t里面的成员拷贝到ngx_hash_elt_t中相应成员



typedef ngx_uint_t (*ngx_hash_key_pt) (u_char *data, size_t len);

/*
Nginx对于server- name主机名通配符的支持规则。
    首先，选择所有字符串完全匹配的server name，如www.testweb.com。
    其次，选择通配符在前面的server name，如*.testweb.com。
    再次，选择通配符在后面的server name，如www.testweb.*。
ngx_hash_combined_t是由3个哈希表组成，一个普通hash表hash，一个包含前向通配符的hash表wc_head和一个包含后向通配符的hash表 wc_tail。
*/
typedef struct {  // 这里面的hash信息是ngx_http_server_names中存储到hash表中的server_name及其所在的server{}上下文ctx，server_name为key，上下文ctx为value
    // 该hash中存放的是ngx_hash_keys_arrays_t->keys[]数组中的成员
    ngx_hash_t            hash;    // 普通hash，完全匹配
    // 该wc_head hash中存储的是ngx_hash_keys_arrays_t->dns_wc_head数组中的成员
    ngx_hash_wildcard_t  *wc_head;  // 前置通配符hash
    // 该wc_head hash中存储的是ngx_hash_keys_arrays_t->dns_wc_tail数组中的成员
    ngx_hash_wildcard_t  *wc_tail;  // 后置通配符hash
} ngx_hash_combined_t;


// hash初始化结构
typedef struct {
    ngx_hash_t       *hash;      // 指向待初始化的hash结构
    ngx_hash_key_pt   key;       // hash函数指针
    /* max_size和bucket_size的意义
    max_size表示最多分配max_size个桶，每个桶中的元素(ngx_hash_elt_t)个数 * NGX_HASH_ELT_SIZE(&names[n])不能超过bucket_size大小
    实际ngx_hash_init处理的时候并不是直接用max_size个桶，而是从x=1到max_size去试，只要ngx_hash_init参数中的names[]数组数据能全部hash到这x个桶中
    并且满足条件:每个桶中的元素(ngx_hash_elt_t)个数 * NGX_HASH_ELT_SIZE(&names[n])不超过bucket_size大小,则说明用x个桶就够用了，然后直接使用x个桶存储。
    */
    ngx_uint_t        max_size;     // 最多需要这么多个桶，实际上桶的个数，是通过计算得到的
    ngx_uint_t        bucket_size;  // 表示每个hash桶中(hash->buckets[i->成员[i]])对应的成员所有ngx_hash_elt_t成员暂用空间和的最大值，就是每个桶暂用的所有空间最大值，通过这个值计算需要多少个桶

    char             *name;
    ngx_pool_t       *pool;         // 该hash结构从pool指向的内存池中分配
    ngx_pool_t       *temp_pool;    // 分配临时数据空间的内存池
} ngx_hash_init_t;


#define NGX_HASH_SMALL            1   // NGX_HASH_SMALL表示初始化元素较少
#define NGX_HASH_LARGE            2   // NGX_HASH_LARGE表示初始化元素较多

#define NGX_HASH_LARGE_ASIZE      16384
#define NGX_HASH_LARGE_HSIZE      10007

#define NGX_HASH_WILDCARD_KEY     1    // 通配符类型
#define NGX_HASH_READONLY_KEY     2



// 该结构只是用来把完全匹配  前置匹配  后置匹配通过该结构体全部存储在该结构体对应的hash和数组中
// 该结构一般用来存储域名信息
typedef struct {
    ngx_uint_t        hsize;  // 散列中槽总数  如果是大hash桶方式，则hsize=NGX_HASH_LARGE_HSIZE,小hash桶方式，hsize=107,见ngx_hash_keys_array_init

    ngx_pool_t       *pool;   // 内存池，用于分配永久性的内存
    ngx_pool_t       *temp_pool;  // 临时内存池，下面的临时动态数组都是由临时内存池分配
    /* 下面这几个实际上是hash通的各个桶的头部指针，每个hash有ha->hsize个桶头部指针，在ngx_hash_add_key的时候头部指针指向每个桶中具体的成员列表
    keys_hash这是个二维数组，第一个维度代表的是bucket的编号，那么keys_hash[i]中存放的是所有的key算出来的hash值对hsize取模以后的值为i的key。
    假设有3个key,分别是key1,key2和key3假设hash值算出来以后对hsize取模的值都是i，那么这三个key的值就顺序存放在keys_hash[i][0],
    keys_hash[i][1], keys_hash[i][2]。该值在调用的过程中用来保存和检测是否有冲突的key值，也就是是否有重复。

    hash桶keys_hash  dns_wc_head_hash   dns_wc_tail_hash头部指针信息初始化在ngx_hash_keys_array_init，其中的具体
    桶keys_hash[i] dns_wc_head_hash[i]  dns_wc_tail_hash[i]中的数据类型为ngx_str_t，每个桶的数据成员默认4个，见ngx_hash_add_key，
    桶中存储的数据信息就是ngx_hash_add_key参数中key参数字符串

    数组keys[] dns_wc_head[] dns_wc_tail[]中的数据类型为ngx_hash_key_t，见ngx_hash_keys_array_init，
    ngx_hash_key_t中的key和value分别存储ngx_hash_add_key中的key参数和value参数

     赋值见ngx_hash_add_key
    原始key                  存放到hash桶(keys_hash或dns_wc_head_hash                 存放到数组中(keys或dns_wc_head或
                                    或dns_wc_tail_hash)                                     dns_wc_tail)
 www.example.com                 www.example.com(存入keys_hash)                        www.example.com (存入keys数组成员ngx_hash_key_t对应的key中)
  .example.com             example.com(存到keys_hash，同时存入dns_wc_tail_hash)        com.example  (存入dns_wc_head数组成员ngx_hash_key_t对应的key中)
 www.example.*                     www.example. (存入dns_wc_tail_hash)                 www.example  (存入dns_wc_tail数组成员ngx_hash_key_t对应的key中)
 *.example.com                     example.com  (存入dns_wc_head_hash)                 com.example. (存入dns_wc_head数组成员ngx_hash_key_t对应的key中)
    */
    ngx_array_t       keys;   // 数组成员ngx_hash_key_t
    ngx_array_t      *keys_hash; // keys_hash[i]对应hash桶头部指针，，具体桶中成员类型ngx_str_t

    ngx_array_t       dns_wc_head;  // 数组成员ngx_hash_key_t
    ngx_array_t      *dns_wc_head_hash;  // dns_wc_head_hash[i]对应hash桶头部指针，具体桶中成员类型ngx_str_t

    ngx_array_t       dns_wc_tail;    // 数组成员ngx_hash_key_t
    ngx_array_t      *dns_wc_tail_hash;  // dns_wc_tail_hash[i]对应hash桶头部指针，具体桶中成员类型ngx_str_t
} ngx_hash_keys_arrays_t;

/*
可以看到，ngx_table_elt_t就是一个key/value对，ngx_str_t 类型的key、value成员分别存储的是名字、值字符串。
hash成员表明ngx_table_elt_t也可以是某个散列表数据结构（ngx_hash_t类型）中的成员。ngx_uint_t 类型的hash
成员可以在ngx_hash_t中更快地找到相同key的ngx_table_elt_t数据。lowcase_key指向的是全小写的key字符串。
显而易见，ngx_table_elt_t是为HTTP头部“量身订制”的，其中key存储头部名称（如Content-Length），value存储对应的值（如“1024”），
*/
typedef struct {
    ngx_uint_t        hash;
    ngx_str_t         key;
    ngx_str_t         value;
    u_char           *lowcase_key;   // 存放的是本结构体中key的小写字母字符串
} ngx_table_elt_t;


void *ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len);
void *ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_combined(ngx_hash_combined_t *hash, ngx_uint_t key,
    u_char *name, size_t len);

ngx_int_t ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);
ngx_int_t ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);

#define ngx_hash(key, c)   ((ngx_uint_t) key * 31 + c)
ngx_uint_t ngx_hash_key(u_char *data, size_t len);
ngx_uint_t ngx_hash_key_lc(u_char *data, size_t len);
ngx_uint_t ngx_hash_strlow(u_char *dst, u_char *src, size_t n);


ngx_int_t ngx_hash_keys_array_init(ngx_hash_keys_arrays_t *ha, ngx_uint_t type);
ngx_int_t ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key,
    void *value, ngx_uint_t flags);


#endif /* _NGX_HASH_H_INCLUDED_ */
