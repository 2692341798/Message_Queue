# Repository Restructure Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 重构仓库目录结构、修正构建路径并完善 README，使仓库更清晰且可继续编译使用。

**Architecture:** 保留现有消息队列实现逻辑，只重组顶层目录与模块归属。通过批量修正头文件相对引用、模块 `makefile` 与根 `Makefile`，让新结构继续兼容原有构建方式。

**Tech Stack:** C++17, Makefile, protobuf, muduo, sqlite3, gtest

---

### Task 1: 建立新目录骨架

**Files:**
- Create: `src/`
- Create: `tests/`
- Create: `third_party/`
- Create: `data/`
- Create: `docs/`

- [ ] **Step 1: 创建目标目录**

Run: `mkdir -p src/common src/server src/client tests/unit tests/playground tests/data third_party data/dev docs`
Expected: 目录创建成功

- [ ] **Step 2: 检查目录结构**

Run: `find . -maxdepth 2 -type d | sort`
Expected: 输出中包含 `src`、`tests`、`third_party`、`data`

### Task 2: 迁移核心模块与数据

**Files:**
- Modify: `src/common/**`
- Modify: `src/server/**`
- Modify: `src/client/**`
- Modify: `tests/unit/**`
- Modify: `tests/playground/**`
- Modify: `third_party/muduo/**`
- Modify: `data/dev/**`
- Modify: `tests/data/**`

- [ ] **Step 1: 迁移目录**

Run: `mv MessageQueues/MQCommon src/common && mv MessageQueues/MQServer src/server && mv MessageQueues/MQClient src/client && mv MessageQueues/MQTest tests/unit && mv MessageQueues/TestCode tests/playground && mv MessageQueues/ThirdLib third_party/muduo`
Expected: 旧模块迁移到新位置

- [ ] **Step 2: 迁移运行与测试数据**

Run: `mkdir -p data/dev tests/data && mv src/server/data/* data/dev/ && mv tests/unit/data/* tests/data/`
Expected: 数据文件从源码目录移出

### Task 3: 修正源码引用与 makefile

**Files:**
- Modify: `src/server/*.hpp`
- Modify: `src/client/*.hpp`
- Modify: `tests/unit/*.cpp`
- Modify: `src/server/makefile`
- Modify: `src/client/makefile`
- Modify: `tests/unit/makefile`

- [ ] **Step 1: 批量替换头文件相对引用**

Run: `python scripts/update_paths.py`
Expected: `../MQCommon`、`../MQServer`、`../ThirdLib` 等旧引用全部替换为新路径

- [ ] **Step 2: 修正模块 makefile**

Run: `grep -R "../MQCommon\\|../MQServer\\|../ThirdLib" -n src tests || true`
Expected: 不再出现旧目录引用

### Task 4: 增加统一构建入口与忽略规则

**Files:**
- Create: `Makefile`
- Modify: `.gitignore`

- [ ] **Step 1: 编写根 Makefile**

Code:

```makefile
.PHONY: server client test clean

server:
	$(MAKE) -C src/server

client:
	$(MAKE) -C src/client

test:
	$(MAKE) -C tests/unit

clean:
	$(MAKE) -C src/server clean || true
	$(MAKE) -C src/client clean || true
	$(MAKE) -C tests/unit clean || true
```

- [ ] **Step 2: 更新 `.gitignore`**

Code:

```gitignore
build/
data/dev/
tests/data/
*.db
*.message_data
server
PublichClient
ConsumeClient
Test_*
```

### Task 5: 重写 README

**Files:**
- Modify: `README.md`
- Modify: `README.en.md`

- [ ] **Step 1: 编写中文 README**

Expected sections: 项目简介、核心能力、架构设计、目录结构、依赖环境、构建运行、测试、playground 说明、后续优化

- [ ] **Step 2: 编写英文 README**

Expected sections: Overview, Features, Architecture, Structure, Dependencies, Build, Test, Playground, Limitations

### Task 6: 验证与清理

**Files:**
- Modify: `src/server/makefile`
- Modify: `src/client/makefile`
- Modify: `tests/unit/makefile`

- [ ] **Step 1: 运行服务端构建**

Run: `make server`
Expected: `src/server` 构建通过或仅暴露明确缺失依赖

- [ ] **Step 2: 运行客户端构建**

Run: `make client`
Expected: `src/client` 构建通过或仅暴露明确缺失依赖

- [ ] **Step 3: 运行测试构建**

Run: `make test`
Expected: `tests/unit` 构建通过或仅暴露明确缺失依赖

- [ ] **Step 4: 查看仓库状态**

Run: `git status --short`
Expected: 仅显示预期的迁移与文档变更
