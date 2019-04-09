## Nginx源码结构

### Nginx源码基本结构

学习 Nginx 的构架之前，对 Nginx 源码结构进行简单的分析，可以了解 Nginx 模块结构以及模块之间的关系。

```
.
├── core
├── event
├── http
├── mail
├── misc
├── os
└── stream
```

输出结果显示有7个目录文件，以下是这些目录文件的功能：

* core  ：Nginx的核心源代码，包括常用数据结构的以及Nginx 内核实现的核心代码；
* event：Nginx事件驱动模型，以及定时器的实现相关代码；
* http   ：Nginx 实现http 服务器相关的代码；
* mail  ：Nginx 实现邮件代理服务器相关的代码；
* misc ：辅助代码，测试C++头 的兼容性，以及对Google_PerfTools 的支持；
* os     ：不同体系统结构所提供的系统函数的封装，提供对外统一的系统调用接口；
* stream : 



### core 核心模块结构

```
/ * 实现对各模块的整体控制，是 Nginx 程序 main 函数 */
├── nginx.c
├── nginx.h
/ * Nginx中的基本数据结构及操作 */
├── ngx_array.c
├── ngx_array.h
├── ngx_hash.c
├── ngx_hash.h
├── ngx_list.c
├── ngx_list.h
├── ngx_queue.c
├── ngx_queue.h
├── ngx_radix_tree.c
├── ngx_radix_tree.h
├── ngx_rbtree.c
├── ngx_rbtree.h
├── ngx_buf.c
├── ngx_buf.h
/* 整个Nginx模块结构基本配置管理 */
├── ngx_conf_file.c
├── ngx_conf_file.h
├── ngx_config.h
/* 网络连接管理 */
├── ngx_connection.c
├── ngx_connection.h
/ * 定义一些头文件与结构别名 */
├── ngx_core.h
├── ngx_cpuinfo.c
/* CRC 校验表信息 */
├── ngx_crc32.c
├── ngx_crc32.h
├── ngx_crc.h
/* 实现对系统运行过程参数、资源的通用管理 */
├── ngx_cycle.c
├── ngx_cycle.h
/* 实现文件读写相关的功能 */
├── ngx_file.c
├── ngx_file.h
/* socket 网络套接字功能 */
├── ngx_inet.c
├── ngx_inet.h
/* 实现日志输出、管理的相关功能 */
├── ngx_log.c
├── ngx_log.h
├── ngx_syslog.c
├── ngx_syslog.h
/* hash字符串操作 */
├── ngx_md5.c
├── ngx_md5.h
├── ngx_murmurhash.c
├── ngx_murmurhash.h
/* 内存管理相关文件 */
├── ngx_open_file_cache.c
├── ngx_open_file_cache.h
├── ngx_palloc.c
├── ngx_palloc.h
├── ngx_shmtx.c
├── ngx_shmtx.h
├── ngx_slab.c
├── ngx_slab.h
/* PCRE 上层封装 */
├── ngx_parse.c
├── ngx_parse.h
/* 反向代理的协议信息 */
├── ngx_proxy_protocol.c
├── ngx_proxy_protocol.h
/* 实现支持正则表达式 */
├── ngx_regex.c
├── ngx_regex.h
/* 字符串处理功能 */
├── ngx_string.c
├── ngx_string.h
/* 时间获取与管理功能 */
├── ngx_times.c
└── ngx_times.h
├── ngx_parse_time.c
├── ngx_parse_time.h
/* Nginx线程池 */
├── ngx_thread_pool.c
├── ngx_thread_pool.h
/* Nginx模块管理 */
├── ngx_module.c
├── ngx_module.h
/* https://ivanzz1001.github.io/records/post/nginx/2018/11/10/nginx-source_part48 */
├── ngx_output_chain.c
/* 其他文件 */
├── ngx_resolver.c
├── ngx_resolver.h
├── ngx_rwlock.c
├── ngx_rwlock.h
├── ngx_sha1.c
├── ngx_sha1.h
├── ngx_spinlock.c
├── ngx_crypt.c
└── ngx_crypt.h
```


### event时间驱动模型结构

event 目录里面包含一种子目录 module 以及一些文件，除了 module 子目录，其他文件提供了事件驱动模型相关数据结构的定义、初始化、事件接收、传递、管理功能以及事件驱动模型调用功能。module 子目录里面的源码实现了Nginx 支持的事件驱动模型：AIO、epoll、kqueue、select、/dev/poll、poll 等事件驱动模型:

