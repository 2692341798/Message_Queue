# Message_Queue 仓库深度整理设计

## 目标

将当前以学习记录为主的仓库整理为可阅读、可编译、可展示的项目仓库。
在不大规模改动核心源码逻辑的前提下，完成顶层目录重构、构建路径修正、文档升级和仓库清理。

## 设计原则

1. 保留核心模块职责，避免无必要的类名、命名空间和头文件名重命名。
2. 将“源码、测试、第三方依赖、实验代码、运行数据、文档”拆分为清晰的独立区域。
3. 优先修正路径和构建入口，不主动修改消息队列实现逻辑。
4. 保留 `TestCode`，但明确归档为实验/学习代码，不混入主链路。
5. 将运行数据与测试数据从源码目录剥离，并补充 `.gitignore` 防止二进制和数据文件继续进入版本库。

## 目标结构

```text
Message_Queue/
├── README.md
├── README.en.md
├── LICENSE
├── .gitignore
├── docs/
│   └── superpowers/
│       └── specs/
├── src/
│   ├── client/
│   ├── common/
│   └── server/
├── tests/
│   ├── data/
│   ├── playground/
│   └── unit/
├── third_party/
│   └── muduo/
├── data/
│   └── dev/
└── Makefile
```

## 模块映射

- `MessageQueues/MQCommon` -> `src/common`
- `MessageQueues/MQServer` -> `src/server`
- `MessageQueues/MQClient` -> `src/client`
- `MessageQueues/MQTest` -> `tests/unit`
- `MessageQueues/TestCode` -> `tests/playground`
- `MessageQueues/ThirdLib` -> `third_party/muduo`
- `MessageQueues/MQServer/data` -> `data/dev`
- `MessageQueues/MQTest/data` -> `tests/data`

## 构建策略

保留原有各模块下 `makefile` 的简单构建方式，但修正相对路径，使其适配新的目录结构。
同时在仓库根目录补充统一入口 `Makefile`，提供 `server`、`client`、`test`、`clean` 等常用命令，降低上手成本。
考虑到新版 protobuf / abseil 依赖要求，构建基线以 `C++17` 为准。

## 数据与产物处理

1. 保留示例数据，但移动到 `data/dev` 与 `tests/data`。
2. 仓库内已有可执行文件和数据库/消息数据文件不再放在源码目录中。
3. `.gitignore` 追加忽略规则，覆盖常见构建目录、测试临时数据和本地产物。

## README 改造范围

`README.md` 重写为中文主文档，包含项目简介、架构、目录结构、依赖、编译运行、测试方式和已知限制。
`README.en.md` 同步去除模板内容，改为简洁英文说明，与中文文档保持结构一致。

## 风险与控制

### 风险

1. 头文件大量使用相对路径，目录调整后容易出现包含失效。
2. `makefile` 中硬编码了旧目录结构与第三方库位置。
3. 测试和服务端依赖相对 `./data/` 路径，迁移后可能找不到数据目录。

### 控制措施

1. 优先整体迁移目录，再批量修正 `#include` 和 `makefile`。
2. 保持模块内部文件名不变，减少逻辑层变更。
3. 用统一根 `Makefile` 对新路径进行二次封装，避免用户感知旧路径细节。
4. 通过最少量编译验证检查服务端、客户端和测试目标是否仍可构建。
