## 在 Makefile 文件中判断 pkgconfig 依赖是否存在

### 概述

分别在 MacOS(Darwin) Linux(Linux) 平台下判断 pkgconfig 管理工具是否存在，该工具能够在 make 过程中自动链接需要的依赖。

### 代码

```shell
WITH_HOST ?= check
WITH_TARGET ?= check

ifeq ($(WITH_HOST),check)
ifeq ($(OS),Windows_NT)
	override WITH_HOST := Windows
else
	override WITH_HOST := $(shell uname)
endif
endif

ifeq ($(WITH_HOST),Windows)
else
ifneq ($(WITH_HOST),Linux)
ifneq ($(WITH_HOST),Darwin)
$(error $(WITH_HOST) is not supported as host)
endif
endif
endif

ifeq ($(WITH_TARGET),check)
	override WITH_TARGET := $(WITH_HOST)
endif



ifeq ($(WITH_TARGET), Darwin)
	PKG_CONFIG := $(shell which pkg-config 2>/dev/null)

ifeq ($(PKG_CONFIG),)
ifneq ($(MAKECMDGOALS),clean)
$(error Could not find pkg-config)
endif
endif
endif

ifeq ($(WITH_TARGET), Linux)
	PKG_CONFIG := $(shell which pkg-config 2>/dev/null)

ifeq ($(PKG_CONFIG),)
ifneq ($(MAKECMDGOALS),clean)
$(error Could not find pkg-config)
endif
endif
endif
```
