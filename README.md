**编译安装folly**
- 安装依赖
```shell
# Clone the repo
git clone https://github.com/facebook/folly
# Look for the dependencies
./build/fbcode_builder/getdeps.py install-system-deps --dry-run --recursive
# Install dependencies
cd folly
sudo ./build/fbcode_builder/getdeps.py install-system-deps --recursive
# Compile folly
python3 ./build/fbcode_builder/getdeps.py --allow-system-packages build --install-prefix ~/.local
```
- 设置PKG_CONFIG_PATH
```shell
export PKG_CONFIG_PATH=~/.local/folly/lib/pkgconfig:$PKG_CONFIG_PATH
# 查看folly库版本信息
pkg-config --modversion libfolly
# 查看folly库编译选项
pkg-config --cflags --libs libfolly
```
**编译本项目**
```shell
# Clone the repo
git clone https://github.com/xdlhzdh/folly-example
cd folly-example
mkdir build
cd build
cmake ..
make
```
**项目说明**
- 基于boost::asio协程实现了一个简单的tcp client, 位于source/boost_asio.cpp
<small>

1. 前提: 删除build目录, 把MAIN_X成main, 重新编译
2. client端运行方法:
```shell
./folly-example 127.0.0.1 8009
```
3. server端利用nc命令模拟echo
```shell
nc -l -p 8009 -k -c "cat"
```
</small>

- 基于folly测试了两个协程执行了其中一个即可退出的情况, 位于source/when_any.cpp

- 基于folly实现了一个把同步函数转换成异步函数的例子, 位于source/sync_to_async.cpp

