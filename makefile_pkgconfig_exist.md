## Makefile 常见语法总结

### 概述

以下面故事为例说明 Makefile 的常见语法：

`分别在 MacOS(Darwin) Linux(Linux) 平台下判断 pkgconfig 管理工具是否存在,该工具能够在 make 过程中自动链接需要的依赖. 如果存在,则使用它检查需要的 arrow 和 parquet 是否存在!`

### 示例代码

* 判断 pkg-config 是否存在

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
* 如果 pkg-config 存在，则使用其检查 arrow 和 parquet 依赖是否存在

```shell
WITH_ARROW ?= check
WITH_PARQUET ?= check
ARROW_EXISTS := $(shell pkg-config --exists arrow && echo yes || echo no)
PARQUET_EXISTS := $(shell pkg-config --exists parquet && echo yes || echo no)

ifeq ($(ARROW_EXISTS), yes)
    override WITH_ARROW := yes
else
    override WITH_ARROW := no
endif

ifeq ($(PARQUET_EXISTS), yes)
    override WITH_PARQUET := yes
else
    override WITH_PARQUET := no
endif

ifeq ($(WITH_ARROW), yes)
    PG_CPPFLAGS := $(shell pkg-config --cflags arrow)
    SHLIB_LINK := $(shell pkg-config --libs arrow)
else
ifneq ($(MAKECMDGOALS),clean)
    $(error Could not find arrow)
endif
endif

ifeq ($(WITH_PARQUET), yes)
    PG_CPPFLAGS += $(shell pkg-config --cflags parquet)
    SHLIB_LINK += $(shell pkg-config --libs parquet)
else
ifneq ($(MAKECMDGOALS),clean)
    $(error Could not find parquet)
endif
endif
```

`ifneq ($(MAKECMDGOALS),clean)` 保证了在使用 `make clean` 时不做检查。

### 参考资料
https://github.com/Tinkerforge/brickd/blob/master/src/brickd/Makefile

