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