# lyserver
是一个c++服务器，reactor模式
# 文件目录结构
1.lib/aip是百度API的sdk
2.include, src分别是头文件和源文件
# 编译
直接下载到linux系统上用cmake编译
工程目录下创建一个build
1、mkdir build
2、cmake ..
3、make
4、cd bin
5、./server
# 服务器模块
1、主从reactor模式
2、线程池模块
3、连接池模块
4、定时器模块
5、存储模块
6、网络模块
7、日志模块
8、业务回调模块
9、FTP模块
10、other:python.h封装模块，A*模块，个性化模块，RF模块