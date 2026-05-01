# mir2x 使用说明（macOS 客户端 + Ubuntu 服务器）

本文档对应你当前的本机构建。所有路径都是绝对路径，可以直接复制粘贴。

---

## 1. 现状：已经做完的事

- ✅ 在你这台 Mac（Apple Silicon，Apple Clang 17）上**编译出客户端**
- ✅ 资源数据 `res/` 已通过 git-lfs 拉取（约 1.8 GB）
- ✅ 写好启动脚本 `run-client.sh`
- ✅ **服务器也在 Mac 上编出来了**（用 brew LLVM clang 22，Apple Clang 17 撞模板崩溃）
- ✅ 写好服务器启动脚本 `run-server.sh`（含目录链接 + 端口绕开 AirPlay）
- ⚠️ 客户端用 `--auto-login` 自动登入会崩在 font id=11（数据 res 缺新字体），手动走登录界面应该没问题；详见第 10 节

构建产物位置：

| 文件 | 路径 |
|---|---|
| 客户端二进制 | `/Users/tetsuya/Development/mir2x/b/client/src/client` |
| 服务器二进制 | `/Users/tetsuya/Development/mir2x/b/server/src/server` |
| 客户端启动脚本 | `/Users/tetsuya/Development/mir2x/b/run-client.sh` |
| 服务器启动脚本 | `/Users/tetsuya/Development/mir2x/b/run-server.sh` |
| 服务器运行目录（symlink + db） | `/Users/tetsuya/Development/mir2x/b/server/run/` |
| 服务器 sqlite 数据库 | `/Users/tetsuya/Development/mir2x/b/server/run/mir2x.db3` |
| 游戏资源（贴图/地图/音效/字体） | `/Users/tetsuya/Development/mir2x/b/3rdparty/mir2x_data/res/` |
| 服务器 lua 脚本 | `/Users/tetsuya/Development/mir2x/server/script/` |
| 源码根目录 | `/Users/tetsuya/Development/mir2x/` |
| 构建目录 | `/Users/tetsuya/Development/mir2x/b/` |
| 客户端日志（运行后） | `/tmp/mir2x-client-v01.g3log.<时间戳>.log` |
| 服务器日志（运行后） | `/tmp/mir2x-server-v01.g3log.<时间戳>.log` |

---

## 2. 怎么启动客户端

### 2.1 前置条件

客户端必须连一个**正在运行的服务器**才能登录。两种情况：

- 你已经在 Ubuntu 机上把服务器跑起来了 → 知道服务器 IP（比如 `192.168.1.10`）
- 暂时还没服务器 → 客户端登录界面打不开人物，但能看到启动画面，至少能验证编译没问题

### 2.2 最简启动

```bash
# 连本机 127.0.0.1（如果服务器跑在同一台 Mac 上）
/Users/tetsuya/Development/mir2x/b/run-client.sh

# 连远程 Ubuntu 服务器
/Users/tetsuya/Development/mir2x/b/run-client.sh 192.168.1.10
```

### 2.3 推荐启动（关掉 macOS 上有点吵的功能）

```bash
/Users/tetsuya/Development/mir2x/b/run-client.sh 192.168.1.10 \
    --disable-audio --disable-ime
```

为什么关 IME：客户端用 libpinyin 做中文输入，macOS 上数据路径找不全会刷错误；不打字交流就先关掉。
为什么关 audio：SDL2_mixer 在 macOS 上偶尔会有杂音/丢帧，先关掉容易调通。

### 2.4 自动登录（开发调试常用）

```bash
/Users/tetsuya/Development/mir2x/b/run-client.sh 192.168.1.10 \
    --disable-audio --disable-ime \
    --auto-login=test:123456
```

`test:123456` 替换成你在服务器数据库里建的账号:密码。

### 2.5 全屏 / 调试参数

```bash
# 桌面无边框全屏
/Users/tetsuya/Development/mir2x/b/run-client.sh 192.168.1.10 \
    --disable-audio --disable-ime --screen-mode=2

# 显示 UID 和地图网格（调试用）
/Users/tetsuya/Development/mir2x/b/run-client.sh 192.168.1.10 \
    --disable-audio --disable-ime --draw-uid --draw-map-grid
```

### 2.6 启动后看到一行报错？

```
error messaging the mach port for IMKCFRunLoopWakeUpReliable
```

**这是 macOS 输入法系统的噪音，不是 bug，忽略即可。**

