# flatpak本地构建

如果是虚拟机无法拷贝
sudo apt-get install open-vm-tools-desktop

预安装环境：
sudo apt install flatpak
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install flathub org.freedesktop.Sdk/x86_64/24.08
flatpak install flathub org.freedesktop.Platform/x86_64/24.08
flatpak update
sudo apt install flatpak-builder

拉取官方flatpak kicad 仓库，进入根目录：
flatpak-builder --repo=KiCadHuaqiu build-dir org.kicad.KiCad.yml --force-clean


如果报错glu-9.json不存在：
ls -l shared-modules/glu/glu-9.json
## 进入项目根目录
cd /home/zjz/code/com.nextpcb.KiCadHQ

## 初始化子模块（首次克隆仓库时使用）
git submodule init

## 更新子模块（拉取子模块代码）
git submodule update --recursive

ls -l shared-modules/glu/glu-9.json


如果报错OpenGL库找不到，wxwidget无法编译，实际libGLU.so 存在只是meason默认安装在/app/lib64/， 解决办法：
{
  "name": "glu",
  "buildsystem": "meson",
  "config-opts": [
    "-Ddefault_library=shared",
    "-Dlibdir=/app/lib"
  ],
  "sources": [
    {
      "type": "archive",
      "url": "https://archive.mesa3d.org/glu/glu-9.0.3.tar.xz",
      "sha256": "bd43fe12f374b1192eb15fe20e45ff456b9bc26ab57f0eee919f96ca0f8a330f"
    }
  ],
  "cleanup": [ "/include", "/lib/*.a", "/lib/*.la", "/lib/pkgconfig" ]
}
或者
wxwidget module的链接选项
  - name: wxWidgets
    cleanup:
      - /bin
      - /include
    config-opts:
      - --with-opengl
      - --disable-glcanvasegl
      - PKG_CONFIG_PATH=/app/lib/pkgconfig
      - LDFLAGS=-Wl,-rpath=/app/lib64:/app/lib -L/app/lib -L/app/lib64 -lGL # 添加 -lGL  解决链接不到/lib64的问题


编译后测试运行
flatpak-builder --run build-dir org.kicad.KiCad.yml kicad

编译后导出到仓库
flatpak build-export KiCadHuaqiu build-dir

flatpak build-bundle KiCadHuaqiu org.kicad.KiCad.flatpak org.kicad.KiCad


上传scp到公网机
ssh-keygen -t ed25519 -C "your_email@example.com"
scp -r -i ~/.ssh/id_ed25519 ./KiCadHuaqiu/* jzzhuang@120.78.83.231:/home/jzzhuang/code/hq-kicad/flatpak/com.nextpcb.KiCadHQ/KiCadHuaqiu
