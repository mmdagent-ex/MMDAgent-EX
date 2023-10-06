# Contents

MMDAgent-EX では、ある一連の動作を行うための3Dモデル、モーション、FSTスクリプト、画像等のファイル集合（アセット）を「コンテンツ」と呼んでいます。１つのコンテンツは、ローカルファイルシステム上のあるフォルダ以下に格納するか、あるいはURLで指定することができます。

## コンテンツの構成

コンテンツはあるフォルダ以下にあるように構成します。以下が典型的なファイル構成です。トップディレクトリに起動の起点となる .mdf ファイルが必ず１つ（以上）あります。さらに、.fst, .dic, .rapiddic, .jconf, .ojt の各種ファイルが同様にトップディレクトリに、.mdf と同じプレフィックスで置かれます。また `BUTTON*.txt`, `PACKAGE_DESC.txt` もトップディレクトリに置きます。そのほかのファイル（モデル・モーション・画像等）については、フォルダ以下の任意の場所に置くことができます。

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

## コンテンツの再生

コンテンツの再生方法として、ローカル再生とWeb再生の２種類があります。MMDAgent-EX は再生するコンテンツをコマンド引数で指定して起動します。

### 指定無し

引数無しでMMDAgent-EXを単体起動したとき、以下の順番でコンテンツが探されます。

- 以前の起動時にユーザが設定したホームコンテンツ
- MMDAgent-EX の実行ファイルがあるフォルダ

起動するコンテンツが見つからない場合、ブランク画面で起動します。


### ローカル再生

コマンド引数に起動したいコンテンツの .mdf ファイルのパスを指定します。

```shell
  % ./Release/MMDAgent-EX.exe /some/where/foobar.mdf
```

もし複数の .mdf が指定された場合、全ての .mdf の内容（パラメータ設定）を順に読み込んで適用し、最後に指定された .mdf のコンテンツが起動されます。

### Web再生

Web サーバ上に公開されているコンテンツをダウンロードして起動できます。利用する場合、まずコンテンツ公開側はコンテンツ一式をあらかじめ Webサーバにアップしておく必要があります。

> Webサーバはディレクトリアクセス可能である必要があります。すなわち、コンテンツ全体を `https://foo.bar/some/dir` という場所にアップした場合、そのコンテンツ内の `model/xxx.pmd` というファイルは `https://foo.bar/some/dir/model/xxx.pmd` というURLで直接アクセス可能になっている必要があります。これは MMDAgent-EX がファイルごとに個別にダウンロードステータスを管理しているためです。このため、Google Drive や DropBox といったファイルごとにユニークなダウンロードURLを生成するサイトは使えないので注意してください。

起動時に、Webコンテンツのトップディレクトリの URL をコマンド引数で与えることでダウンロードと再生が行われます。URLは `https://foo.bar/some/dir` のほか、`mmdagent://foo.bar/some/dir` のようにも指定可能です。

```shell
  % ./Release/MMDAgent-EX.exe mmdagent://foo.bar/some/dir
```

この方法でダウンロードされたコンテンツはディスクにキャッシュされ、２回目以降は高速に起動します。ダウンロードが中断した場合でも次回同じコンテンツを起動する際に続きから再開されます。

## コンテンツの自動更新

Webコンテンツは、常に同期して最新に保たれます。Web コンテンツの更新は定期的にチェックされ、差があるときは自動的に差分のダウンロード・更新が行われます。これにより、再生する Web コンテンツが常に最新であることが保証されます。