---

## 3. 常用启动参数速查

完整列表见 `client/src/clientargparser.hpp`。挑常用的：

| 参数 | 作用 |
|---|---|
| `--server-ip=<ip>` | **必填**。服务器 IP |
| `--server-port=<port>` | 服务器端口（默认 5000） |
| `--auto-login=<账号>:<密码>` | 启动后直接登录，跳过界面 |
| `--screen-mode=0\|1\|2` | 0=窗口 / 1=独占全屏 / 2=桌面无边框 |
| `--res-path=<dir>` | 资源目录（启动脚本已自动设置，平时不用管） |
| `--disable-audio` | 关闭声音 |
| `--disable-ime` | 关闭中文输入法 |
| `--draw-uid` | 在每个生物头顶显示 UID（调试） |
| `--draw-map-grid` | 显示地图格子线（调试） |
| `--draw-hp-bar` | 显示血条（调试） |
| `--enable-client-monitor` | 客户端性能监视器 |

---

## 4. 退出与日志

- **退出**：关闭窗口，或按客户端的退出按钮
- **强制结束**：`pkill -9 -f "client --server-ip"`
- **看日志**：

```bash
ls -t /tmp/mir2x-client-v01.g3log.*.log | head -1   # 最新一份日志
tail -f $(ls -t /tmp/mir2x-client-v01.g3log.*.log | head -1)
```

---

## 5. 在 Mac 上跑服务器（推荐）

### 5.1 启动

```bash
/Users/tetsuya/Development/mir2x/b/run-server.sh
```

第一次跑会做这几件事：
- 在 `b/server/run/` 自动创建 `map`/`script` 软链
- 自动建 sqlite 数据库 `b/server/run/mir2x.db3`
- 预置 6 个测试账号：`test/123456`、`good/123456`、`id_1`..`id_4` 都是 `123456`
- 弹 FLTK 主窗口（普通模式不用动它）
- 监听 **TCP 7100**（不用默认 7000，因为 macOS 的 AirPlay Receiver 占着 7000）

### 5.2 客户端怎么连

```bash
# 同一台 Mac
/Users/tetsuya/Development/mir2x/b/run-client.sh 127.0.0.1 \
    --server-port=7100 --disable-audio --disable-ime
```

### 5.3 想用 7000？关掉 macOS 的 AirPlay Receiver

`System Settings → General → AirDrop & Handoff → AirPlay Receiver: OFF`，然后改 `run-server.sh` 把 `--client-port=7100` 删掉，客户端那边也别加 `--server-port=7100`，全走默认 7000。

### 5.4 退出 / 强制结束

- 关 FLTK 窗口
- 或：`pkill -9 -f "/server/src/server"`
- 看日志：`tail -f $(/bin/ls -t /tmp/mir2x-server-v01.g3log.*.log | head -1)`

### 5.5 重置 server 数据

```bash
rm /Users/tetsuya/Development/mir2x/b/server/run/mir2x.db3   # 下次启动会重新建库 + 预置账号
```

---

## 6. 在 Ubuntu 上编服务器（备用方案）

如果 Mac 不顺利或想跑生产环境，仍然可以在 Ubuntu 上编。

### 6.1 安装依赖（Ubuntu 22.04+ 或 24.04）

```bash
sudo apt update
sudo apt install -y build-essential cmake git git-lfs pkg-config \
    g++-13 \
    libssl-dev zlib1g-dev libpng-dev \
    libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-gfx-dev \
    libfltk1.3-dev libcairo2-dev \
    libpinyin-dev liblua5.4-dev

git lfs install
```

如果系统默认 GCC 比 13 老，强制使用 13：

```bash
export CC=gcc-13
export CXX=g++-13
```

### 6.2 拉代码 + 编译

```bash
cd ~/mir2x          # 假设代码在这
mkdir b && cd b
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
make install
```

### 6.3 启动 + 客户端连

```bash
cd ~/mir2x/b/install/server
./server

# Mac 上的客户端
/Users/tetsuya/Development/mir2x/b/run-client.sh <Ubuntu的IP>
```

---

## 7. 编译过程中改了哪些文件

为了在 macOS + Apple Clang 下能编通客户端，做了以下**最小化补丁**（都在源码里，已就地保存）：

