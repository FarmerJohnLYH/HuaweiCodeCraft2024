#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

// 定义相关常量和枚举
const int MAP_SIZE = 200;
const int ROBOT_COUNT = 10;
const int BOAT_COUNT = 5;
const int MAX_FRAMES = 15000;
const int MAX_ITEM_VALUE = 200;

enum class Direction { RIGHT, LEFT, UP, DOWN };
enum class RobotState { IDLE, MOVING, CARRYING };
enum class BoatState { IDLE, LOADING, TRAVELING };

// 定义结构体
struct Item {
    int x, y;
    int value;
};

struct Robot {
    int x, y;
    bool carryingItem;
    RobotState state;
};

struct Boat {
    int id;
    int targetBerthId;
    BoatState state;
    int cargoCount;
};

struct Berth {
    int id;
    int x, y;
    int transportTime;
    int loadingSpeed;
};

// 主要类，包含比赛逻辑
class TransportCompany {
public:
    void initialize(const std::vector<std::string>& mapData,
                    const std::vector<Berth>& berths,
                    int boatCapacity);

    void processFrame(int frameNum, int money,
                      const std::vector<Item>& newItems,
                      const std::vector<Robot>& robotsStatus,
                      const std::vector<Boat>& boatsStatus);

private:
    // 计算机器人和船只的行动指令
    void calculateCommands();

    // 机器人和船只指令执行相关函数
    void moveRobot(int robotId, Direction dir);
    void pickupItem(int robotId);
    void deliverItemToBerth(int robotId, int berthId);
    void moveBoatToBerth(int boatId, int berthId);
    void startTransport(int boatId);

    // 数据成员
    std::vector<std::string> mapData_;
    std::vector<Berth> berths_;
    std::vector<Robot> robots_;
    std::vector<Boat> boats_;
    int boatCapacity_;
    int currentMoney_;
};
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

// ...（省略之前的常量、枚举、结构体和类定义）

void TransportCompany::processFrame(int frameNum, int money,
                                    const std::vector<Item>& newItems,
                                    const std::vector<Robot>& robotsStatus,
                                    const std::vector<Boat>& boatsStatus) {
    currentMoney_ = money;

    // 更新机器人状态
    for (size_t i = 0; i < robots_.size(); ++i) {
        robots_[i].x = robotsStatus[i].x;
        robots_[i].y = robotsStatus[i].y;
        robots_[i].carryingItem = robotsStatus[i].carryingItem;
        robots_[i].state = robotsStatus[i].state;
    }

    // 更新船只状态
    for (size_t i = 0; i < boats_.size(); ++i) {
        boats_[i].state = boatsStatus[i].state;
        boats_[i].targetBerthId = boatsStatus[i].targetBerthId;
    }

    // 处理新生成的物品
    for (const auto& item : newItems) {
        // ...（处理新物品的逻辑，如分配给机器人、记录位置等）
    }

    // 计算机器人和船只的行动指令
    calculateCommands();

    // 输出控制指令
    for (const auto& command : robotCommands_) {
        std::cout << command.first << ' ' << command.second.first << ' ' << command.second.second << '\n';
    }
    for (const auto& command : boatCommands_) {
        std::cout << command.first << ' ' << command.second << '\n';
    }
    std::cout << "OK\n";
    std::flush(std::cout);  // 刷新输出以确保判题器及时收到数据
}


int main() {
    // 读取地图数据、泊位信息和船只容量
    std::vector<std::string> mapData(200);
    std::vector<Berth> berths;
    int boatCapacity;
    // ...（此处省略读取数据的代码）

    TransportCompany company;
    company.initialize(mapData, berths, boatCapacity);

    for (int frameNum = 1; frameNum <= MAX_FRAMES; ++frameNum) {

        // 读取当前帧信息
        int money;
        std::vector<Item> newItems;
        std::vector<Robot> robotsStatus;
        std::vector<Boat> boatsStatus;

        // 读取帧序号、当前金钱

        std::cin >> money;

        

        // 读取新增货物信息

        int gd_num;

        std::cin >> gd_num;

        for (int i = 0; i < gd_num; ++i) {

            int x, y, val;

            std::cin >> x >> y >> val;

            newItems.push_back({x, y, val});

        }


        // 读取机器人状态

        for (auto& robot : robotsStatus) {

            int hasItem, x, y, status;

            std::cin >> hasItem >> x >> y >> status;

            robot.hasItem = (hasItem == 1);

            robot.x = x;

            robot.y = y;

            robot.status = (status == 1 ? RobotState::MOVING : RobotState::IDLE);

        }


        // 读取船只状态

        for (auto& boat : boatsStatus) {

            int state, targetBerth;

            std::cin >> state >> targetBerth;

            boat.state = static_cast<BoatState>(state);

            boat.targetBerth = targetBerth;

        }


        // 读取判题器的“OK”确认信息

        std::string confirmation;

        std::getline(std::cin, confirmation);

        if (confirmation != "OK") {

            std::cerr << "Invalid confirmation from the judge. Exiting.\n";

            exit(1);

        }


        company.processFrame(frameNum, money, newItems, robotsStatus, boatsStatus);

    }

    return 0;
}


void TransportCompany::initialize(const std::vector<std::string>& mapData,

                                  const std::vector<Berth>& berths,

                                  int boatCapacity) {

    mapData_ = mapData;

    berths_ = berths;

    boatCapacity_ = boatCapacity;

    robots_.resize(ROBOT_COUNT);

    boats_.resize(BOAT_COUNT);

}


void TransportCompany::processFrame(int frameNum, int money,

                                    const std::vector<Item>& newItems,

                                    const std::vector<Robot>& robotsStatus,

                                    const std::vector<Boat>& boatsStatus) {

    currentMoney_ = money;


    // 更新机器人状态

    for (size_t i = 0; i < robots_.size(); ++i) {

        robots_[i].x = robotsStatus[i].x;

        robots_[i].y = robotsStatus[i].y;

        robots_[i].carryingItem = robotsStatus[i].carryingItem;

        robots_[i].state = robotsStatus[i].state;

    }


    // 更新船只状态

    for (size_t i = 0; i < boats_.size(); ++i) {

        boats_[i].state = boatsStatus[i].state;

        boats_[i].targetBerthId = boatsStatus[i].targetBerthId;

    }


    // 处理新生成的物品

    for (const auto& item : newItems) {

        // ...（处理新物品的逻辑，如分配给机器人、记录位置等）

    }


    // 计算机器人和船只的行动指令

    calculateCommands();


    // 输出控制指令

    for (const auto& command : robotCommands_) {

        std::cout << command.first << ' ' << command.second.first << ' ' << command.second.second << '\n';

    }

    for (const auto& command : boatCommands_) {

        std::cout << command.first << ' ' << command.second << '\n';

    }

    std::cout << "OK\n";

    std::flush(std::cout);  // 刷新输出以确保判题器及时收到数据

}
