# Building KiCad on Ubuntu

## Install the deps by installing the KiCad Daily Builds

sudo add-apt-repository ppa:kicad/kicad-dev-nightly
sudo apt update

<!-- For debug purpose -->
<!-- sudo apt install kicad-nightly -->
<!-- We'd like to install libwxgtk3.2-dev and python3-wxgtk4.0 from source or apt-build -->

sudo apt update && sudo apt install -y debhelper cmake doxygen libbz2-dev libcairo2-dev libglu1-mesa-dev \
libgl1-mesa-dev libglew-dev libx11-dev mesa-common-dev pkg-config libssl-dev libzstd-dev \
build-essential cmake-curses-gui grep python3-dev swig dblatex po4a asciidoc \
libgit2-dev libsecret-1-dev source-highlight libboost-all-dev libglm-dev \
libcurl4-openssl-dev libgtk-3-dev libngspice0-dev libngspice0  \
libocct-modeling-algorithms-dev libocct-modeling-data-dev libocct-data-exchange-dev \libocct-visualization-dev \
libocct-foundation-dev libocct-ocaf-dev unixodbc-dev libnng-dev libprotobuf-dev protobuf-compiler zlib1g-dev \
shared-mime-info python3-pytest python3-cairosvg python3-numpy

## Install webview && lld

sudo apt install libwxgtk-webview3.2-dev lld ninja-build -y 

## Building from source

### Build wxWidgets

git clone -b v3.2.4-fix-webview https://github.com/liangtie/wxWidgets.git
cmake -G Ninja -S . -B build_linux -D CMAKE_BUILD_TYPE=Debug -DwxUSE_WEBVIEW=ON -DwxBUILD_SAMPLES=ALL -DCMAKE_EXPORT_COMPILE_COMMANDS=ON  -DCMAKE_CXX_FLAGS=-fuse-ld=lld

## Build through apt-build

sudo apt-build install libwxgtk3.2-dev python3-wxgtk4.0



