#include "AStar.h"
#include <algorithm>
#include <math.h>
namespace AStar
{
    using namespace std::placeholders;

    // Vec2i结构体的相等运算符重载
    bool AStar::Vec2i::operator==(const Vec2i &coordinates_)
    {
        return (x == coordinates_.x && y == coordinates_.y);
    }

    // Node结构体的构造函数
    AStar::Node::Node(Vec2i coordinates_, Node *parent_)
    {
        parent = parent_;
        coordinates = coordinates_;
        G = H = 0;
    }

    // 计算节点的总分数（F）
    uint AStar::Node::getScore()
    {
        return G + H;
    }

    // Generator类的构造函数
    AStar::Generator::Generator()
    {
        /*8*40                                                          31
    world = {0  1, 2, 3, 4, 5, 6, 7, 8, 9, 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39
            {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
            {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
            {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1, 1, 1, 1, 1},
            {0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}};

    */
        world = {
            {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
            {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
            {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1, 1, 1, 1, 1},
            {0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}};
        // world = {
        //     {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},
        //     {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
        //     {0, 1, 1, 1, 1, 1, 1, 1, 0, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1, 1, 1, 1},
        //     {0, 2, 2, 2, 2, 2, 2, 2, 0, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 1},
        //     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        //     {2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 3, 1, 3, 3, 3, 3, 0, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0},
        //     {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        //     {2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 3, 1, 3, 3, 3, 3, 0, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0},
        //     {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 3, 1, 3, 1, 2, 2, 2, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1},
        //     {2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 1, 3, 1, 0, 0, 0, 0, 0, 3, 1, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0},
        //     {2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 3, 1, 3, 3, 3, 3, 0, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 0}
        // };
        setDiagonalMovement(false);
        setHeuristic(&Heuristic::manhattan);
        direction = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {-1, -1}, {1, 1}, {-1, 1}, {1, -1}}; // 0-3是上下左右，其他使对角线
    }
    void AStar::Generator::init_map()
    {
        // 根据地图内容添加障碍物、车位和充电车位
        for (int i = 0; i < (int)world.size(); ++i)
        {
            for (int j = 0; j < (int)world[i].size(); ++j)
            {
                AStar::Vec2i coordinates{i, j}; // 注意这里的坐标顺序
                int value = world[i][j];

                if (value == 1)
                {
                    // 添加障碍物
                    this->addCollision(coordinates);
                }
                else if (value == 2)
                {
                    // 添加车位
                    this->addStall(coordinates);
                }
                else if (value == 3)
                {
                    // 添加充电车位
                    this->addcharging_pile(coordinates);
                }
            }
        }
    }
    // 设置算法的世界大小
    void AStar::Generator::setWorldSize(Vec2i worldSize_)
    {
        worldSize = worldSize_;
    }

    // 启用或禁用算法中的对角线移动
    void AStar::Generator::setDiagonalMovement(bool enable_)
    {
        directions = (enable_ ? 8 : 4);
    }

    // 设置算法的启发函数
    void AStar::Generator::setHeuristic(HeuristicFunction heuristic_)
    {
        heuristic = std::bind(heuristic_, _1, _2);
    }

    // 在指定坐标添加障碍物
    void AStar::Generator::addCollision(Vec2i coordinates_)
    {
        walls.push_back(coordinates_);
    }
    void AStar::Generator::addStall(Vec2i coordinates_)
    {
        stall.push_back(coordinates_);
    }
    void AStar::Generator::addcharging_pile(Vec2i coordinates_)
    {
        charging_pile.push_back(coordinates_);
    }

    // 在指定坐标移除碰撞
    void AStar::Generator::removeCollision(Vec2i coordinates_)
    {
        auto it = std::find(walls.begin(), walls.end(), coordinates_);
        if (it != walls.end())
        {
            walls.erase(it);
        }
    }

    // 清除所有碰撞
    void AStar::Generator::clearCollisions()
    {
        walls.clear();
    }

    // 从源点到目标点找到路径
    AStar::CoordinateList AStar::Generator::findPath(Vec2i source_, Vec2i target_)
    {
        // this->source_ = source_;
        // this->target_ = target_;
        // 表示目前结点
        Node *current = nullptr;
        Vec2i endpoint;
        // openSet 用于存储待探索的节点，而 closedSet 存储已经探索过的节点
        NodeSet openSet;
        NodeSet closedSet;
        openSet.reserve(100);
        closedSet.reserve(100);
        openSet.push_back(new Node(source_));

        while (!openSet.empty())
        {
            auto current_it = openSet.begin();
            current = *current_it;

            for (auto it = openSet.begin(); it != openSet.end(); it++)
            { // 把代价最小的结点作为当前结点
                auto node = *it;
                if (node->getScore() <= current->getScore())
                {
                    current = node;
                    current_it = it;
                }
            }
            Vec2i up{target_.x - 1, target_.y};
            Vec2i down{target_.x + 1, target_.y};
            if (current->coordinates == up)
            {
                // 找到了目的地，此时的车位置在目标位置的下边
                endpoint = {up.x + 1, up.y};
                break;
            }
            else if (current->coordinates == down)
            {
                //找到了目的地，此时的车位置在目标位置的上边
                endpoint = {down.x - 1, down.y};
                break;
            }

            closedSet.push_back(current); // 当前结点已经探测过了
            openSet.erase(current_it);

            for (uint i = 0; i < directions; ++i) // directions方向4或8
            {
                Vec2i newCoordinates(current->coordinates + direction[i]);                        // 取出当前结点的一个邻居结点
                if (detectCollision(newCoordinates) || findNodeOnList(closedSet, newCoordinates)) // 找是否超出边界，且是否已经被探测过
                {
                    continue;
                }

                uint totalCost = current->G + ((i < 4) ? 10 : 14); // 如果邻居节点在当前节点的上、下、左、右位置，移动的代价为10；如果邻居节点在当前节点的斜对角位置，移动的代价为14。

                Node *successor = findNodeOnList(openSet, newCoordinates); // 表示在待探测集合中查找邻居是否在里面
                if (successor == nullptr)                                  // 邻居不在待探测集合中，加入
                {
                    successor = new Node(newCoordinates, current);
                    successor->G = totalCost;//已经走过的代价
                    successor->H = heuristic(successor->coordinates, target_);//剩下的估计代价
                    openSet.push_back(successor);
                }
                else if (totalCost < successor->G)
                {
                    successor->parent = current;
                    successor->G = totalCost;
                }
            }
        }

        CoordinateList path;
        path.push_back(endpoint);
        while (current != nullptr)
        {
            path.push_back(current->coordinates);
            current = current->parent;
        }

        releaseNodes(openSet);
        releaseNodes(closedSet);

        return path;
    }
    CoordinateList Generator::findbestpath(Vec2i startpoint, std::vector<Vec2i> endpointarray)
    {
        std::map<int, CoordinateList> score_pathlist;
        for (Vec2i &Endpath : endpointarray)
        {
            CoordinateList path = findPath(startpoint, Endpath);
            int scorepath = path.size();
            score_pathlist.insert(std::pair<int, CoordinateList>(scorepath, path));
        }
        return score_pathlist.begin()->second;
    }

    // 在集合中查找是否存在
    AStar::Node *AStar::Generator::findNodeOnList(NodeSet &nodes_, Vec2i coordinates_)
    {
        for (auto node : nodes_)
        {
            if (node->coordinates == coordinates_)
            {
                return node;
            }
        }
        return nullptr;
    }

    // 释放节点集合中的节点内存
    void AStar::Generator::releaseNodes(NodeSet &nodes_)
    {
        for (auto it = nodes_.begin(); it != nodes_.end();)
        {
            delete *it;
            it = nodes_.erase(it);
        }
    }

    // 检测指定坐标是否有障碍物、车位和充电车位，还有是否超出边界
    bool AStar::Generator::detectCollision(Vec2i coordinates_)
    {
        if (coordinates_.x < 0 || coordinates_.x >= worldSize.x ||
            coordinates_.y < 0 || coordinates_.y >= worldSize.y ||
            std::find(walls.begin(), walls.end(), coordinates_) != walls.end() ||
            std::find(stall.begin(), stall.end(), coordinates_) != stall.end() ||
            std::find(charging_pile.begin(), charging_pile.end(), coordinates_) != charging_pile.end())
        {
            return true;
        }
        return false;
    }
    void AStar::Generator::print_path(AStar::CoordinateList &path)
    {
        for (auto it = path.rbegin(); it != path.rend(); ++it)
        {
            if (it == path.rend() - 1)
            {
                std::cout << "(" << it->x << "," << it->y << ")\n";
            }
            else
            {
                std::cout << "(" << it->x << "," << it->y << ")->";
            }
        }
    }

    // 启发式类的getDelta函数，获取源点和目标点之间坐标的差异
    AStar::Vec2i AStar::Heuristic::getDelta(Vec2i source_, Vec2i target_)
    {
        return {abs(source_.x - target_.x), abs(source_.y - target_.y)};
    }

    // 曼哈顿启发函数
    AStar::uint AStar::Heuristic::manhattan(Vec2i source_, Vec2i target_)
    {
        auto delta = std::move(getDelta(source_, target_));
        return static_cast<uint>(10 * (delta.x + delta.y));
    }

    // 欧几里得启发函数
    AStar::uint AStar::Heuristic::euclidean(Vec2i source_, Vec2i target_)
    {
        auto delta = std::move(getDelta(source_, target_));
        return static_cast<uint>(10 * sqrt(pow(delta.x, 2) + pow(delta.y, 2)));
    }

    // 八方向启发函数
    AStar::uint AStar::Heuristic::octagonal(Vec2i source_, Vec2i target_)
    {
        auto delta = std::move(getDelta(source_, target_));
        return 10 * (delta.x + delta.y) + (-6) * std::min(delta.x, delta.y);
    }
}
