# JARVIS

`JARVIS`是一款辅助工具，使用 C/C++/Lua 三种语言进行开发，其中 Lua 主要实现业务逻辑，C/C++ 则用于第三方库的封装。

## `main.exe`

`main.exe` 主动调用 `script/main.lua`。

## `script/main.lua`

`script/main.lua` 根据`命令行参数1`，选择调用的入口脚本。

* 目前示例的入口脚本有 `script/game.lua` 和 `script/demo.lua`。

## Lua OpenCV

Lua OpenCV 将常用的 OpenCV 函数封装为 Lua 接口。

## Lua Async Task

由于 OpenCV 操作比较耗时，增加了Lua Async Task模块，进行多线程调用。
需要注意的是，该后台线程中的 Lua 虚拟机和主线程的 Lua 虚拟机不是同一个，并且没有初始化逻辑(即不会主动调用init)。

## 多协程系统

为了简化心智负担，`script/game.lua` 引入了多协程系统，用于各个模块间的解耦。
分为系统协程和用户协程。当切换命令时，会清除所有的用户协程，并执行新的命令的 `init` 函数。

## OCR

没有选用内置的 OCR 库，而是通过 HTTP 请求的方式请求独立的 OCR 服务。

