# ビルド手順

本レポジトリは Git LFS を使っています。
必ず Git LFS 対応のツールで clone するか git-lfs をインストールしてから clone 行ってください。

## Windows

Visual Studio 2022 でビルドする。Windows 11 + Visual Studio 2022 Community Edition で動作を確認している。インストール時に「C++によるデスクトップ開発環境」をインストールしておく（既にインストール済みの場合はインストーラを起動して再設定）。

ビルド設定は `win32 Release` と `win32 Debug` が用意されている。通常は `Release` の利用を推奨する。`Debug` はデバッグ可能なオプションでビルドされるがレンダリング処理が非常に遅いため軽量なコンテンツでないと動作しない。

1. `MMDAgent_vs2022.sln` を Visual Studio 2022 で開く
2. ソリューションエクスプローラで `main` を右クリックしてスタートアッププロジェクトに設定
3. ビルド設定を `Release` （あるいは `Debug`） に設定
4. 「ソリューションをビルド」を実行

ファイルアクセスのエラーにより、ビルドエラーが出ることがある。ビルドエラーが出た場合は、同じ「ソリューションをビルド」を何回か繰り返して実行してみるとよい。

ビルドされた実行バイナリ(.exe) とプラグイン (Plugins/*.dll) は `Release/` フォルダ（Debugビルドの時は `Debug`）以下に生成される。

実行時に、実行ファイルと同じディレクトリに `AppData` フォルダと `DLLs` フォルダが必要。`Plugins` フォルダの中は .dll ファイルのみ必要であり、他は消してしまって問題ない。別の場所で動かすときは `Release` フォルダの中身ごと移動・コピーすること。

## Ubuntu

Ubuntu-22.04 LTS で開発を行っている。また Ubuntu-20.04 でも動作を確認している。ビルドシステムは CMake である。

### 必要なパッケージのインストール

事前に必要なパッケージをインストールする。必要パッケージ名のリストが `requirements-linux.txt` にある。中に記されているパッケージを、全て事前に `apt install` しておく。以下の要領で一括インストールできる。

```shell
% sudo apt install `cat requirements-linux.txt`
```

### ビルド

以下の手順でリリースビルドする。デバッグビルドしたい場合は cmake のオプションを `-DCMAKE_BUILD_TYPE=Debug` に変更する。

```shell
% cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
% cmake --build build
```

ビルド成功後、必要な実行バイナリとプラグインは `Release/` ディレクトリ以下にコピーされる。

### 起動

起動手順は以下の通り。

```shell
% ./Release/MMDAgent-EX
```

実行時は実行ファイルと同じディレクトリに `AppData` フォルダが必要。別の場所で動かすときは `Release` フォルダの中身ごと移動・コピーする。

## macOS

M2 Macbook Air / macOS Ventura 13.5 で開発および動作確認をしている。CMake でビルド可能。

### 必要なパッケージのインストール

あらかじめ以下のパッケージを全て `brew install` しておく。

```text
ffmpeg
cmake
poco
glew
libjpeg
re2
portaudio
minizip
opencv
sox
rabbitmq-c
libomp
```

なお `libomp` については上記の `brew install libomp` でヘッダがうまくインストールされないことがあるようなので、インストール後に以下を追加で行っておく。

```shell
% brew link --force libomp
```

### ビルド

デバッグビルドしたい場合は cmake のオプションを `-DCMAKE_BUILD_TYPE=Debug` に変更する。

```shell
% cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
% cmake --build build
```

ビルドした実行バイナリとプラグインは `Release/` ディレクトリ以下にもコピーされる。

### 起動

起動は以下の通り。

```shell
% ./Release/MMDAgent-EX
```

実行時は実行ファイルと同じディレクトリに `AppData` フォルダが必要。別の場所で動かすときは `Release` フォルダの中身ごと移動・コピーする。

### macOS アプリとして登録

macOS アプリとして登録したい場合は、`os/macOS` 以下の `MMDAgent-EX.app` フォルダに必要なファイルが用意してあるのでそれを使う。ビルド後の `Release` フォルダの中身を全て `MMDAgent-EX.app` フォルダ内の `/Contents/MacOS` 以下にコピーすることで、Finderにおいてその `MMDAgent-EX.app` フォルダがアプリケーションバンドルとして機能する。Finder でこのアプリ（フォルダ）を アプリケーションフォルダに入れることで、 .mdf ファイルが関連付けられ、Finder で .mdf ファイルを開くと MMDAgent-EX が起動するようにできる。

## 追記

以下の同梱ライブラリは、Windows では使われるが、Ubuntu および macOS のビルドでは用いられない。Ubuntu / macOS ではシステムにインストールされているほうが優先して使われる。

```text
Library_RE2
Library_zlib
Library_JPEG
Library_libsndfile
Library_PortAudio
Library_glew
Library_Poco
Library_ffmpeg
Library_OpenCV
```
