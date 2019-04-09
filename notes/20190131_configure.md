## 执行configure发生了什么

```bash
|-- objs
|   |-- autoconf.err
|   |-- Makefile
|   |-- ngx_auto_config.h
|   |-- ngx_auto_headers.h
|   |-- ngx_modules.c
|   `-- src
|       |-- core
|       |-- event
|       |   `-- modules
|       |-- http
|       |   |-- modules
|       |   |   `-- perl
|       |   `-- v2
|       |-- mail
|       |-- misc
|       |-- os
|       |   |-- unix
|       |   `-- win32
|       `-- stream
|
```

最终的产物是生成了nginx源码目录下面的`Makefile`文件和`objs`目录。

其中，src目录用于存在编译生产的目标文件。

### configure注释说明

```sh 

#!/bin/sh

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


LC_ALL=C
export LC_ALL

# auto/options脚本处理configure命令的参数
# 例如，如果参数是--help，那么显示支持的所有参数格式。options脚本会定义后续工作将要用到的变量，然后根据本次参数以及默认值设置这些变量
. auto/options
# auto/init脚本初始化后续将产生的文件路径。
# 例如，Makefile、ngx_modules.c等文件默认情况下将会在<nginx-source>/objs/
. auto/init
# auto/sources脚本将分析Nginx的源码结构，这样才能构造后续的Makefile文件
. auto/sources

# 编译过程中所有目标文件的路径有-builddir=DIR参数指定，默认情况下<nginx-source>/objs, 此时这个目录将会被创建
test -d $NGX_OBJS || mkdir -p $NGX_OBJS

# 开始准备创建ngx_auto_headers.h、ngx_autoconf.err等必要的编译文件
# auto/init中定义了  NGX_AUTO_HEADERS_H和NGX_AUTOCONF_ERR变量 
echo > $NGX_AUTO_HEADERS_H
echo > $NGX_AUTOCONF_ERR

# 向objs/ngx_auto_config.h写入命令行带的参数
echo "#define NGX_CONFIGURE \"$NGX_CONFIGURE\"" > $NGX_AUTO_CONFIG_H

# 判断DEBUG标志，如果有，那么在objs/ngx_auto_config.h文件中写入DEBUG宏
if [ $NGX_DEBUG = YES ]; then
    have=NGX_DEBUG . auto/have
fi

# 现在开始检查操作系统参数是否支持后续编译
if test -z "$NGX_PLATFORM"; then
    echo "checking for OS"
    # 系统名，操作系统版本，当前硬件设备类型
    NGX_SYSTEM=`uname -s 2>/dev/null`
    NGX_RELEASE=`uname -r 2>/dev/null`
    NGX_MACHINE=`uname -m 2>/dev/null`
    # 屏幕上输出OS名称，内核版本，32/64位内核
    echo " + $NGX_SYSTEM $NGX_RELEASE $NGX_MACHINE"

    NGX_PLATFORM="$NGX_SYSTEM:$NGX_RELEASE:$NGX_MACHINE";

    case "$NGX_SYSTEM" in
        MINGW32_* | MINGW64_* | MSYS_*)
            NGX_PLATFORM=win32
        ;;
    esac

else
    echo "building for $NGX_PLATFORM"
    NGX_SYSTEM=$NGX_PLATFORM
fi


# 检查并设置编译器，如gcc是否安装，gcc版本是否支持后续编译nginx
. auto/cc/conf

# 对非window操作系统定义一些必要的头文件，并检查是否存在，决定configure后续步骤是否可以成功
if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/headers
fi

# 对当前操作系统，定义一些特定的操作系统相关的方法并检查当前环境是否支持。
# 例如对于Linux，使用sched_setaffinity设置进程优先级，使用Linux特有的sendfile系统调用来加速向网络中发送文件块
. auto/os/conf

# 定义类UNIX 操作系统中通用的头文件和系统调用等，并检查当前环境是否支持
if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/unix
fi

. auto/threads

# 最核心的构造运行期的modules的脚本。将生成ngx_modules.c文件，编译进Nginx中，其中所做的唯一的事情就是定义ngx_modules数组。
# nginx_modules指明Nginx运行期哪些模块会参与到请求的处理中，包括HTTP请求可能会使用哪些模块处理，因此，它对数组元素的顺序非常敏感。
# 也就是说，绝大数模块是数组中的顺序是固定的。例如，一个请求必须先执行ngx_http_gzip_filter_module模块重新修改HTTP响应中的头部后，才能使用
# ngx_http_header_filter模块按照header_in结构体里的成员构造出TCP流形式发给客户端的HTTP响应头部。
# 注意，在--add-module=参数里加入第三方模块也在此步骤写入ngx_modules.c文件中。
. auto/modules

# conf脚本用来检查Nginx在链接期间需要链接的第三方静态库、动态库或者目标文件是否存在
. auto/lib/conf


# 处理Nginx安装后的路径
case ".$NGX_PREFIX" in
    .)
        NGX_PREFIX=${NGX_PREFIX:-/usr/local/nginx}
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;

    .!)
        NGX_PREFIX=
    ;;

    *)
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;
esac

# 处理Nginx安装后conf文件的路径
if [ ".$NGX_CONF_PREFIX" != "." ]; then
    have=NGX_CONF_PREFIX value="\"$NGX_CONF_PREFIX/\"" . auto/define
fi

# 处理Nginx安装后，二进制文件、pid、lock等其他文件的路径可参见configure参数中路径类选项的说明
have=NGX_SBIN_PATH value="\"$NGX_SBIN_PATH\"" . auto/define
have=NGX_CONF_PATH value="\"$NGX_CONF_PATH\"" . auto/define
have=NGX_PID_PATH value="\"$NGX_PID_PATH\"" . auto/define
have=NGX_LOCK_PATH value="\"$NGX_LOCK_PATH\"" . auto/define
have=NGX_ERROR_LOG_PATH value="\"$NGX_ERROR_LOG_PATH\"" . auto/define

have=NGX_HTTP_LOG_PATH value="\"$NGX_HTTP_LOG_PATH\"" . auto/define
have=NGX_HTTP_CLIENT_TEMP_PATH value="\"$NGX_HTTP_CLIENT_TEMP_PATH\""
. auto/define
have=NGX_HTTP_PROXY_TEMP_PATH value="\"$NGX_HTTP_PROXY_TEMP_PATH\""
. auto/define
have=NGX_HTTP_FASTCGI_TEMP_PATH value="\"$NGX_HTTP_FASTCGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_UWSGI_TEMP_PATH value="\"$NGX_HTTP_UWSGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_SCGI_TEMP_PATH value="\"$NGX_HTTP_SCGI_TEMP_PATH\""
. auto/define


# 创建编译时使用的objs/Makefile文件
. auto/make
# 为objs/Makefile加入需要连接的第三方静态库、动态库或者目标文件
. auto/lib/make
# 为objs/Makefile加入install功能，当执行make install时将编译生成的必要文件复制到安装路径，建立必要的目录
. auto/install

# STUB # # 在ngx_auto_config.h文件中加入NGX_SUPPRESS_WARN宏、NGX_SMP宏
. auto/stubs

# 在ngx_auto_config.h文件中指定NGX_USER和NGX_GROUP宏
# 如果执行configure时没有参数指定，默认两者皆为nobody（也就是默认以nobody用户运行进程）
have=NGX_USER value="\"$NGX_USER\"" . auto/define
have=NGX_GROUP value="\"$NGX_GROUP\"" . auto/define

if [ ".$NGX_BUILD" != "." ]; then
    have=NGX_BUILD value="\"$NGX_BUILD\"" . auto/define
fi

# 显示configure执行的结果，如果失败，则给出原因
. auto/summary

```