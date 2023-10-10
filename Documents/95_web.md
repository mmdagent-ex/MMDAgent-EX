---
title: Webコンテンツ
---

Web サーバ上に公開されているコンテンツをダウンロードして起動できます。

## コンテンツをWebへ公開する

### PACKAGE_DESC.txt の編集

### インデックスの更新

### Webサーバへアップロード

利用する場合、まずコンテンツ公開側はコンテンツ一式をあらかじめ Webサーバにアップしておく必要があります。

> Webサーバはディレクトリアクセス可能である必要があります。すなわち、コンテンツ全体を `https://foo.bar/some/dir` という場所にアップした場合、そのコンテンツ内の `model/xxx.pmd` というファイルは `https://foo.bar/some/dir/model/xxx.pmd` というURLで直接アクセス可能になっている必要があります。これは MMDAgent-EX がファイルごとに個別にダウンロードステータスを管理しているためです。このため、Google Drive や DropBox といったファイルごとにユニークなダウンロードURLを生成するサイトは使えないので注意してください。

## Webにあるコンテンツを再生する

起動時に、Webコンテンツのトップディレクトリの URL をコマンド引数で与えることでダウンロードと再生が行われます。URLは `https://foo.bar/some/dir` のほか、`mmdagent://foo.bar/some/dir` のようにも指定可能です。

```shell
  % ./Release/MMDAgent-EX.exe mmdagent://foo.bar/some/dir
```

この方法でダウンロードされたコンテンツはディスクにキャッシュされ、２回目以降は高速に起動します。ダウンロードが中断した場合でも次回同じコンテンツを起動する際に続きから再開されます。

### コンテンツの自動更新

Webコンテンツは、常に同期して最新に保たれます。Web コンテンツの更新は定期的にチェックされ、差があるときは自動的に差分のダウンロード・更新が行われます。これにより、再生する Web コンテンツが常に最新であることが保証されます。

### PACKAGE_DESC.txtの書式

基本

```text
# 起動するmdfファイルを指定する。
# 指定がない場合はコンテンツフォルダを検索し、最も深いフォルダ階層に
# ある .mdf ファイルから起動する。
execMDFFile=some/where/foobar.mdf

# ブックマークや履歴に表示するテキスト名。
# 指定しない場合、ファイル名が使われる。
# "label=" だけ書いた場合、テキスト名は出力されない。
#label=string

# ブックマークや履歴に表示する画像ファイル。
# 上記の label 指定と重ねて表示される（label= なら画像のみになる）。
# 7:1 に伸長して表示されるのでアスペクト比 7:1 で作るのがベスト。
# 指定しない場合は banner.png という名前のファイルが探される。
# banner.png も存在しなければ画像は使用されない。
#image=hoge.png

# コンテンツユーザに読んでほしい README ファイルを指定。
# 指定した場合コンテンツの初回起動時に全画面で表示される。
# テキストファイル (UTF-8) であること。
#readme=readme.txt

# README ファイルを表示したあとユーザに同意を強制するかを指定。
# trueに指定した場合、README の一番下に Accept と Decline の 2つのボタンが
# 表示され、Decline を押した場合に再生させないようにできる。
# true にする場合は上記の readme も必ず指定すること。
#readmeForceAgreement=true
```

セキュリティ関連

```text
# 内蔵ブラウザでブラウズを不許可にする
nonBrowse=true

# デスクトップOS（Win/Mac/Linux）での起動を不許可にする
nonDesktop=true
```

自動更新関連

コンテンツ再生中に特定ファイルを自動更新する。指定した時間ごとに、指定したファイルの更新を、そのコンテンツのソースのサーバに問い合わせ、サーバ側で更新があればそのファイルのみをバックグラウンドでダウンロードする。

通常はコンテンツ起動のタイミングでWebコンテンツの更新チェックと新規差分のダウンロードが行われる。これに加えてこのオプションを使用することで、コンテンツの起動中でも任意のファイルをコンテンツを止めずにバックグラウンド更新できる。

```text
# 更新の自動チェックとバックグラウンド更新を試みるファイル名を指定。
autoUpdateFiles=xxx.mdf[,xxx.fst,…]

# 自動チェックの間隔を秒で指定
autoUpdatePeriod=20
```

データ収集

`LOG_START` と `LOG_FINISH` の機能で記録したログをサーバにアップロードする設定。ステルス収集にならないよう、利用する際はユーザの同意等、運用に十二分に注意すること。

```text
# ログのサーバアップロード機能を有効にする。
# 送信先のサーバURLを指定。
#logUploadURL=url_string

# HTTP のバージョン文字列を指定するときに使う。
# デフォルトは "HTTP/1.1"
#logUploadHTTPVersion=STRING

# ログ識別文字列を指定
# コンテンツの名前やバージョンをログに埋め込みたいときに指定
#logIdentifier=string
```

Plugin_Kafka関連

```text
# Broker address for Apache Kafka logging.
kafkaBroker=host:port

# Partition number of the broker to connect.
kafkaPartition=partition_number

# Producer name to connect. When this option is specified, the app will
# connect to the Kafka server as “producer” at startup, and start
# sending all  #logs to the topic channel at real time.
#
# The app can be either producer or consumer, but not both.
# Do not specify both KafkaProducerTopic and KafkaConsumerTopic.
kafkaProducerTopic=topic_string

# Consumer name to connect. When this option is specified, the app will
# connect to the Kafka server as “consumer” at content startup, and start
# receiving feeded messages from the topic channel and processing them
# at real time.
#
# The app can act either producer or consumer, but not both.
# Do not specify both KafkaProducerTopic and KafkaConsumerTopic.
kafkaConsumerTopic=topic_string

# Compression codec.
kafkaCodec=codec_string
```
