# MMDAgent-EX

[![en](https://img.shields.io/badge/lang-en-red.svg)](README.md)
[![ja](https://img.shields.io/badge/lang-ja-blue.svg)](README.ja.md)
[![DOI](https://zenodo.org/badge/691418774.svg)](https://zenodo.org/doi/10.5281/zenodo.10427368)
---

![banner](CG-CAs.png)

[![Demo movie](http://img.youtube.com/vi/iu2gU2uHAcc/0.jpg)](https://www.youtube.com/watch?v=iu2gU2uHAcc)
---
## MMDAgent-EX とは

MMDAgent-EX は音声対話システム・アバターコミュニケーションのオープンソース研究開発プラットフォームです。CGキャラクターの表示・制御と音声・言語処理を組み合わせ、任意の音声対話システムを構築できる単体のシステムです。様々なモジュールの追加により、カメラやネットワークを組み合わせた多様なマルチモーダル対話システムを構築することが可能です。また、動作コマンドや音声データを外部からネットワーク経由で送り込むことでリップシンク付き音声再生やモーション再生・制御ができ、さまざまな既存の対話システムのフロントエンドとして動作させることもできます。

MMDAgent は2011年に音声インタラクションの研究開発のためのオープンソースのツールキットとして名古屋工業大学で開発されました。その後、研究のために対応フォーマット拡張、ネットワーク対応、UIの整備、外部操作機能などが開発されてきました。2020年12月より、ムーンショット型研究開発事業「アバター共生社会」において、CGキャラクターを介して会話するCGアバターコミュニケーションのプラットフォームとしても研究開発が進められています。ここでは、その研究開発の成果の一部をオープンソースで公開するものです。

MMDAgent-EX は MMD (MikuMikuDance) 互換の独自のOpenGLベースの描画エンジンを持ち、MMDの形式の3Dモデルとモーションを用いて対話キャラクターを構築することができます。Juliusや Open JTalk などの名工大で開発された音声認識・音声合成エンジンを持つほか、FSTベースの状態遷移型の原始的な対話スクリプト機能を内蔵しており、これ単体で音声対話システムを構築することが可能です。また、Python等のスクリプト言語と接続することができ、任意のクラウド音声認識・音声合成エンジンや ChatGPT 等の LLM をモジュールとして追加することができます。また、ソケット接続による外部制御にも対応していますので、既存の対話応答生成システムの入出力フロントエンドとして用いることもできます。

## 実行環境

Windows / macOS / Linux / WSL

## ドキュメント

使い方やチュートリアルなど、全てのドキュメントは [公式サイト](https://mmdagent-ex.dev/ja/) にあります。

## CGアバター

本ツールとともに高品位 CG アバターを制作しており、そのいくつかはGitHubで公開しています。動作テスト・学術研究・デモ・検証等にお使いいただけます。MMDAgent-EXと併せてご利用ください。

- [CGアバター ジェネ / Gene](https://github.com/mmdagent-ex/gene)
- [CGアバター うか / Uka](https://github.com/mmdagent-ex/uka)

## コントリビューションについて

不具合報告、改善提案、実験結果の共有などを歓迎します。実装以外の貢献（実験・検証・報告）も重要な貢献となります。

- 🐞 再現可能な不具合（OS / ビルド条件 / ログがあると助かります）
- 💡 機能追加・改善の要望（実装アイデアがあれば歓迎します）
- 🛠 実装・実験などの着手できる作業

👉 詳細は **CONTRIBUTING.ja.md** を参照してください。

---

## 利用事例・研究利用について

MMDAgent-EX を研究・デモ・教育で利用されている場合、

- Issue や Discussions で用途を共有いただけると嬉しいです
- README 等での紹介を検討します

研究用途での利用実績は、プロジェクトの継続的改善に役立ちます。

利用例:

- [ムーンショット石黒プロジェクト](https://avatar-ss.org/)
- [JST CREST uDialogue project](http://www.udialogue.org/ja/)

## 引用

MMDAgent-EX を学術活動に利用した場合はぜひ引用してください。下記は引用形式の参考です。

**APA**

```text
Lee, A. (2023). MMDAgent-EX (Version 1.0.0) [Computer software]. https://doi.org/10.5281/zenodo.10427369
```

**BibTeX**

```text
@software{Lee_MMDAgent-EX_2023,
  author = {Lee, Akinobu},
  doi = {10.5281/zenodo.10427369},
  license = {Apache-2.0},
  month = dec,
  title = {{MMDAgent-EX}},
  url = {https://github.com/mmdagent-ex/MMDAgent-EX},
  version = {1.0.0},
  year = {2023}
}
```

## ライセンス

MMDAgent-EX のコードは Apache License, Version 2.0 で提供されています。

また、MMDAgent-EX は多くのソフトウェアを利用して構成されています。以下は利用しているライブラリの一覧です（これのみであることを保証するものではありません）。利用においては各ソフトウェアのライセンスにも留意してください。

- MMDAgent
- Bullet Physics
- FreeType
- GLFW
- GLEW
- libjpeg
- libpng
- librdkafka
- Libsndfile
- libsamplerate
- UTF8-CPP
- google/re2
- zlib
- PortAudio
- Open JTalk
- Julius
- POCO C++ Libraries
- RabbitMQ
- cpp-httplib

また、本ツールで3Dモデルやモーション、音や画像等の素材を利用する場合は、それらの作者（権利者）が定める利用規約に従ってください。

## ELSI ガイドライン

本ソフトウェアの倫理的・法的・社会的課題 (Ethical, Legal and Social Issues; ELSI) に対する利用ガイドラインを以下のとおり定めます。なお、本ガイドラインは時代や技術の進展、法的要件の変更に応じて更新されることがあります。

してはいけない使い方：

- 違法行為
- 児童性的虐待となる素材、または児童を搾取したり傷つけたりするコンテンツの再生
- 憎悪（ヘイト）、ハラスメント、暴⼒を助長するコンテンツの作成
- 物理的な危害を及ぼす危険性の⾼い活動。以下を含む：
  - 武器の製造
  - 軍事・紛争
  - エネルギー・交通・水等の重要施設の管理やオペレーション
  - 自殺・傷つけ・摂食障害などの自傷行為を促進、奨励、または描写するコンテンツ
- 詐欺的あるいは欺瞞的な⾏為。以下を含む：
  - 詐欺
  - 組織的な不正行為
  - 盗作
  - 偽情報の拡散
  - 他者を欺く目的で、実在の人物、団体、組織になりすましたり、虚偽の表現をする行為
- MikuMikuDance (MMD) の関連コミュニティおよびそこにある有形無形の価値を害する行為。以下を含む：
  - モデル・モーション・音楽等の権利者の権利を損なう行為
  - 権利者が定めるガイドラインに従わない利用
  - 不当な二次配布・盗用・作者なりすまし

## Contacts

- 開発元：[名古屋工業大学 李 研究室](https://www.slp.nitech.ac.jp/)

## Acknowledgments

MMDAgent-EX:

- Akinobu Lee (Nagoya Institute of Technology, Japan)

MMDAgent:

- Keiichi Tokuda (Nagoya Institute of Technology, Japan)
- Akinobu Lee (Nagoya Institute of Technology, Japan)
- Keiichiro Oura (Nagoya Institute of Technology, Japan)
- Daisuke Yamamoto (Nagoya Institute of Technology, Japan)
