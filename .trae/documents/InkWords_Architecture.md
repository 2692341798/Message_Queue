# Message_Queue Architecture

## 目标

将仓库从原先以 `MessageQueues/` 为中心的学习记录结构，整理为更清晰的源码、测试、第三方依赖、数据与文档分层结构，同时保持原有消息队列实现逻辑可继续编译运行。

## 当前结构

- `src/common`：公共工具、日志、线程池、protobuf 协议文件与生成代码
- `src/server`：Broker、VirtualHost、Queue、Exchange、Binding、Connection、Channel 等核心服务端实现
- `src/client`：发布者与消费者客户端示例
- `tests/unit`：核心模块单元测试
- `tests/playground`：归档的实验代码与历史学习代码
- `third_party/muduo`：项目依赖的 muduo 头文件、protobuf codec 适配和静态库
- `data/dev`：服务端本地开发数据
- `docs/superpowers`：本次仓库整理的设计与计划文档

## 运行架构

服务端通过 muduo `TcpServer` 接收 protobuf 请求，由 `BrokerServer` 注册的分发器将不同请求转发给对应的 `Connection` 和 `Channel` 处理逻辑。

主要链路如下：

1. 客户端发送 protobuf 请求
2. `ProtobufCodec` 解码消息
3. `ProtobufDispatcher` 按消息类型路由
4. `BrokerServer` 通过连接管理器定位 `Connection`
5. `Connection` 通过信道编号定位 `Channel`
6. `Channel` 调用交换机、队列、绑定、消息存储等核心逻辑

## 数据持久化

- 元数据：`sqlite3`
- 消息体：文件存储（`*.message_data`）

## 本次整理涉及的工程变化

- 顶层目录重构
- 根 `Makefile` 统一构建入口
- README 中英文重写
- `.gitignore` 补充可执行文件、数据库和消息数据忽略规则
- 为 macOS 本地编译补齐依赖和兼容性修正

## 已知限制

- 仍以学习项目为主，未按生产级中间件标准设计高可用、恢复与监控能力
- 构建系统仍基于 Makefile，跨平台能力依赖本地环境配置
