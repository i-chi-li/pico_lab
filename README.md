# pico-lab

このプロジェクトは、Raspberry Pi Pico で、色々実験をするための環境となる。
作業は、Windows 上で実施する前提とする。
デバッグ用に picoprobe ツールを利用する場合、実行用と合わせて 2 台の Pico が必要となる。

# 環境
以下のツール類をインストール済みとする。
Pico 用ソースをクロスコンパイルするツールチェインは、WSL 上にインストールしたものを利用する前提とする。

- WSL Ubuntu 22.04 (GitHub [i-chi-li/wsl-setup](https://github.com/i-chi-li/wsl-setup.git) を参照。)
- MSYS2 MinGW64
- CLion（新 UI）

# 初期設定

## Pico 用の USB ドライバをインストールする
Pico 用の USB ドライバは、picotool が Pico に接続するために必要なドライバとなる。

以下のサイトから、zadig-2.8.exe をダウンロードする。バージョンは現時点での最新となる。

https://zadig.akeo.ie/

- Pico を BootSel ボタンを押しながら USB 接続する。
- デバイスマネージャーで、`ほかのデバイス` に `RP2 Boot` と表示されていることを確認する。
- zadig-2.8.exe を起動する。
    - 上のドロップダウンには `RP2 Boot (Interface 1)` と表示される。
    - Driver は、 `(NONE)` -> `WinUSB (v6.1.7600.16385)` となっているはず。
    - `Install Driver` ボタンを押下する。
    - ドライバのインストールが正常に完了したら、 zadig ツールを閉じる。

ドライバのインストールが完了すると、デバイスマネージャーには、
`ユニバーサル シリアル バス デバイス` に `RP2 Boot` と表示される。

- Pico に後にビルドする hello.uf2 を書き込んだ状態で USB 接続する。
- デバイスマネージャーで、`ほかのデバイス` に `Reset` と表示されていることを確認する。
- zadig-2.8.exe を起動する。
  - 上のドロップダウンには `Reset (Interface 2)` と表示される。
  - Driver は、 `(NONE)` -> `WinUSB (v6.1.7600.16385)` となっているはず。
  - `Install Driver` ボタンを押下する。
  - ドライバのインストールが正常に完了したら、 zadig ツールを閉じる。

ドライバのインストールが完了すると、デバイスマネージャーには、
`ユニバーサル シリアル バス デバイス` に `Reset` と表示される。


## picotool をビルドする。
picotool は、Pico に対して、実行ファイルを書き込んだり、
各種情報を取得したりするツール。必須ではない。

picotool のビルドは、MSYS2 MinGW64 上で行う前提とする。
MinGW64 コンソールを起動して、ホームディレクトリに移動する。

```
# picotool で必要なパッケージをインストール（インストール対象の選択肢が表示されるので、そのまま Enter ですべてインストールする）
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-libusb

# ソースをクローンする
git clone https://github.com/raspberrypi/picotool.git
git clone https://github.com/raspberrypi/pico-sdk.git

export PICO_SDK_PATH=$PWD/pico-sdk
MSYS2_ARG_CONV_EXCL=- cmake -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=$MINGW_PREFIX -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++" -S picotool -B picotool-build
cmake --build picotool-build -j 12
```

ビルドに成功すると、実行ファイル（`picotool-build/picotool.exe`）が生成される。
libusb のみ静的リンクにならないので、`mingw64/bin/libusb-1.0.dll` ファイルと合わせて、
以後の手順でクローンする `pico-lab` プロジェクトルートにコピーする。

## WSL 環境のパッケージインストール

```
# pico-sdk で必要なライブラリをインストール
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

## ソースリポジトリのクローン
Windows 上のストレージにソースを格納する前提とする。
現時点では、WSL 上に配置すると、デバッグ時に、elf ファイルの読み込みに失敗する。
依存するソースリポジトリは、このプロジェクトのディレクトリと同じ階層にクローンする。

```
git config --global core.autocrlf false

# pico-sdk をクローンする
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init

# pico-extras をクローンする
git clone https://github.com/raspberrypi/pico-extras.git
```

このプロジェクトをクローンする。

```
git clone https://github.com/i-chi-li/pico-lab.git
```

前述の手順で生成した、`picotool.exe` と、依存 DLL の `libusb-1.0.dll` を `pico-lab` ディレクトリにコピーする。

# ビルド手順

## コマンドラインでビルドをする場合

デバッグビルド

```
cd pico-lab
cmake -G Ninja -S . -B cmake-build-debug
cmake --build cmake-build-debug -j 12
```

リリースビルド

```
cd pico-lab
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -S . -B cmake-build-release
cmake --build cmake-build-release -j 12
```

## CLion でビルドをする場合

### ツールチェインの設定

- File | Settings | Build, Execution, Deployment | Toolchains
- ＋（Add）ボタン | WSL
  - Name: WSL arm gcc
  - Toolset: Ubuntu-22.04-devel
  - CMake: WSL CMake
  - Build Tool: /usr/bin/ninja
  - C Compiler: /usr/bin/arm-none-eabi-gcc
  - C++ Compiler: /usr/bin/arm-none-eabi-g++
  - Debugger: WSL GDB

作成した `WSL arm gcc` を `Up` ボタンで最上位まで移動して、デフォルトにする。

### CMake の設定
- 
- File | Settings | Build, Execution, Deployment | CMake
- ＋（Add）ボタンを押下すると、Release 設定が自動的に作成される。

`CMake options` に `-DCMAKE_VERBOSE_MAKEFILE=1` を設定すると、ビルド時に実行コマンドをログで確認できる。

### ビルド実施

- Run Debug ドロップダウンから「hello | Debug」を選択する。
- Build -> Build Project を選択する。

完了すると、`cmake-build-debug/hello.uf2` ファイルが生成される。
このファイルを、Pico の USB ドライブにコピーすると、LED の点滅が始まる。

# Pico で実行
uf2 ファイルを Pico に書き込む方法は、以下のようなものがある。
以下以外にも書き込み方法はある。

- BootSel モードでの書き込み
- picotool による書き込み
- OpenOCD による書き込み

picotool で Pico に書き込む場合

Pico を USB で接続する。

```
# BootSel モードに変更（BootSel ボタンを押しながら USB 接続した状態と同じにする）
picotool.exe reboot -f -u
# 書き込む
picotool.exe load -x cmake-build-debug/hello/hello.uf2
```

# Pico の USB 接続について
プログラム実行中の Pico は、USB 接続をすることで COM ポートを経由してアクセスすることができる。
Pico で出力したログを参照したり、BootSel モードにしたりすることができる。
Pico の COM ポートは、デバイスマネージャーの `ポート(COM と LPT)` に、
`USB シリアル デバイス(COM4)` のように表示される。環境によって COM 番号は変わる。
この COM ポートに、ターミナルソフトでアクセスすることで、Pico のログ参照や、BootSel モード変更ができる。

ログを参照する場合は、以下の設定で接続する。

- ビット/秒: 115200
- データビット：8
- パリティ：No
- ストップビット：1
- フロー制御：None

BootSel モードに切り替える場合は、以下の設定で接続する。
接続直後に BootSel モードに切り替わり、COM への接続は切断される。

- ビット/秒: 1200
- データビット：8
- パリティ：No
- ストップビット：1
- フロー制御：None

# デバッグの事前設定
Pico のデバッグには、picoprobe を利用する。
picoprobe は、Pico にインストールして、デバッグ用のツールとして利用する。
このため、プログラム実行用の Pico とは別に、追加で picoprobe 用 Pico が必要となる。

picoprobe の詳細は、以下のドキュメントを参照。
ドキュメントの `Debug with a second Pico` 部分に、
Pico を 2 台使用してデバッグする場合の配線方法が記載されている。

- [Getting Started with the Raspberry Pi Pico](https://rptl.io/pico-get-started)

上記の方法で、配線をした場合、デバッグ対象の Pico の UART 出力は、
picoprobe 側の Pico に入力され、picoprobe 側の USB 接続の COM ポートに出力される。
したがって、デバッグ対象に USB 接続せずに、
picoprobe 側の COM ポートに接続することで、デバッグ対象のログを参照できる。

## picoprobe をダウンロードする
以下のサイトでリリース `picoprobe-cmsis-v1.0.3` の `picoprobe.uf2` をダウンロードする。

[raspberrypi/picoprobe](https://github.com/raspberrypi/picoprobe.git)

## picoprobe を Raspberry pi pico に書き込む
BootSel ボタンを押しながら USB に接続し、pico のドライブに `picoprobe.uf2` をコピーする。

Windows のデバイスマネージャーで確認すると、以下のように認識されている。

- ユニバーサル シリアル バス デバイス
  - CMSIS-DAP v2 Interface

## Windows 用ドライバをインストールする（適宜）
現時点では、このドライバーのインストールなしで動作している。
以降の手順で問題が発生した場合は、適宜インストールする。

以下のサイトの必要なツール Windows 固有で ST-LINK/V2 ドライバ をダウンロードしてインストールする。
[OpenOCD のサポート](https://pleiades.io/help/clion/openocd-support.html)

## OpenOCD をセットアップする

### MSYS2 環境を設定する
MSYS2 コンソールを開く。

```
pacman -Syu
pacman -Su
pacman -S mingw-w64-x86_64-toolchain git make libtool pkg-config autoconf automake texinfo mingw-w64-x86_64-libusb
exit
```

MinGW64 コンソールを開く。

```
git clone --branch rp2040 --recursive --depth=1 https://github.com/raspberrypi/openocd.git
cd openocd
./bootstrap
./configure --enable-picoprobe --disable-werror
make -j 12
make install
exit
```

## CLion の設定

### OpenOCD 実行ファイルの設定

- File | Settings | Build, Execution, Deployment | Embedded Development
- OpenOCD Location: C:\path\to\msys2\mingw64\bin\openocd.exe

### 実行・デバッグ設定

- Run | Edit Configurations
- Add New Configurations | OpenOCD Download & Run
  - Name: hello
  - Target: hello.elf
  - Executable binary hello.elf
  - Debugger: Bundled GDB multiarch
  - Board config file: board/pico-debug.cfg
  - GDB port: 3333
  - Telnet port: 4444
  - Download: If updated
  - Reset: Init
  - Before launch: Build
  - Show this page: off
  - Activate tool windows: on
  - Focus tool windows: off

# デバッグの実施

- `CMake Profiles` から、`Debug` を選択する。
- `Recent configurations` から `hello` を選択する。
- `Debug 'hello'` ボタンを押下する。

.uf2 ファイルが OpenOCD 経由で Pico にアップロードされ、LED が点滅したら、
デバッグツールで、Pause Program で一時停止させる。

任意の箇所にブレークポイントを設定する。
デバッグを開始し、一時停止をしないとブレークポイントの設定はできないので注意。

# プログラムの実行
デバッグせずに、実行だけする場合は、`Run 'hello'` ボタンを押下する。
OpenOCD 経由で uf2 ファイルが Pico に書き込まれ、実行される。

# デバック対象のコンソールログを確認する方法

デバッグ対象のコンソールログは、picoprobe 用 Pico の COM ポートにも出力されている。

Windows で COM ポートにアクセスするには、IntelliJ や、RLogin などのツールを使用する。

## IntelliJ で COM ポートにアクセスする場合

IntelliJ（CLION なども含む）で、COM ポートにアクセスする方法は、
`Serial Port Monitor` プラグイン（JetBrains 公式）を利用する。
このプラグインは、デフォルトでインストールされているはずだが、
無い場合はインストールする。

以下のように利用する。

- `View` メニューの `Tool Windows` から `Serial Connections` を選択する。
- 画面左下の `Serial Connections` アイコンをクリックする。
- `Available Ports` から、該当の COM ポートを選択する。
- `Connect` ボタンをクリックする。
- 右側に表示された設定を行う。
- `Create profile...` をクリックし、適宜名前を付けて保存すると、同一ポートで複数の設定を保存できる。
- `Connect` ボタンをクリックすると、接続される。
- デフォルトでは、16 進表示なので、`Switch to HEX View` アイコンを OFF にするとテキスト表示のみとなる。

## RLogin で COM ポートにアクセスする場合

インストール方法などは、以下を参照。

- [RLogin](https://kmiya-culti.github.io/RLogin/)(ターミナルソフト)

COM ポート接続の設定方法は、以下のようにする。

- RLogin を起動する。
- `Server Select` ダイアログで、`新規` ボタンを押下する。
- エントリー（上）に、任意の名前を入力する。
- プロトコルは、'com' を選択する。
- TCPポートを選択する。
- シリアルの設定ボタンを押下する。
- ビット/秒、データビットや、パリティなど、各種設定を行い、OK ボタンを押下する。
- OK ボタンを押下して、設定を保存する。
- Server Select ダイアログで、上記で作成した設定を選択して、OK ボタンを押下すると、接続される。設定をダブルクリックでも接続できる。

