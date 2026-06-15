# Message_Queue API 概览

## 概述

本项目没有提供 HTTP API，而是通过 `protobuf` 定义的二进制协议与 Broker 通信。
服务端入口位于 `src/server/server.cpp`，默认监听 `8085` 端口。

## 核心请求

- `openChannelRequest`：打开信道
- `closeChannelRequest`：关闭信道
- `declareExchangeRequest`：声明交换机
- `deleteExchangeRequest`：删除交换机
- `declareQueueRequest`：声明队列
- `deleteQueueRequest`：删除队列
- `queueBindRequest`：绑定队列与交换机
- `queueUnBindRequest`：解绑队列与交换机
- `basicPublishRequest`：发布消息
- `basicAckRequest`：确认消息
- `basicConsumeRequest`：订阅队列
- `basicCancelRequest`：取消订阅

以上请求定义位于 `src/common/request.proto`。

## 核心响应

- `basicConsumeResponse`：服务端向消费者推送消息
- `basicCommonResponse`：通用成功/失败响应

## 消息模型

`src/common/message.proto` 中定义了以下核心结构：

- `ExchangeType`：交换机类型
- `DeliveryMode`：消息投递模式
- `BasicProperties`：消息属性
- `Payload`：有效载荷
- `Message`：持久化消息记录

## 当前约束

- 接口协议以学习用途为主，缺少对版本演进和兼容性的额外设计。
- README 中描述的是使用方式；更底层的字段含义仍以 `.proto` 文件为准。
