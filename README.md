# linux_kernel_module_use_tracepoint

## 模块说明：

该代码提供了一个可用的linux内核模块使用tracepoint示例，并提供了十分详细的代码注释。示例使用的是sched_switch这个tracepoint，测试于linux-5.15.77环境。

## 模块核心原理：

找到内核中用于管理对应tracepoint的struct tracepoint结构体，然后在struct tracepoint对应成员填入自定义回调函数地址。

## 修改用于其他tracepoint方法：

1. 生成一个管理想要使用的tracepoint的struct interest_tracepoint结构体，可用INIT_INTEREST_TRACEPOINT宏完成。
2. 生成一个用于for_each_kernel_tracepoint回调来比对tracepoint表条目以找到指定tracepoint对应struct tracepoint地址的回调函数，可用TRACEPOINT_FIND宏完成。
3. 自定义用于tracepoint的回调函数。
4. 修改tracepoint_init函数中的几句代码。
5. 在tracepoint_exit中完成回调函数注销工作。

## 感谢与参考：

https://gist.github.com/HugoGuiroux/0894091275169750d22f 