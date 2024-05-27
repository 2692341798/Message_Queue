# Install script for directory: /home/myroot/linux/lib/muduo-master/examples

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

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/ace/ttcp/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/asio/chat/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/asio/tutorial/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/fastcgi/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/filetransfer/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/hub/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/idleconnection/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/maxconnection/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/memcached/client/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/memcached/server/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/multiplexer/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/netty/discard/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/netty/echo/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/netty/uptime/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/pingpong/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/roundtrip/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/shorturl/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/simple/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/socks4a/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/sudoku/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/twisted/finger/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/wordcount/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/zeromq/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/ace/logging/cmake_install.cmake")
  INCLUDE("/home/myroot/linux/lib/build/release-cpp11/examples/protobuf/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

