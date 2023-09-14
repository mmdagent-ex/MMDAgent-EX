# MMDAgent-EX

[![en](https://img.shields.io/badge/lang-en-red.svg)](https://github.com/mmd-agent/MMDAgent-EX/blob/main/README.md)
[![ja](https://img.shields.io/badge/lang-ja-blue.svg)](https://github.com/mmd-agent/MMDAgent-EX/blob/main/README.ja.md)

---

(注意事項)

- 本レポジトリに含まれるのは MMDAgent-EX のシステム（本体＋プラグイン）のみです。
- 3Dモデルや対話スクリプト等のコンテンツファイル（アセット）は別途ご用意ください。
- 本システムは学術用途であれば無償で利用できます。商用利用については、別途お問い合わせください。

---

## これはなに？

MMDAgent-EX は音声対話システム・アバターコミュニケーションの研究開発プラットフォームです。CGキャラクターの表示・制御と音声・言語処理を組み合わせ、任意の音声対話システムを構築できる単体のシステムです。また、様々なモジュールの追加により、カメラやネットワークを組み合わせた多様なマルチモーダル対話システムを構築することが可能です。動作コマンドや音声・フェイストラッキングの情報を外部からリアルタイムに送り込むことができ、遠隔操作・遠隔会話できるCGアバターとして動作させることもできます。

MMDAgent は2011年に機械と人間が音声言語を通じて会話を行う音声対話研究のためのオープンソースのツールキットとして名古屋工業大学で開発されました。その後、実験用システム MMDAgent-EX として、フォーマットの拡張や研究・実験のための様々な拡張、ネットワーク対応などが同大学の 李 研究室にて実装開発されてきました。2020年12月より、ムーンショット型研究開発事業「アバター共生社会」において、CGキャラクターを介して会話するアバターコミュニケーションの機能が追加され、自律音声対話システムと遠隔アバター会話がシームレスに融合した「自律・遠隔融合対話プラットフォーム」として研究開発が進められており、ここにその研究成果の一部を提供するものです。

MMDAgent-EX は MMD (MikuMikuDance) 互換の独自のOpenGLベースの描画エンジンを持ち、MMDの3Dモデルとモーションを用いて対話キャラクターを構築することができます。独自の音声認識・音声合成のエンジン、および原始的な対話スクリプト機能を内蔵しており、これ単体で音声対話システムを構築することが可能です。さらに、音声処理や対話処理を外部のエンジンから追加することも容易であり、クラウド音声エンジンや LLM によるチャットボットと接続したシステムも容易に構築可能です。

## ドキュメント

資料は Documents フォルダに記述されています。

- セットアップ
- ビルド手順
- チュートリアル
- 操作方法
- 設定方法
- 通信方法
- 外部アプリ連携
- コマンド仕様

本レポジトリにはツールのみが含まれています。対話システムやアバターシステムとして動作させるためには、本ツールのほかに、CGモデルやモーション、対話スクリプトといったモジュールやコンテンツが必要です。別途ご準備ください。

## ライセンス

### 基本事項

- システム本体のソースコードは修正BSDライセンスです。
- 本システムは学術用途であれば無償で利用できます。
- 本システムの二次配布はお控えください。
- MMD形式の3Dモデルを利用する場合は、モデルの利用規約に必ず従ってください。モーションや音楽等についても同様です。
- 商用利用については別途お問い合わせください。
- 本ライセンスは予告なく変更されることがあります。

### 音声認識モジュールについて

音声認識モジュールは、京都大学河原研究室が開発したものです。ライセンスについては以下の README と利用許諾書を必ずお読みください。

- [Release/AppData/msasr/src/README.md](https://github.com/mmd-agent/MMDAgent-EX/blob/main/Release/AppData/msasr/src/README.md)
- [Release/AppData/msasr/src/LICENSE-r2asr.txt](https://github.com/mmd-agent/MMDAgent-EX/blob/main/Release/AppData/msasr/src/LICENSE-r2asr.txt)

### その他のモジュール

MMDAgent-EX は他にも多くのソフトウェアを利用して構成されています。利用においては各ソフトウェアのライセンスにも留意してください。以下は一覧です（これのみであることを保証するものではありません）。

- Bullet Physics
- FreeType
- GLFW
- GLEW
- libjpeg
- libpng
- librdkafka
- Libsndfile
- UTF8-CPP
- google/re2
- zlib
- POCO C++ Libraries
- OpenCV
- FFmpeg
- PortAudio
- Open JTalk
- Julius

## Acknowledgments

（本体部分のみ）

MMDAgent-EX:

- Akinobu Lee (Nagoya Institute of Technology, Japan)

MMDAgent:

- Keiichi Tokuda (Nagoya Institute of Technology, Japan)
- Akinobu Lee (Nagoya Institute of Technology, Japan)
- Keiichiro Oura (Nagoya Institute of Technology, Japan)
- Daisuke Yamamoto (Nagoya Institute of Technology, Japan)
