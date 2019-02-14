## HTTP模块的数据结构
 > `./nginx-1.15.8/src/core/ngx_module.h`

定义HTTP模块

```c
ngx_module_t  ngx_http_mytest_module;
```

### `ngx_module_t`的数据结构


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



