
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGINX_H_INCLUDED_
#define _NGINX_H_INCLUDED_


#define nginx_version      1015008
#define NGINX_VERSION      "1.15.8"
#define NGINX_VER          "nginx/" NGINX_VERSION

#ifdef NGX_BUILD
#define NGINX_VER_BUILD    NGINX_VER " (" NGX_BUILD ")"
#else
#define NGINX_VER_BUILD    NGINX_VER
#endif

/*
在执行不重启不服务升级Nginx的操作，老的Nginx进程会通过环境变量"NGINX"来传递需要打开的监听端口
新的Nginx进程会通过ngx_add_inherited_sockets方法来使用已经打开的TCP监听端口
*/
#define NGINX_VAR          "NGINX"
#define NGX_OLDPID_EXT     ".oldbin" // 热升级nginx可执行文件的时候，修改nginx.pid为nginx.pid.oldbin


#endif /* _NGINX_H_INCLUDED_ */
