## NUMA Test Suite

This suite contains a set of simple programs to test numa interfaces provided by libnuma.

## Prerequisites

Make sure you have `numactl`, if you don't, do

`sudo yum install numactl`

`sudo yum install numactl-devel`

or use other package package management tool for your platform.

## Check NUMA Support

To check if your machine has NUMA, do

`numactl --hardware`

The first line shows the number of NUMA nodes in the system. It needs to be more than one node.

Alternatively,

`sudo ls /sys/devices/system/node/`

Directory `node0` represents node 0, 'node1' for node 1, etc. If you only have `node0`, then there is only one node.

Alternatively, write a simple program to call `numa_available()`, if it returns -1, all other functions in this library are undefined.

## Overview

The Linux kernel will use the INTERLEAVE policy by default on boot-up to avoid putting excessive load on a single memory node when processes require access to the operating-system structures. The system default policy is changed to NODE LOCAL when the first userspace process (init daemon) is started.

*Node* is frequently used in the document, more formally, a *node* is defined as an area where all memory has the same speed as seen from a particular CPU. A node can contain multiple CPUs. Caches are ignored for this definition.

A good [overview](http://queue.acm.org/detail.cfm?id=2513149)

## Build & Test

`cd make/; make all`

To make a single test case, do `make move-pages`, where `move-pages` is the test case that tests *move_pages()*. Cat the makefile to see all the targets.

To run the test, do `cd ../build/; ./move-pages --help`.

## Monitor & Verify

You may use `pcm-memory.x` from [Intel PCM](https://github.com/opcm/pcm) to verify if you are really using a NUMA node. Caveat: create enough memory bandwidth to make monitoring obvious. 

