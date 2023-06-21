# DNSRelay

A DNS Relay Server Implementation with C

### 文件结构

本项目的主要文件结构如下

```
dns_relay/
|-- include/         # 存放头文件
|-- src/             # 存放源代码文件
|   |-- common/      # 存放跨平台通用代码
|   |-- linux/       # 存放Linux平台特定代码
|   |-- windows/     # 存放Windows平台特定代码
|-- build/           # 存放编译后的二进制文件
|-- Makefile         # 编译脚本
|-- README.md        # 项目说明文档
```
