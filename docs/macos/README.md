# Building KiCad

## Download Command line tools

https://developer.apple.com/download/all/?q=Command%20Line%20Tools

The following version is validated:

Command Line Tools for Xcode 15

Higher versions may not work


## Building deps with kicad-macos-builder

### Install deps with brew

Check ci/src/brew_deps.sh for deps to be installed by brew

### Run build.py

export WX_SKIP_DOXYGEN_VERSION_CHECK=1  

./build.py --arch=arm64 --target setup-kicad-dependencies

After building completed, you'd get the CMAKE_TOOL_CHAIN_FILE var like:

-DCMAKE_TOOLCHAIN_FILE=/Users/admin/code/kicad-mac-builder/toolchain/kicad-mac-builder.cmake


### Build wxWidgets

./configure \
    CPPFLAGS=-D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=1 \
    MAC_OS_X_VERSION_MIN_REQUIRED=${MACOS_MIN_VERSION} \
    CC=clang \
    CXX=clang++ \
    --prefix=${wxwidgets_INSTALL_DIR} \
    --with-macosx-version-min=${MACOS_MIN_VERSION} \
    --enable-unicode \
    --with-osx_cocoa \
    --enable-sound \
    --enable-graphics_ctx \
    --enable-display \
    --enable-geometry \
    --enable-debug_flag \
    --enable-debug \
    --enable-optimise \
    --disable-debugreport \
    --enable-uiactionsim \
    --enable-autoidman \
    --enable-monolithic \n
    --enable-aui \
    --enable-html \
    --disable-stl \
    --enable-richtext \
    --disable-mediactrl \
    --with-libjpeg=builtin \
    --with-libpng=builtin \
    --with-regex=builtin \
    --with-libtiff=builtin \
    --with-zlib=builtin \
    --with-expat=builtin \
    --without-liblzma \
    --with-opengl
