# Install script for directory: /home/myroot/linux/lib/muduo-master/muduo/net

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/home/myroot/linux/lib/build/release-install-cpp11")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/myroot/linux/lib/build/release-cpp11/lib/libmuduo_net.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/muduo/net" TYPE FILE FILES
    "/home/myroot/linux/lib/muduo-master/muduo/net/Buffer.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/Callbacks.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/Channel.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/Endian.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/EventLoop.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/EventLoopThread.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/EventLoopThreadPool.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/InetAddress.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/TcpClient.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/TcpConnection.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/TcpServer.h"
    "/home/myroot/linux/lib/muduo-master/muduo/net/TimerId.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/muduo/net/http/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/muduo/net/inspect/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/muduo/net/tests/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/muduo/net/protobuf/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/muduo/net/protorpc/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

