FROM lyserver_base:0.0.2

# 将项目文件从宿主机复制到镜像中的指定路径
COPY . /home/core

# 设置工作目录为项目路径
WORKDIR /home/core

CMD [ "./home/lyserver_master/build/bin/ly_server" ]
EXPOSE 22 8080 8999 8998 8997

#端口映射
# -p 8080:8080 -p 8999:8999 -p 8998:8998 -p 8997:8997  




















# 使用官方的C++编译器镜像作为基础镜像
# FROM gcc:latest

# ADD sources.list /etc/apt/
# # 安装Python 3.10
# RUN apt-get update && apt-get install -y python3.10 python3-pip python3.10-distutils

# # 安装C++11支持的编译器
# RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 60 --slave /usr/bin/g++ g++-11

# # 安装必要的库和工具
# RUN apt-get install -y \
#     libmysqlclient-dev \
#     libssl-dev \
#     libcrypto++-dev \
#     libcurl4-gnutls-dev \
#     libyaml-cpp-dev \
#     libjsoncpp-dev \
#     libpthread-stubs0-dev \
#     vim \
#     gdb \
#     && rm -rf /var/lib/apt/lists/*

# # 安装Python依赖
# RUN pip3 install --upgrade pip
# RUN pip3 install mysqlclient requests pycryptodome yamlcpp pandas scikit-learn==1.1.2 joblib==1.3.2

# # 设置工作目录
# WORKDIR /app

# # 复制源代码到工作目录
# COPY . /app

# # 编译C++程序
# RUN g++ -std=c++11 -pthread -o myapp main.cpp -lmysqlclient -lcurl -lcrypto++ -lyaml-cpp -ljsoncpp

# # 设置Python环境变量
# ENV PYTHONPATH "${PYTHONPATH}:/app"

# # 设置默认命令，这里使用Python脚本作为示例
# CMD ["python3.10", "app.py"]