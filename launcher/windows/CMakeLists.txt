cmake_minimum_required(VERSION 3.3.0)

add_library(proxy_loader SHARED proxyLoader.c)
set_property(TARGET proxy_loader PROPERTY OUTPUT_NAME "XInput9_1_0")