### 7.1 顶层 `CMakeLists.txt`
- 新增 `MIR2X_CLIENT_ONLY` 选项，开启时跳过 `server/` 和 `tools/` 子目录
- Apple Clang 不加 `-fcoroutines`（GCC 才需要）和 `-Werror`（避免警告变错误）

### 7.2 `client/src/CMakeLists.txt`
- APPLE 时追加 `-L/opt/homebrew/lib`，让链接器找到 `SDL2_ttf/SDL2_gfx/SDL2_image/SDL2_mixer`

### 7.3 `common/src/CMakeLists.txt`
- `stdc++fs` 仅在 GCC 下链接（Apple Clang 没这库也用不到）
- common 显式 include SDL2/FLTK 头路径（避免子目标找不到）

### 7.4 缺失的标准头补全
- `common/src/triangle.cpp` 加 `<algorithm>`（`std::max/min`）
- `common/src/log.hpp` 加 `<unistd.h>`（`getpid()`）
- `common/src/buffrecord.hpp` 加 `<cmath>`（`std::lround`）
- `common/src/staticvector.hpp` 加 `<span>`（`std::span`）
- `client/src/layoutboard.cpp` 加 `<ranges>`（`std::views`）

### 7.5 sol2 / Lua 兼容
- 全工程 `sol::nil` → `sol::lua_nil`（sol2 v3 改名），影响：
  - `common/src/luaf.cpp`、`luamodule.cpp`
  - `server/src/player.cpp`、`servermap.cpp`、`npchar.cpp`、`serverluacoroutinerunner.cpp`、`quest.cpp`
- 终端执行 `brew unlink lua`：让 sol2 用项目内置的 Lua 5.4，而不是 brew 的 Lua 5.5（sol2 还不支持 5.5）

### 7.6 Clang 比 GCC 严格的地方
- `common/src/sdruntimeconfig.hpp`：模板特化 `=delete` 改成 `static_assert`（Clang 要求 `=delete` 必须在首次声明）
- `common/src/zcompf.hpp`：`src.data()` 改成 `src`（参数本来就是裸指针）
- `common/src/protocoldef.hpp`：补 `#include <utility>`（GCC 透过别的头间接引入了 `std::move`，Clang 要求显式引入）

### 7.7 第三方库的 macOS 不兼容
- `b/3rdparty/libpng/libpng-1.6.39/pngpriv.h`：去掉 `TARGET_OS_MAC` 分支（这条会拉 Classic Mac OS 的 `<fp.h>`，现代 macOS 没有）

### 7.8 运行期 dylib 路径修复

客户端和服务器都需要这两步 `install_name_tool` 修补：
- `liblz4.1.dylib` 的硬编码路径 `/usr/local/lib/...` → 改成 build 目录里的实际路径
- 给 `libz.1.dylib` 加 rpath 指向 zlib 安装目录

> 重新编完之后要再跑一次（或写进 `CMakeLists.txt` 的 POST_BUILD）。命令模板：
>
> ```bash
> for bin in client/src/client server/src/server; do
>   install_name_tool -change /usr/local/lib/liblz4.1.dylib \
>       /Users/tetsuya/Development/mir2x/b/3rdparty/lz4/install_lz4lib/usr/local/lib/liblz4.1.dylib \
>       /Users/tetsuya/Development/mir2x/b/$bin
>   install_name_tool -add_rpath \
>       /Users/tetsuya/Development/mir2x/b/3rdparty/zlib/build/install/lib \
>       /Users/tetsuya/Development/mir2x/b/$bin
> done
> ```

### 7.9 server 必须用 brew LLVM clang，不能用 Apple Clang 17

Apple Clang 17（`/usr/bin/clang++` 1700.6.4.2）在编 `server/src/player.cpp` 时撞编译器自己的 bug：模板里有嵌套 lambda + `const auto = [&]() -> int { ... }()` 的写法，前端解析时无限递归触发 SIGSEGV（`clang::Expr::EvaluateAsInitializer`）。换 brew 的 mainline LLVM 22.1.4 就过了，二者都用 libc++ 不会和 brew SDL2/FLTK 撞 ABI。

- 装 brew LLVM：`brew install llvm`
- 切编译器并清缓存重新 cmake：

```bash
cd /Users/tetsuya/Development/mir2x/b
rm -rf CMakeCache.txt CMakeFiles common/CMakeFiles server/CMakeFiles client/CMakeFiles \
       common/src/CMakeFiles server/src/CMakeFiles client/src/CMakeFiles
CC=/opt/homebrew/opt/llvm/bin/clang \
CXX=/opt/homebrew/opt/llvm/bin/clang++ \
cmake .. -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang \
         -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
         -DMIR2X_CLIENT_ONLY=OFF
make -j10 server
```

