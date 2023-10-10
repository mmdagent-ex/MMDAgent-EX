---
title: 実行
---

3Dモデル、モーション、FSTスクリプト、音声・画像等のファイルの集合（アセット）を「コンテンツ」と呼びます。MMDAgent-EX は起動時に1つのコンテンツを再生できます。以下、コンテンツの基本構成と起動方法を説明します。

## コンテンツの構成

コンテンツはあるフォルダ以下に構成されます。以下がコンテンツの典型的な構成例です。トップディレクトリに、起動の起点となる .mdf ファイルが必ず１つおかれます。さらに、.fst, .dic, .rapiddic, .jconf, .ojt の各種ファイルが同様にトップディレクトリに、.mdf と同じプレフィックスで置かれます。また `BUTTON*.txt`, `PACKAGE_DESC.txt` もトップディレクトリに置きます。そのほかのファイル（モデル・モーション・画像等）については、フォルダ以下の任意の場所に置かれます。1つのコンテンツの中に複数のコンテンツを置くこともできます。

```text
 topdir/
  |- Startup / Configuration file (.mdf) [*]
  |- Dialogue scenario script (.fst) [*]
  |- Recognition word dictionary (.dic) [*]
  |- Rapid word dictionary (.rapiddic) [*]
  |- Julius JConf file (.jconf) [*]
  |- Open JTalk setting file (.ojt) [*]
  |- Button definitions (BUTTON0.txt - BUTTON9.txt) [*]
  |- Package description (PACKAGE_DESC.txt) [*]
  |- Description text (README.txt) [*]
  +- (SubDirectories)
      |- 3-D models (.pmd)
      |- Motions (.vmd)
      |- TTS Voice model (.htsvoice)
      |- Background/Floor (images)
      |- Sound / Music files (sound files)
      |- Stage models (.pmd)
      |- Other assets (images, text files, etc.)
```

なお、コンテンツ内のあらゆる相対パス指定は、このトップフォルダからの相対パスとして扱われます。

## コンテンツを指定して起動

MMDAgent-EX のコマンドライン引数に.mdf ファイルを指定することで、そのコンテンツを再生できます。

## 指定なしで起動

引数無しでMMDAgent-EXを単体起動したときは、以下の順番でコンテンツが探されます。

- 以前の起動時にユーザが設定したホームコンテンツ
- MMDAgent-EX の実行ファイルがあるフォルダ

起動するコンテンツが見つからない場合、ブランク画面で起動します。


