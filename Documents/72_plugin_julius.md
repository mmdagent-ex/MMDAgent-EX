# Plugin_Julius

Plugin_Julius は音声認識エンジン Julius を用いた音声認識機能を提供するプラグインです。コンパクトな動作が特徴です。以下、このプラグインのための設定項目やメッセージ、使い方等を解説します。

## .mdf 設定

**Plugin_Julius_conf**, **Plugin_Julius_lang** （必須）

音声認識エンジンの設定名と言語名。

デフォルト指定は無し。モデルを準備し、これらの有効な組み合わせを .mdf で指定することで Plugin_Julius が有効化される。

デフォルトのモデルがサポートする組み合わせ：

- dnn, ja
- dnn, en
- gmm, ja

```text
Plugin_Julius_conf=dnn
Plugin_Julius_lang=en
```

**Plugin_Julius_wordspacing**

認識結果の出力において、単語を区切るかどうかを指定。

- `no`: 単語間に何も入れずに詰める（`ja` 時のデフォルト）
- `yes`: 単語間に空白を入れる（`ja` 以外のデフォルト）
- `comma`: 単語間にカンマを入れる（旧MMDAgentと互換）

```text
Plugin_Julius_wordspacing=yes
```

**Plugin_Julius_logfile**

Julius エンジンの内部ログをファイルに出力させる。

```text
Plugin_Julius_logfile=log.txt
```

**show_caption**

字幕を表示する。画面左側に音声認識結果が、右側に音声合成内容（**SYNTH_START** で与えられた文章）がそれぞれ表示される。

```txt
show_caption=true
```

## イベントメッセージ

**RECOG_EVENT_START**

音声入力を検知したときに出力。

```text
RECOG_EVENT_START
```

**RECOG_EVENT_STOP**

認識結果が得られたときに出力。

```text
RECOG_EVENT_STOP|認識結果の文章
```

**RECOG_EVENT_OVERFLOW**

入力音のレベルが大きすぎてオーバーフローを起こしたときに出力。

```text
RECOG_EVENT_OVERFLOW
```

**RECOG_EVENT_MODIFY**

RECOG_MODIFY メッセージの処理が完了したときに出力。

```text
RECOG_EVENT_MODIFY|GAIN
RECOG_EVENT_MODIFY|USERDICT_SET
RECOG_EVENT_MODIFY|USERDICT_UNSET
RECOG_EVENT_MODIFY|CHANGE_CONF|(jconf_file_prefix)
```

**RECOG_EVENT_AWAY**

メニュー操作や外部制御等によって音声認識を一時中断(ON)あるいは再開(OFF)したときに出力。

```text
RECOG_EVENT_AWAY|ON
RECOG_EVENT_AWAY|OFF
```

**RECOG_EVENT_GMM**

Juliusの環境音識別機能を用いているときの識別結果タグの出力。

```text
RECOG_EVENT_GMM|noise
```

## コマンドメッセージ

**RECOG_MODIFY**

エンジン設定の変更コマンド。動作中のエンジンを動的に変更する。

- `GAIN`: 入力音声の振幅スケーリング係数（デフォルト 1.0）
- `USERDICT_SET`: ユーザ辞書読み込み（既に読み込んである場合、入れ替え）
- `USERDICT_UNSET`: ユーザ辞書を削除
- `CHANGE_CONF`: 指定した jconf 設定ファイルでエンジンを再起動

```text
RECOG_EVENT_MODIFY|GAIN|(scale)
RECOG_EVENT_MODIFY|USERDICT_SET|(dict_file_path)
RECOG_EVENT_MODIFY|USERDICT_UNSET
RECOG_EVENT_MODIFY|CHANGE_CONF|(jconf_file_prefix)
```

**RECOG_RECORD_START**

入力音声の自動録音を開始。切り出された音声断片が、指定したディレクトリ内に個別の .wav ファイルとして逐次保存される。

```text
RECOG_RECORD_START|(directory)
```

**RECOG_RECORD_STOP**

入力音声の自動録音を停止。

```text
RECOG_RECORD_STOP
```

## オーディオ入力状態の連携

動作中は、全表示モデルにおいて、以下の名称のモーフ値がオーディオ入力の状態に合わせて自動更新され続ける（モーフが無い場合は変化なし）。

- モーフ "`volume`"：オーディオ入力のボリューム値(0.0～1.0)
- モーフ "`trigger`"：オーディオ入力が音声のとき 1.0、非音声のとき 0.0

これを使うことで、例えば入力ボリュームに連動してモーフを変化させたり、音声入力ON/OFFに合わせて表示を切り替えるといったインタラクティブ性を実装することができる。

また、オーディオ入力のボリュームは KeyValue 値 "`Julius_MaxVol`" にも随時セットされる。

## カスタマイズ

### コンテンツ辞書 (.dic)

未知語を定義した辞書を用意することで語彙を拡張できます。コンテンツごとの辞書は、.mdf ファイルの拡張子を .dic に変えたファイル名（仮に `foobar.mdf` であれば `foobar.dic`） でコンテンツ内に置いておきます。Plugin_Julius は起動時に上記の .dic ファイルを探し、あればそれを追加のユーザ辞書として読み込みます。

### コンテンツごとの設定 (.jconf)

また、Plugin_Julius は同様に `foobar.jconf` のようなファイルがあれば、それを追加の設定ファイルとして読み込みます。これを利用して、コンテンツごとに異なる Julius のパラメータや設定を与えることも可能です。

### モデルの追加等のさらなる拡張

オリジナルの最新版の Julius がフルで組み込まれており、フルカスタマイズが可能です。Julius で使える機能やモデル、設定をすべて利用できます。例えば、ある言語の Julius 用言語モデルと音響モデルを用意することで、他の言語のサポートを追加することができます。

カスタマイズされたモデルや辞書を使う場合、その Julius 設定ファイルを、 `Release/AppData/Julius` 以下に  `jconf_設定名_言語名.txt` というファイル名で置いてください。.mdf で、それらの設定名と言語名を指定することで、その設定ファイルで Plugin_Julius が起動します。

## 他のエンジンを使いたいとき

Julius はコンパクトなオープンソースの音声認識エンジンですが、ひと昔前の技術で作られており、モデルの性能や耐雑音性、特に雑音環境下での認識精度は最新の音声認識エンジンに劣る部分があります。

Google STT や Whisper のようなクラウド音声認識エンジンを Python でシステムを作成した場合、

- Plugin_AnyScript で MMDAgent-EX のサブモジュールとして動かす
- WebSocket 機能で別プロセスの MMDAgent-EX と外部連携させる

の2つの方法で連携できます。それぞれ関連するドキュメントを参考にしてください。
