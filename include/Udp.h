#pragma once
#include <string>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/fcntl.h>
class UdpReceiver {
public:
    UdpReceiver(const std::string& ipAddress, int port);
    ~UdpReceiver();

    void start();
    int get_udp_listen();
private:
    int udpSocket;
    std::string ipAddress;
    int port;
};