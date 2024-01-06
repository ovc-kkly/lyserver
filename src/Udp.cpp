#include "Udp.h"

UdpReceiver::UdpReceiver(const std::string &ipAddress, int port)
    : ipAddress(ipAddress), port(port)
{
    // 创建 UDP 套接字
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1)
    {
        std::cerr << "Error: Failed to create socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    // int opt=SO_REUSEADDR;
    // setsockopt(udpSocket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    // fcntl(udpSocket, F_SETFL, O_NONBLOCK);
    // 设置地址结构
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    // 绑定套接字
    if (bind(udpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Error: Failed to bind socket." << std::endl;
        close(udpSocket);
        exit(EXIT_FAILURE);
    }else{
        std::cout<<"udp bind succeed"<<std::endl;
    }
}

UdpReceiver::~UdpReceiver()
{
    // 关闭套接字
    close(udpSocket);
}

void UdpReceiver::start()
{
    // 接收数据
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    ssize_t bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);

    if (bytesRead == -1)
    {
        std::cerr << "Error: Failed to receive data." << std::endl;
        return;
    }

    // 在这里处理接收到的数据
    // 你可以将 buffer 转换为图像或进行其他处理
    std::cout << "Received data from " << inet_ntoa(clientAddr.sin_addr) << ": " << buffer << std::endl;
}
int UdpReceiver::get_udp_listen()
{
    return udpSocket;
}