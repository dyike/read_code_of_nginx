# Nginx模块的数据结构
 > `./nginx-1.15.8/src/core/ngx_module.h`

## `ngx_module_t`的数据结构

```c
struct ngx_module_s {
    // ctx_index, index, spare0, spare1, version字段不需要再定义时赋值，可以使用nginx准备好的宏NGX_MODULE_V1定义
    // ctx_index判断当前模块在这类模块中顺序(注意同一类)，非常重要，用于表达优先级，也用于表明模块的位置。
    ngx_uint_t            ctx_index;
    // index表明当前模块在ngx_modules数组中的序号，于ctx_index有所不同，index是在所有模块中的顺序，同样重要。
    // nginx启动会会根据ngx_modules数组设置各个模块的index
    // ngx_max_module = 0;
    // for (i = 0; ngx_modules[i]; i++) {
    //     ngx_modules[i]->index = ngx_max_moudle++;
    // }
    ngx_uint_t            index;

    char                 *name;

    // 保留，暂未使用
    ngx_uint_t            spare0;
    ngx_uint_t            spare1;
    // 模块版本
    ngx_uint_t            version;
    const char           *signature;

    // 用于指向一类模块的上下文结构体
    // 每个模块都有自己的功能，比如事件模块处理I/O事件，HTTP模块处理HTTP应用层
    // ctx将会指向特定类型模块的公共结构，HTTP模块，需要将ctx指向ngx_http_module_t结构体
    void                 *ctx;
    // commands处理nginx.conf中配置项
    ngx_command_t        *commands;

    // 表示模块的类型，于ctx紧密相关
    // 取值为NGX_HTTP_MODULE,NGX_CORE_MODULE,NGX_CONF_MODULE,NGX_EVENT_MODULE,NGX_MAIL_MODULE五个值
    // 也可以定义自己的新类型
    ngx_uint_t            type;

    // 下面七个函数指针表示有7个执行点分别调用这7种方法，如果nginx不需要执行，设置NULL即可
    // master进程启动回调init_master,但目前为止，框架代码从来不会调用它，因此init_master设为NULL
    ngx_int_t           (*init_master)(ngx_log_t *log);
    // init_module回调方法在初始化所有模块时被调用，在master/worker模式下，启动worker子进程前完成
    ngx_int_t           (*init_module)(ngx_cycle_t *cycle);
    // init_process回调方法在正常服务前被调用。在master/worker模式下，多个worker子进程已经产生
    // 在每个worker进程初始化过程会调用所有模块的init_process函数
    ngx_int_t           (*init_process)(ngx_cycle_t *cycle);
    // Nginx不支持多线程，所有未被使用，设为NULL
    ngx_int_t           (*init_thread)(ngx_cycle_t *cycle);
    // 目前不支持，设为NULL
    void                (*exit_thread)(ngx_cycle_t *cycle);

    // exit_process回调方法在服务停止前被调用，在master/worker模式下，worker进程会在退出前调用它
    void                (*exit_process)(ngx_cycle_t *cycle);
    // exit_master在master退出前被调用
    void                (*exit_master)(ngx_cycle_t *cycle);


    // 保留字段
    uintptr_t             spare_hook0;
    uintptr_t             spare_hook1;
    uintptr_t             spare_hook2;
    uintptr_t             spare_hook3;
    uintptr_t             spare_hook4;
    uintptr_t             spare_hook5;
    uintptr_t             spare_hook6;
    uintptr_t             spare_hook7;
};

```


注意：对于`init_module,init_process,exit_process,exit_master`方法，调用他们是nginx的框架代码。这四个回调方法与HTTP框架无关。即使在nginx.conf中没有设置http{...}的配置项，这些回调仍然会被调用，因此在开发HTTP模块时，把他们设置为NULL。

最重要的是设置ctx和commands这两个成员，对于http类型的模块来说,`ngx_module_t`中的ctx指针必须指向`ngx_http_module_t`接口。

### 在自己的模块中定义模块，比如HTTP模块

```c
ngx_module_t  ngx_http_mytest_module = {
    NGX_MODULE_V1,
    &ngx_http_mytest_module_ctx,   /* module context */
    ngx_http_mytest_commands,    /* module directives */
    NGX_HTTP_MODULE,   /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};
```

## 模块配置指令
模块指令存储在一个 ngx_command_t 类型的静态数组结构中
### ngx_command_t结构
>`./nginx-1.15.8/src/core/ngx_conf_file.h`

```c
typedef struct ngx_command_s ngx_command_t;
struct ngx_command_s {
    // 配置项名称 如 gzip
    ngx_str_t             name;
    // 配置项类型，指定配置项可以出现的位置，详情core/ngx_conf_file.h
    ngx_uint_t            type;
    // 出现了name中指定的配置项后，调用set方法处理配置项参数
    // 这是一个函数指针，当Nginx在解析配置时，若遇到该配置指令，将会把读取到的值传递给这个函数进行分解处理。
    /*
    1. cf: 指向ngx_conf_t结构的指针，该结构包括从配置指令传递的参数
    2. cmd: 指向当前ngx_command_t结构
    3. conf: 指向模块配置结构
    */
    char               *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
    // 在配置文件中的偏移量，仅在type中没有设置NGX_DIRECT_CONF 和NGX_MAIN_CONF 时才生效。
    ngx_uint_t            conf;
    ngx_uint_t            offset;
    // 配置项读取后的处理方法，必须是ngx_conf_post_t结构
    void                 *post;
};
```

