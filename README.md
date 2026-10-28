# SIMCom Linux MPU / TBOX

本仓库是运行在 SIMCom Linux MPU 上的 TBOX 应用。

## 构建

在仓库根目录执行 `make`。`Makefile` 只编译 `src/main.c`，目标名 `demo_app`，输出为 `bin/demo_app`。`make clean` 删除 `bin/` 与 `obj/`。

头文件与链接库路径来自环境变量 `LIB_PATH`（本仓库的 `Makefile` 未给它赋值）。链接参数包含 `-lsdk`，搜索目录为 `$(LIB_PATH)/usr/lib`。可选：`SIMCOM_BLUETOOTH_FUNC=TRUE` 时增加蓝牙相关编译宏与库。

## 目录

| 路径 | 作用 |
| --- | --- |
| `src/main.c` | 入口。初始化后调用 `start_tbox()`，随后循环休眠 |
| `src/apps/start_tbox` | `start_tbox()` 所在目录；由 `src/src.c` → `src/apps/apps.c` 包含进 `main.c` |
| `src/apps/tcp_client` | TCP 客户端 |
| `src/apps/mcu` | 与 MCU 的通信 |
| `libs/libsdk.a` | 随仓库提供的 SIMCom SDK 静态库，需保持纳入版本管理 |
| `inc/` | 头文件（含 SDK 头文件） |

`src/start_tbox`、`src/tcp_client`、`src/mcu` 是并列副本，当前这条编译链不包含它们。

## 变更记录

版本说明见 [`src/readme.txt`](src/readme.txt)。
