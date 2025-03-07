cmake_minimum_required(VERSION 3.10.0)
project(Chat VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Chat chat_server.cpp chat_client.cpp)