```
├── modules
│   ├── ngx_devpoll_module.c   /* dev/poll 事件驱动模型 */
│   ├── ngx_epoll_module.c     /* epoll 事件驱动模型 */
│   ├── ngx_eventport_module.c   /* 事件驱动模型端口 */
│   ├── ngx_kqueue_module.c   /* kqueue 事件驱动模型 */ 
│   ├── ngx_poll_module.c    /* poll 事件驱动模型 */
│   ├── ngx_select_module.c     /* Linux 平台下的 select 事件驱动模型 */
│   └── ngx_win32_select_module.c   /* Win32 平台下的 select 事件驱动模型 */
├── ngx_event_accept.c
├── ngx_event.c
├── ngx_event_connect.c
├── ngx_event_connect.h
├── ngx_event.h
├── ngx_event_openssl.c
├── ngx_event_openssl.h
├── ngx_event_openssl_stapling.c
├── ngx_event_pipe.c
├── ngx_event_pipe.h
├── ngx_event_posted.c
├── ngx_event_posted.h
├── ngx_event_timer.c
├── ngx_event_timer.h
└── ngx_event_udp.c

```


### Http模块结构

http 目录和 event 目录一样，通用包含了模块实现源码的 module 目录文件以及一些结构定义、初始化、网络连接建立、管理、关闭，以及数据报解析、服务器组管理等功能的源码文件。module 目录文件实现了HTTP 模块的功能。

```
├── modules
│   ├── ngx_http_access_module.c
│   ├── ngx_http_addition_filter_module.c
│   ├── ngx_http_auth_basic_module.c
│   ├── ngx_http_auth_request_module.c
│   ├── ngx_http_autoindex_module.c
│   ├── ngx_http_browser_module.c
│   ├── ngx_http_charset_filter_module.c
│   ├── ngx_http_chunked_filter_module.c
│   ├── ngx_http_dav_module.c
│   ├── ngx_http_degradation_module.c
│   ├── ngx_http_empty_gif_module.c
│   ├── ngx_http_fastcgi_module.c
│   ├── ngx_http_flv_module.c
│   ├── ngx_http_geoip_module.c
│   ├── ngx_http_geo_module.c
│   ├── ngx_http_grpc_module.c
│   ├── ngx_http_gunzip_filter_module.c
│   ├── ngx_http_gzip_filter_module.c
│   ├── ngx_http_gzip_static_module.c
│   ├── ngx_http_headers_filter_module.c
│   ├── ngx_http_image_filter_module.c
│   ├── ngx_http_index_module.c
│   ├── ngx_http_limit_conn_module.c
│   ├── ngx_http_limit_req_module.c
│   ├── ngx_http_log_module.c
│   ├── ngx_http_map_module.c
│   ├── ngx_http_memcached_module.c
│   ├── ngx_http_mirror_module.c
│   ├── ngx_http_mp4_module.c
│   ├── ngx_http_not_modified_filter_module.c
│   ├── ngx_http_proxy_module.c
│   ├── ngx_http_random_index_module.c
│   ├── ngx_http_range_filter_module.c
│   ├── ngx_http_realip_module.c
│   ├── ngx_http_referer_module.c
│   ├── ngx_http_rewrite_module.c
│   ├── ngx_http_scgi_module.c
│   ├── ngx_http_secure_link_module.c
│   ├── ngx_http_slice_filter_module.c
│   ├── ngx_http_split_clients_module.c
│   ├── ngx_http_ssi_filter_module.c
│   ├── ngx_http_ssi_filter_module.h
│   ├── ngx_http_ssl_module.c
│   ├── ngx_http_ssl_module.h
│   ├── ngx_http_static_module.c
│   ├── ngx_http_stub_status_module.c
│   ├── ngx_http_sub_filter_module.c
│   ├── ngx_http_try_files_module.c
│   ├── ngx_http_upstream_hash_module.c
│   ├── ngx_http_upstream_ip_hash_module.c
│   ├── ngx_http_upstream_keepalive_module.c
│   ├── ngx_http_upstream_least_conn_module.c
│   ├── ngx_http_upstream_random_module.c
│   ├── ngx_http_upstream_zone_module.c
│   ├── ngx_http_userid_filter_module.c
│   ├── ngx_http_uwsgi_module.c
│   ├── ngx_http_xslt_filter_module.c
│   └── perl
├── ngx_http.c
├── ngx_http_cache.h
├── ngx_http_config.h
├── ngx_http_copy_filter_module.c
├── ngx_http_core_module.c
├── ngx_http_core_module.h
├── ngx_http_file_cache.c
├── ngx_http.h
├── ngx_http_header_filter_module.c
├── ngx_http_parse.c
├── ngx_http_postpone_filter_module.c
├── ngx_http_request_body.c
├── ngx_http_request.c
├── ngx_http_request.h
├── ngx_http_script.c
├── ngx_http_script.h
├── ngx_http_special_response.c
├── ngx_http_upstream.c
├── ngx_http_upstream.h
├── ngx_http_upstream_round_robin.c
├── ngx_http_upstream_round_robin.h
├── ngx_http_variables.c
├── ngx_http_variables.h
├── ngx_http_write_filter_module.c
└── v2
    ├── ngx_http_v2.c
    ├── ngx_http_v2_encode.c
    ├── ngx_http_v2_filter_module.c
    ├── ngx_http_v2.h
    ├── ngx_http_v2_huff_decode.c
    ├── ngx_http_v2_huff_encode.c
    ├── ngx_http_v2_module.c
    ├── ngx_http_v2_module.h
    └── ngx_http_v2_table.c

```

