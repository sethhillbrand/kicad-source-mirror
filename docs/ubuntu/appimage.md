## 1.从源码编译并打包成appimage

### 1.1 正常ubuntu 下源码编译华秋kicad并安装到./build/AppDir

注意修改为以下cmake配置，make后再安装


```bash
# cmake .. -DCMAKE_INSTALL_PREFIX=/usr

# make install DESTDIR=AppDir

cmake -S . -B build -G Ninja -DCMAKE_CXX_FLAGS=-fuse-ld=lld -DKICAD_USE_CMAKE_FINDPROTOBUF=ON -DCMAKE_INSTALL_PREFIX=/usr -DKICAD_BUILD_I18N=ON -DKICAD_BUILD_QA_TESTS=OFF -DCMAKE_BUILD_TYPE=Release

cmake --build build

# For the linuxdeploy to find the deps dlls 
cmake --install build

# Will install to ${project_root}/AppDir
DESTDIR=AppDir cmake --install build
```


### 1.2 下载 linuxdeploy

在上一步的 AppDir 上运行 linuxdeploy：

> ./linuxdeploy-x86_64.AppImage --appdir AppDir

### 1.3 下载生成appimage的插件


[linuxdeploy-plugin-appimage](github.com/linuxdeploy/linuxdeploy-plugin-appimage)

下载后都要chmod +x

### 1.4 执行指令生成 appimage

最简指令版：

> ./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage

或者更详细指令指定所用插件，用qt举例：

./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt --output appimage

### 1.5 得到并移动生成的AppImage，你要的位置替换下面的$OLD_CWD

mv QtQuickApp*.AppImage "$OLD_CWD"

### 1.6 以下是完整的shell脚本举例，以qt为例，将这个shell集成到ci中就可以用流水线打包生成appimage

```
#! /bin/bash

set -x
set -e

# building in temporary directory to keep system clean
# use RAM disk if possible (as in: not building on CI system like Travis, and RAM disk is available)
if [ "$CI" == "" ] && [ -d /dev/shm ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR=$(mktemp -d -p "$TEMP_BASE" appimage-build-XXXXXX)

# make sure to clean up build dir, even if errors occur
cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}
trap cleanup EXIT

# store repo root as variable
REPO_ROOT=$(readlink -f $(dirname $(dirname $0)))
OLD_CWD=$(readlink -f .)

# switch to build dir
pushd "$BUILD_DIR"

# configure build files with CMake
# we need to explicitly set the install prefix, as CMake's default is /usr/local for some reason...
cmake "$REPO_ROOT" -DCMAKE_INSTALL_PREFIX=/usr

# build project and install files into AppDir
make -j$(nproc)
make install DESTDIR=AppDir

# now, build AppImage using linuxdeploy and linuxdeploy-plugin-qt
# download linuxdeploy and its Qt plugin
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage

# make them executable
chmod +x linuxdeploy*.AppImage

# make sure Qt plugin finds QML sources so it can deploy the imported files
export QML_SOURCES_PATHS="$REPO_ROOT"/src

# initialize AppDir, bundle shared libraries for QtQuickApp, use Qt plugin to bundle additional resources, and build AppImage, all in one single command
./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt --output appimage

# move built AppImage back into original CWD
mv QtQuickApp*.AppImage "$OLD_CWD"
```

### 1.7 ci yml 示例

```
script:
  - bash travis/build-with-cmake.sh

```

