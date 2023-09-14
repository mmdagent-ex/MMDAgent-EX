# 音声認識システム

ムーンショットで開発された京大の音声認識ソフトウェアが含まれています。

使うには独自のセットアップが必要です。また Plugin_AnyScript が必要です。以下、使い方です。

## ファイルの場所

ソフトウェアは Release/AppData/msasr フォルダに格納されています。
京都大学で公開されているバージョンに比べて以下の変更点があります。性能は変わりません。

- １つの Python スクリプトに統合
- 出力を MMDAgent-EX の仕様に合わせて変更
- 外部から一時停止 / 再開する機能を追加
- 入力待ち時のCPU負荷を軽減
- スレッド数の上限を制限

## インストール

実行には Python が必要。Release/AppData/msasr フォルダ内の Install.txt に従ってセットアップする。作成した conda 環境の `python.exe` の場所をメモする。

## 設定

.mdf に以下を設定する。 `python.exe` のところは上記でメモしたパスに置き換える。また `ALL.py` もこの `src` にある `ALL.py` をフルパスで指定する

```text
Plugin_AnyScript_Command=D:\miniconda\envs\ms_asr\python.exe -u C:\some\path\ALL.py --vad-conf conf.vad.yml --asr-conf conf.asr.yml --input mic
```

## 動作

上記の .mdf をもとに MMDAgent-EX を起動する。なお初回は 600MB弱 のモデルファイルをネットワークからダウンロードするため、ネットワーク環境が初回のみ必要。

音声認識モジュールは起動同時にバックグラウンドで自動起動する。デフォルトではマイク入力（既定デバイス）を認識する。

何らかの認識結果を持つ有効な音声入力が開始されたときに `RECOG_EVENT_START` メッセージを、
認識終了時に `RECOG_EVENT_STOP|認識結果の文字列` を出力する。

## 制御

`MSASR_DEACTIVATE` メッセージを送ることで待機状態にできる。待機状態では音声認識処理は行われず、CPUパワーもほとんど消費しない。

待機状態からの復帰は `MSASR_ACTIVATE` を送る。

それぞれ受信確認として `MSASR_EVENT_DEACTIVATE`, `MSASR_EVENT_ACTIVATE` メッセージが出される。

## その他

元のモジュールにあった各種通信用のソケット通信はそのまま残してあるので接続可能である（MMDAgent-EXとは標準入出力を介してやり取りしており、ソケット通信類は使用していない）。
