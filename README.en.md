# Message_Queue

`Message_Queue` is a learning-oriented message queue project inspired by AMQP concepts. It focuses on implementing the core workflow of a broker, including exchanges, queues, bindings, publishing, acknowledgements, subscriptions, and connection/channel management.

## Overview

This repository is better understood as a system design and infrastructure practice project rather than a production-ready middleware.  
The implementation uses `protobuf` for request/message definitions and `muduo` for network communication.

Project blog:
[Project introduction](https://blog.csdn.net/ZHENGZJM/article/details/147812975?fromshare=blogdetail&sharetype=blogdetail&sharerId=147812975&sharerefer=PC&sharesource=ZHENGZJM&sharefrom=from_link)

## Features

- Basic exchange, queue, and binding management
- Message publishing, acknowledgement, consuming, and canceling
- Connection, channel, and consumer management
- Partial persistence with `sqlite3` and message data files
- Demo client programs and unit tests

## Repository Structure

```text
Message_Queue/
├── src/
│   ├── common/        # shared utilities, protocol definitions, thread pool
│   ├── server/        # broker server implementation
│   └── client/        # publish/consume demo clients
├── tests/
│   ├── unit/          # unit tests for core modules
│   ├── playground/    # archived experimental and learning code
│   └── data/          # test data used by unit tests
├── third_party/
│   └── muduo/         # third-party headers / references
├── data/
│   └── dev/           # development runtime data for the server
├── docs/
├── Makefile
├── README.md
└── README.en.md
```

## Modules

### `src/common`

Shared infrastructure and protocol-related files:

- `Helper.hpp`
- `Logger.hpp`
- `ThreadPool.hpp`
- `message.proto` / `request.proto`
- generated protobuf files

### `src/server`

Core broker-side implementation:

- `Broker.hpp`
- `Connection.hpp`
- `Channel.hpp`
- `Exchange.hpp`
- `Queue.hpp`
- `Binding.hpp`
- `Message.hpp`
- `VirtualHost.hpp`
- `server.cpp`

### `src/client`

Client demo programs:

- `PublichClient.cpp`
- `ConsumeClient.cpp`

### `tests/unit`

Unit tests for queue, exchange, binding, channel, connection, consumer, and related modules.

### `tests/playground`

Archived experimental code. These files are kept for study and reference, but they are not part of the main project path.

## Dependencies

Linux remains the recommended build environment; this repository now also includes the compatibility adjustments required to compile successfully on macOS.

- Linux
- `g++` with C++17 support
- `make`
- `protobuf`
- `sqlite3`
- `pthread`
- `zlib`
- `muduo`
- `gtest`

Notes:

1. `third_party/muduo` stores the project-specific muduo headers, protobuf codec compatibility patches, and local static library layout.
2. The macOS validation path uses Homebrew packages such as `protobuf`, `boost`, `sqlite3`, `googletest`, and `zlib`, together with `/opt/homebrew` include/library paths.
3. If linking fails because `muduo`, `protobuf`, or `gtest` libraries are missing, install the corresponding development packages locally or adjust the library paths in the `makefile`s.

## Build

Use the root-level commands:

```bash
make server
make client
make test
make clean
```

Or build each module directly:

```bash
make -C src/server
make -C src/client
make -C tests/unit
```

## Run

Build and start the server:

```bash
make server
./src/server/server
```

The server uses `data/dev/` as the default runtime data directory.

Build and run the client demos:

```bash
make client
./src/client/PublichClient
./src/client/ConsumeClient
```

## Test

Build the unit tests:

```bash
make test
```

Test data is stored in `tests/data/`.

## Project Documents

The repository also keeps the project-level documents needed for future maintenance:

- `docs/superpowers/`

## Cleanup Summary

This repository cleanup includes:

- moving old modules into `src`, `tests`, `third_party`, and `data`
- archiving experimental code into `tests/playground`
- moving runtime/test data out of source folders
- removing committed binaries and improving `.gitignore`
- adding a root `Makefile`
- rewriting both READMEs

## Limitations

- This is still a learning-oriented implementation
- The build system is based on simple `makefile`s
- Third-party library linkage may require local environment setup
- Archived playground code is intentionally preserved and not fully normalized