### 自己的http模块中的配置

```c
static ngx_command_t ngx_http_mytest_commands[] = {
    {
        ngx_string("test"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
        // ngx_http_mytest是ngx_command_t结构体重的set成员，
        // 完整定义为char *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
        // 当在某个配置块中出现mytest配置项，将会调用ngx_http_mytest方法
        ngx_http_mytest,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};
```

## 模块上下文
这是一个静态的 ngx_http_module_t 结构，它的名称是ngx_http__module_ctx。
### ngx_http_module_t结构
`
./nginx-1.15.8/src/http/ngx_http_config.h
`

```c
typedef struct {
    // 解析配置文件前调用
    ngx_int_t   (*preconfiguration)(ngx_conf_t *cf);
    // 完成配置文件的解析后调用
    ngx_int_t   (*postconfiguration)(ngx_conf_t *cf);
    // 当需要创建数据结构用于存储main级别的全局配置项
    void       *(*create_main_conf)(ngx_conf_t *cf);
    // 初始化main级别的配置项
    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);
    // 当需要创建数据结构用于存储srv级别的配置项
    void       *(*create_srv_conf)(ngx_conf_t *cf);
    // merge_srv_conf回调方法主要用于合并main和srv级别下的同名配置项
    char       *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);
    // 当需要创建数据结构用于存储loc级别的配置项
    void       *(*create_loc_conf)(ngx_conf_t *cf);
    // merge_loc_conf回调方法主要用于合并srv级别和loc级别下的同名配置项
    char       *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
} ngx_http_module_t;
```

调用顺序和定义顺序可能不一样，在HTTP框架调用的回调的顺序可能是这样的：

```
1) create_main_conf
2) create_srv_conf
3) create_loc_conf
4) preconfiguration
5) init_main_conf
6) merge_srv_conf
7) merge_loc_conf
8) postconfiguration
```

## 模块的定义
对任何开发模块，都需要定义一个 ngx_module_t 类型的变量来说明这个模块本身的信息。

### 例如http_test_module

```c
ngx_module_t ngx_http_mytest_module = {
    NGX_MODULE_V1,
    &ngx_http_mytest_module_ctx,
    ngx_http_mytest_commands,
    NGX_HTTP_MODULE,   /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};
```


## Handler模块

Handler 模块必须提供一个真正的处理函数，这个函数负责处理来自客户端的请求。该函数既可以选择自己直接生成内容，也可以选择拒绝处理，并由后续的 Handler 去进行处理，或者是选择丢给后续的 Filter 模块进行处理。

### 函数原型
```c
typedef ngx_int_t (*ngx_http_handler_pt)(ngx_http_request_t *r);
```
其中r是request结构http请求，包含客户端请求所有的信息，例如：request method, URI, and headers。
该函数处理成功返回NGX_OK，处理发生错误返回NGX_ERROR，拒绝处理（留给后续的Handler 进行处理）返回NGX_DECLINE。

Handler 模块处理过程中做了四件事情：获取location配置、生成合适的响应、发送响应的header头部、发送响应的body包体。

```c
static ngx_int_t
ngx_http_mytest_handler(ngx_http_request_t *r)
{
    printf("yuanfeng test: <%s, %u>\n", __FUNCTION__, __LINE__);
    if (!(r->method && (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    // discard request body
    // 如果不想处理请求中的包体，就调用ngx_http_discard_request_body()将接收自客户端的HTTP包体丢掉
    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }


    // set response header
    ngx_str_t type = ngx_string("text/plain");
    ngx_str_t response = ngx_string("{\"code\":0,\"msg\":\"ok\"}");
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = response.len;
    r->headers_out.content_type = type;

    // http响应主体包括响应行，响应头部，包体
    // 需要执行发送HTTP头部和发送HTTP包体两步操作
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    ngx_buf_t *b;
    b = ngx_create_temp_buf(r->pool, response.len);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_memcpy(b->pos, response.data, response.len);
    b->last = b->pos + response.len;
    b->last_buf = 1;

    ngx_chain_t out;
    out.buf = b;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
}
// 挂载函数
static char*
ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;
    // 1)找到mytest配置所属的配置块，clcf看上去像是location块内的数据结构，其实不然
    // 它是main、srv或者loc级别配置项
    // 2)在每个http{} 和 server{} 内都有一个ngx_http_core_loc_conf_t结构体
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    // HTTP框架在处理用户请求进行到NGX_HTTP_CONTENT_PHASE阶段时
    // 如果请求的主机域名，URI和mytest配置项所在的配置块相匹配，就调用ngx_http_mytest_handler方法处理请求
    // 该函数在ngx_http_core_content_phase中的ngx_http_finalize_request(r, r->content_handler(r));中的
    // r->content_handler(r)执行
    clcf->handler = ngx_http_mytest_handler;

    return NGX_CONF_OK;
}
```


