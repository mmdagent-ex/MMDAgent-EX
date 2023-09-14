# PACKAGE_DESC.txt

## 基本

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

## セキュリティ

```text
# 内蔵ブラウザでブラウズを不許可にする
nonBrowse=true

# デスクトップOS（Win/Mac/Linux）での起動を不許可にする
nonDesktop=true
```

## 自動更新

（Webコンテンツのみ）

コンテンツ再生中に特定ファイルを自動更新する。指定した時間ごとに、指定したファイルの更新を、そのコンテンツのソースのサーバに問い合わせ、サーバ側で更新があればそのファイルのみをバックグラウンドでダウンロードする。

通常はコンテンツ起動のタイミングでWebコンテンツの更新チェックと新規差分のダウンロードが行われる。これに加えてこのオプションを使用することで、コンテンツの起動中でも任意のファイルをコンテンツを止めずにバックグラウンド更新できる。

```text
# 更新の自動チェックとバックグラウンド更新を試みるファイル名を指定。
autoUpdateFiles=xxx.mdf[,xxx.fst,…]

# 自動チェックの間隔を秒で指定
autoUpdatePeriod=20
```

## データ収集（取り扱い注意）

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

## Plugin_Kafka

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
