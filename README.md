# Message_Queue

一个基于 AMQP 设计思路实现的简易消息队列学习项目，目标是梳理消息队列的核心组件与基本工作流，包括交换机、队列、绑定、发布、确认、订阅和连接/信道管理。

## 项目概览

这个仓库更适合作为消息队列底层机制的练习型实现，而不是生产级中间件。  
核心逻辑围绕 broker、virtual host、exchange、queue、message persistence 和 client/server 通信展开，使用 `protobuf` 组织协议消息，使用 `muduo` 完成网络通信。

项目介绍博客：
[消息队列项目介绍](https://blog.csdn.net/ZHENGZJM/article/details/147812975?fromshare=blogdetail&sharetype=blogdetail&sharerId=147812975&sharerefer=PC&sharesource=ZHENGZJM&sharefrom=from_link)

## 主要能力

- 基于自定义协议请求实现交换机、队列、绑定等基础操作
- 支持消息发布、确认、消费、取消订阅
- 维护连接、信道与消费者管理
- 使用 `sqlite3` 和文件存储保存部分元数据与消息数据
- 提供基础客户端示例与单元测试代码

## 目录结构

```text
Message_Queue/
├── src/
│   ├── common/        # 公共组件、协议定义、线程池、日志、辅助类
│   ├── server/        # Broker 服务端实现
│   └── client/        # 发布/消费客户端示例
├── tests/
│   ├── unit/          # 核心模块单元测试
│   ├── playground/    # 保留的实验/学习代码归档
│   └── data/          # 单元测试依赖的数据
├── third_party/
│   └── muduo/         # 第三方头文件/参考代码
├── data/
│   └── dev/           # 服务端开发演示数据
├── docs/
│   └── superpowers/   # 本次整理生成的设计与计划文档
├── Makefile           # 统一构建入口
├── README.md
└── README.en.md
```

## 模块说明

### `src/common`

公共基础模块，包含：

- `Helper.hpp`：文件与路径等辅助能力
- `Logger.hpp`：日志封装
- `ThreadPool.hpp`：线程池
- `message.proto` / `request.proto`：协议定义
- `*.pb.cc` / `*.pb.h`：协议编译产物

### `src/server`

服务端核心实现，主要包括：

- `Broker.hpp`：Broker 服务入口和请求分发
- `Connection.hpp` / `Channel.hpp`：连接与信道管理
- `Exchange.hpp` / `Queue.hpp` / `Binding.hpp`：交换机、队列、绑定管理
- `Message.hpp`：消息存储和读取
- `VirtualHost.hpp`：虚拟主机与整体资源聚合
- `server.cpp`：服务端启动入口

### `src/client`

客户端演示代码，包含发布和消费两个示例程序：

- `PublichClient.cpp`
- `ConsumeClient.cpp`

### `tests/unit`

针对队列、交换机、绑定、信道、连接、消费者等模块的单元测试。

### `tests/playground`

归档的实验代码与学习代码，不属于主链路实现。这里保留了对日志、随机数、`gtest`、`protobuf`、`sqlite`、`muduo` 等内容的练习示例，方便回顾，但不建议把这里的结构当作主项目架构参考。

## 依赖环境

推荐优先在 Linux 环境下构建；当前仓库已经补齐了在 macOS 上编译通过所需的兼容调整。至少准备：

- `g++`，支持 `C++17`
- `make`
- `protobuf`
- `sqlite3`
- `pthread`
- `zlib`
- `muduo`
- `gtest`

说明：

1. 仓库中的 `third_party/muduo` 保留了项目依赖的头文件、协议编解码适配和本地静态库位置。
2. macOS 本地验证使用 Homebrew 安装 `protobuf`、`boost`、`sqlite3`、`googletest`、`zlib`，并配合 `/opt/homebrew` 路径完成编译。
3. 如果链接阶段提示找不到 `muduo`、`protobuf` 或 `gtest` 相关库，需要先在本机安装对应开发库，或按你的本地环境调整各模块 `makefile` 中的库路径。

## 构建方式

仓库根目录提供了统一入口：

```bash
make server
make client
make test
make clean
```

也可以分别进入模块目录构建：

```bash
make -C src/server
make -C src/client
make -C tests/unit
```

## 运行方式

### 启动服务端

在仓库根目录执行：

```bash
make server
./src/server/server
```

服务端默认使用 `data/dev/` 作为运行数据目录。

### 运行客户端

先构建客户端：

```bash
make client
```

然后在 `src/client/` 下运行相应程序：

```bash
./src/client/PublichClient
./src/client/ConsumeClient
```

## 测试

构建单元测试：

```bash
make test
```

测试依赖的数据位于 `tests/data/`。  
如果你调整了运行目录或手动执行某个测试程序，请确认相对路径仍指向该目录。

## 工程文档

仓库内同步维护了本次整理所需的项目文档，便于后续继续迭代：

- `.trae/documents/InkWords_API.md`
- `.trae/documents/InkWords_Architecture.md`
- `.trae/documents/InkWords_Conversation_Log.md`
- `.trae/documents/InkWords_Database.md`
- `.trae/documents/InkWords_Development_Plan_and_Log.md`
- `.trae/documents/InkWords_PRD.md`
- `docs/superpowers/`

## 仓库整理说明

这次整理主要做了几件事：

- 将原先集中在 `MessageQueues/` 下的内容拆分到 `src`、`tests`、`third_party`、`data`
- 将实验代码单独归档到 `tests/playground`
- 将服务端数据和测试数据从源码目录迁出
- 删除仓库内已有编译产物，补充 `.gitignore`
- 增加根 `Makefile`
- 重写中英文 README，降低阅读门槛

## 当前限制

- 当前实现更偏学习性质，未覆盖生产环境下的完整可靠性要求
- 构建系统仍以简单 `makefile` 为主，跨平台兼容性较弱
- 第三方库路径依赖本地环境，可能需要手动调整
- `tests/playground` 中的代码风格和结构不完全统一，它的目标是归档，不是主工程标准

## 后续可继续优化

- 用 `CMake` 替代分散的 `makefile`
- 为服务端和客户端提供统一配置文件
- 增加完整的启动示例和消息流示意图
- 清理 `PublichClient` 等历史命名，统一代码风格
