cmake_minimum_required (VERSION 2.8.4 FATAL_ERROR)

project(daemon)

add_executable(daemon
  src/buffered_sender.cpp
  src/connection.cpp
  src/daemon.cpp
  src/echo.cpp
  src/fastcgi_enums.cpp
  src/fastcgi_request.cpp
  src/fastcgi_response.cpp
  src/main.cpp
  src/libev_wrapper.cpp
  src/searcher.cpp
  src/send_buffer_trivial.cpp
  src/socket_async_base.cpp
  src/socket_async_tcp_server.cpp
  src/storage.cpp
)
target_link_libraries (daemon -lev)
