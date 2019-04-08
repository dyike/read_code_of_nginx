## nginx http module开发

### 简单介绍nginx模块的开发流程

#### 先看全部的代码

[ngx_http_mytest_module.c](https://github.com/dyike/read_code_of_nginx/blob/master/nginx-1.15.8/src/ngx_http_mytest_module/ngx_http_mytest_module.c)

```c
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
/* ngx_http_mytest_handler方法处理传来的ngx_http_request_t对象中就有这个请求的内存池管理对象，
 * 对内存的操作都可以基于它来进行，这样在请求结束的时候，内存池分配的内存也会被释放
 * 假设不支持PUT方法，PUT方法返回NGX_HTTP_NOT_ALLOWED给用户
 */

static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r);
static char* ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// command注册
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

// 不管如何实现处理请求的ngx_http_mytest_handler方法
// 如果没有什么工作是必须在HTTP框架初始化时完成的，那就不必实现ngx_http_module_t的8个回调方法，
// 可以像下面这样定义ngx_http_module_t接口
static ngx_http_module_t ngx_http_mytest_module_ctx = {
    NULL, /* preconfiguration */
    NULL, /* postconfiguration */

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    NULL, /* create location configuration */
    NULL /* merge location configuration */
};

// 定义mytest模块
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
    ngx_str_t response = ngx_string("hi, this message comes from bili dev server");
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


#### 配置文件

[config配置文件](https://github.com/dyike/read_code_of_nginx/blob/master/nginx-1.15.8/src/ngx_http_mytest_module/config)

```
#!/bin/sh

# 仅在configure执行时使用，一般设置为模块名称
ngx_addon_name=ngx_http_mytest_module

# 在重新设置HTTP_MODULES变量时，不要直接覆盖它，因为configure调用到自定义的config脚本前，
# 已经将各个HTTP模块设置到HTTP_MODULES变量中了，因此需要如下设置
HTTP_MODULES="$HTTP_MODULES ngx_http_mytest_module"

# 用于指定新增代码模块的源代码
# 在设置NGX_ADDON_SRCS时可以使用$ngx_addon_dir变量，等价于configure执行时`--add-module=PATH`的path参数
NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_mytest_module.c"

```



#### 将Http模块编译到nginx中

我们的模块文件夹名称叫`ngx_http_mytest_module`，放在nginx-1.15.8下面的src目录下面，在编译的时候添加参数即可

```
./configure --prefix=/root/nginx --add-module=./src/ngx_http_mytest_module

make 
make install 

```


#### 测试模块的正确性

编辑`nginx.conf`文件

```
http {
    include       mime.types;
    default_type  application/octet-stream;
    sendfile        on;
    keepalive_timeout  65;
    server {
        listen       80;
        server_name  localhost;
        location = /test {
            test;
        }
    }
}
```

请求将会看到效果

```
curl -v "http://localhost/test"
```

