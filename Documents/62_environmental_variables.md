---
title: 環境変数
---
MMDAgent-EX のファイルで環境変数を参照する方法、および MMDAgent-EX の動作に影響を与える環境変数について。

## .mdf ファイルでの環境変数の参照

`％ENV{名前}` で環境変数を参照可能。指定された名前の環境変数が定義されていない場合、空白になる。

## `AUDIO_START` 用再生コマンド play

Ubuntu と macOS では、`AUDIO_START` メッセージでのサウンドファイル再生に sox 付属のコマンド "play" を使用する。再生は MMDAgent-EX 内部から以下の要領で `-q` をつけて起動される。

```shell
play -q file.mp3
```

この `play` コマンドは、まず最初に PATH 上で探される。もしパス上に `play` が無い等の理由でエラーになる場合は、`/opt/homebrew/bin/play`, `/usr/local/bin/play`, `/usr/bin/play` の順で探し、最初に見つかったものが使われる。

`play` の代わりのサウンド再生コマンドを指定したい場合は環境変数 `MMDAGENT_AUDIO_PLAY_COMMAND` で指定する。

### コンテンツフォルダ

コンテンツフォルダはダウンロードしたコンテンツや履歴情報などを保存するワークエリアで、デフォルトではデスクトップ直下の "MMDAgent-Contents"だが、
環境変数 `MMDAgentContentDir` でその場所を別の場所に指定できる。

### Python 指定

`MMDAGENT_MSASR_PYTHON` で msasr 起動用の Python パスを指定する。

### 音声入力デバイス指定（MS版音声認識モジュール msasr）

MS版音声認識モジュール用の音声入力デバイスを指定できる。デバイス番号と名前の一覧は `list_audio_capture_devices.py` を実行することでチェックできる。指定しない場合のデフォルトはシステムの既定デバイス。

- `MMDAGENT_MSASR_MIC_DEVICE_NAME`: 音声入力デバイスを名前で指定。指定された文字列を含むデバイスで最初に見つかったものが選択される。
- `MMDAGENT_MSASR_MIC_DEVICE_ID`: 音声入力デバイスをIDで指定。

音声入力デバイスを決定するアルゴリズムは以下のとおり。

1. `MMDAGENT_MSASR_MIC_DEVICE_NAME` が定義されていればその名前を含むデバイスを探す。デバイスが見つかれば、それに決定。
2. 上記が未定義あるいはそれを含むデバイス名が見つからない場合、`MMDAGENT_MSASR_MIC_DEVICE_ID` をチェック。指定されていればそれに決定。
3. 上記も失敗する場合はシステムの規定デバイスに決定。

### Webカメラの起動を高速に

環境変数 `OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS` を `0` に指定することで、Webカメラの起動が早くなる。

この環境変数は OpenCV ライブラリ内でWebカメラを初期化する際、ハードウェアエンコード機能をもったWebカメラだと（開く際に全機能をスキャンしに行くので）開くのに数十秒～数分かかることがある状況に対応するもの。これを設定することで、ハードウェアエンコードに関する機能をスキャンせずに初期化する。MMDAgent ではキャプチャ映像の内部処理が必要なのでハードウェアエンコードは使用していない。なのでこれで問題になることは無い。

Plugin_Remote でのカメラ映像伝送や、Webカメラ on TextArea において効果あり。開くのが以上に遅い場合は上記をセットしてみるとよい。
