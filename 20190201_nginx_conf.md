## Nginx的配置

### Nginx进程间的关系

Nginx是支持单进程提供服务的(master进程)，为什么要按照master-worker方式配置同时穷多个进程？

好处：
1) master进程不对用户请求提供服务，只用来管理真正提供服务的worker进程，所以master可以是唯一的。
专注的事情比较单一，比如启动服务，停止服务，重载配置文件，平滑升级等等。
通常情况下，root用户启动master进程，worker进程的权限小鱼或者等于master进程。当某个worker进程coredump，master进程会启动新的worker进程继续提供服务。

2) 多个worker进程提高服务健壮性，一个worker进程出错，其他worker进程仍然继续服务。充分SMP多核架构，实现微观上真正的多核并发处理。这也是为什么worker进程数设置与CPU核心数量一致的原因。不同的worker进程之间处理并发请求几乎没有同步锁的限制，worker进程通常不会进入休眠状态，最好保证每个worker进程绑定特定的CPU核心，这样进程切换的代价最小。

### 关于Nginx配置，需要实际操作

略

1) 是否以守护进程方式运行Nginx,默认开启
```
daemon on | off;
```

2) 是否以master/worker方式工作
```
master_process on | off;
```

3) 限制coredump核心转储文件的大小
```
worker_rlimit_core size;
```

4) 指定coredump文件生成目录
```
working_directory_path;
```





