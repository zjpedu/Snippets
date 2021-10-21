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
