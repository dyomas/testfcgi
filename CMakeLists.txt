cmake_minimum_required (VERSION 2.8.4 FATAL_ERROR)

project(daemon)


include(ExternalProject)
ExternalProject_Add(stemka
  URL "http://www.keva.ru/stemka/stemka.tar.gz"
  URL_MD5 "a90af9bc2f4b78176317ea3961981687"
  TIMEOUT 15
  CMAKE_COMMAND ""
  BINARY_DIR ${CMAKE_BINARY_DIR}/stemka-prefix/src/stemka
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_COMMAND make CC=g++
)


add_executable(daemon
  src/buffered_sender.cpp
  src/connection.cpp
  src/daemon.cpp
  src/echo.cpp
  src/fastcgi_enums.cpp
  src/fastcgi_request.cpp
  src/fastcgi_response.cpp
  src/main.cpp
  src/morph_parser.cpp
  src/libev_wrapper.cpp
  src/searcher.cpp
  src/send_buffer_trivial.cpp
  src/socket_async_base.cpp
  src/socket_async_tcp_server.cpp
  src/storage.cpp
)
target_link_libraries (daemon -lev ${CMAKE_BINARY_DIR}/stemka-prefix/src/stemka/libstemka.a)
set_target_properties (daemon PROPERTIES
  COMPILE_FLAGS "-I${CMAKE_BINARY_DIR}/stemka-prefix/src/stemka/include"
)
add_dependencies(daemon stemka)