> **不用 GCC 的原因**：brew GCC 默认带 libstdc++，brew 的 SDL2/FLTK 用 libc++，混链会 ABI 崩。brew LLVM 默认就是 libc++，配上同样 libc++ 的 brew 库一致。

---

## 8. 常见问题（FAQ）

### Q1：客户端启动后立刻闪退，没看到窗口

```bash
# 看日志最后几行
tail -50 $(ls -t /tmp/mir2x-client-v01.g3log.*.log | head -1)
```

最常见的原因：
- `dyld: Library not loaded: ...` → 某个 dylib 路径没修好，参考第 7.8 节
- 资源路径错 → 确认你是用 `run-client.sh` 启动的，没有手工跑 `client` 二进制

### Q2：连接服务器失败

- 确认 server 在跑：`/bin/ps -ef | grep "/server/src/server" | grep -v grep`
- 确认 server 在监听：`lsof -iTCP:7100 -P -n`（默认改成了 7100，别忘了客户端 `--server-port=7100`）
- 防火墙放行端口（远程时）：`sudo ufw allow 7100`
- 服务器日志在 `/tmp/mir2x-server-v01.g3log.*.log`，启动失败优先看末尾几行

### Q3：怎么重新编译客户端 / 服务器

```bash
cd /Users/tetsuya/Development/mir2x/b
make -j10 client      # 只编客户端
make -j10 server      # 只编服务器（前提：cmake 已用 brew LLVM 配置好，见 7.9）

# 编完之后两者都要重新跑 install_name_tool 修复 dylib 路径（第 7.8 节）
```

### Q4：怎么彻底重新构建（删掉缓存）

```bash
rm -rf /Users/tetsuya/Development/mir2x/b
mkdir /Users/tetsuya/Development/mir2x/b
cd /Users/tetsuya/Development/mir2x/b

# 只要客户端
cmake .. -DMIR2X_CLIENT_ONLY=ON
make -j10

# 客户端 + 服务器（要 brew LLVM）
CC=/opt/homebrew/opt/llvm/bin/clang \
CXX=/opt/homebrew/opt/llvm/bin/clang++ \
cmake .. -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang \
         -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
         -DMIR2X_CLIENT_ONLY=OFF
make -j10
```

⚠️ 这会重新下载并解压所有第三方库（~30 分钟），并重新拉 1.8 GB 的 git-lfs 资源。

### Q5：在 Mac 上能不能编工具（pkgviewer 等）查看资源？

cmake 已经能配置 tools 子目录（FLTK 1.4 brew 装好了：`brew install fltk cairo`），但每个具体工具还没逐个验证编译。要试的话：

```bash
cd /Users/tetsuya/Development/mir2x/b
make -j10 pkgviewer    # 或 mapeditor / animaker 等具体目标
```

碰到错误就抓日志看，大概率是缺头文件或 Apple Clang 严格度问题，按第 7 节的套路补。

### Q6：客户端用 `--auto-login=...` 自动登入会崩

崩在 `xmltypeset.cpp:583: invalid default font: font = 11, fontsize = 15`。原因：客户端代码默认用 font id=11（要 `0B_WenQuanYi_Bitmap_Song_15_px.TTF`），但 `mir2x_data/res/font/` 只到 `07_*.TTF`，数据 res 比代码老。

绕开的办法：**启动时不加 `--auto-login`**，到登录界面手动输账号/密码。等以后数据 res 更新了或者改默认 font id 再说。

---

## 9. 一行 TL;DR

```bash
# Mac 上一台机起 server + client（最常见）：
/Users/tetsuya/Development/mir2x/b/run-server.sh &
/Users/tetsuya/Development/mir2x/b/run-client.sh 127.0.0.1 \
    --server-port=7100 --disable-audio --disable-ime
# 默认账号 test / 123456（手动登，别用 --auto-login，见 Q6）
```

```bash
# 想跑生产，Ubuntu 上编服务器：
cd ~/mir2x && mkdir b && cd b && cmake .. && make -j$(nproc) && make install
./install/server/server
# Mac 客户端
/Users/tetsuya/Development/mir2x/b/run-client.sh <Ubuntu的IP>
```
