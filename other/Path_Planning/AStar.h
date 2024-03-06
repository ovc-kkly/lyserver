#ifndef ASTAR_H
#define ASTAR_H

#include <vector>
#include <functional>
#include <iostream>
#include <map>
namespace AStar
{
    // 表示2D坐标的结构体
    struct Vec2i
    {
        int x, y; // x行，y列

        // 重载Vec2i的相等运算符
        bool operator==(const Vec2i &coordinates_);

        // 重载Vec2i的加法运算符
        friend Vec2i operator+(const AStar::Vec2i &left_, const AStar::Vec2i &right_)
        {
            return {left_.x + right_.x, left_.y + right_.y};
        }
    };

    using uint = unsigned int;
    using HeuristicFunction = std::function<uint(Vec2i, Vec2i)>;
    using CoordinateList = std::vector<Vec2i>;

    // 表示网格中一个点的节点结构体
    struct Node
    {
        // G 值表示起点到当前节点的路径实际代价, H 值表示从当前节点到目标节点的估计代价
        uint G, H;
        // 当前结点的坐标
        Vec2i coordinates;
        Node *parent;

        // Node的构造函数
        Node(Vec2i coord_, Node *parent_ = nullptr);

        // 计算节点的总分数（F）
        uint getScore();
    };
    struct CompareNodes
    {
        bool operator()(Node *left, Node *right)
        {
            return (left->getScore() < right->getScore());
        }
    };

    using NodeSet = std::vector<Node *>;

    // AStar路径规划算法生成器
    class Generator
    {
        bool detectCollision(Vec2i coordinates_);
        Node *findNodeOnList(NodeSet &nodes_, Vec2i coordinates_);
        void releaseNodes(NodeSet &nodes_);

    public:
        // 生成器的构造函数
        Generator();

        // 设置算法的世界大小
        void setWorldSize(Vec2i worldSize_);

        // 启用或禁用算法中的对角线移动
        void setDiagonalMovement(bool enable_);

        // 设置算法的启发函数
        void setHeuristic(HeuristicFunction heuristic_);

        // 从源点到目标点找到路径
        CoordinateList findPath(Vec2i source_, Vec2i target_);

        // 在指定坐标添加障碍物
        void addCollision(Vec2i coordinates_);
        // 在指定坐标添加车位
        void addStall(Vec2i coordinates_);
        // 在指定坐标添加充电车位
        void addcharging_pile(Vec2i coordinates_);
        // 在指定坐标移除碰撞
        void removeCollision(Vec2i coordinates_);

        // 清除所有碰撞
        void clearCollisions();
        //初始化地图
        void init_map();
        //输出路径
        void print_path(AStar::CoordinateList& path);
        CoordinateList findbestpath(Vec2i startpoint, std::vector<Vec2i> endpointarray);
    private:
        HeuristicFunction heuristic;
        CoordinateList direction, walls, stall, charging_pile;
        Vec2i worldSize;
        // std::vector<std::vector<Vec2i>> worldSize;
        uint directions;
        std::vector<std::vector<int>> world;
        // Vec2i source_,target_;
        std::vector<CoordinateList> allpathlist;
    };

    // 启发式类，提供不同的启发函数
    class Heuristic
    {
        // 获取源点和目标点之间坐标的差异
        static Vec2i getDelta(Vec2i source_, Vec2i target_);

    public:
        // 曼哈顿启发函数
        static uint manhattan(Vec2i source_, Vec2i target_);

        // 欧几里得启发函数
        static uint euclidean(Vec2i source_, Vec2i target_);

        // 八方向启发函数
        static uint octagonal(Vec2i source_, Vec2i target_);
    };
}

#endif 
