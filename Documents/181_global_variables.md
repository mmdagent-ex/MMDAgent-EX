# グローバル変数

MMDAgent-EX はグローバル変数領域（キーと値のペア）を内部に持っており、コンテンツやプログラムから任意のキー名と値（文字列）を代入・参照可能である。以下、利用方法のサマリー。

## .mdf で代入

以下の形で書かれた任意の key-value ペアが起動時にグローバル変数に代入される。

```text
KeyName=String
```

## .fst で参照・代入

任意のフィールドでグローバル変数を `${%KeyName}` の形で値を参照できる。
値はロード時ではなく実行するタイミングで評価される。

```text
10 20:
    <eps> MODEL_ADD|${%KeyName}|...
```

以下のように条件に使うことも可能。

```text
10 20:
    ${%KeyName}==string SYNTH_START|mei|...
```

値の代入は `KEYVALUE_SET` メッセージでできる。

```text
10 20  MODEL_EVENT_ADD|0 KEYVALUE_SET|KeyName|String
```

あるいは第5フィールドを使っても行える。

```text
10 20  <eps>  MODEL_ADD|mei|... ${%KeyName}=String
```

## メッセージで代入

`KEYVALUE_SET` メッセージで値を代入できる。

```text
KEYVALUE_SET|(key name)|(value)
```

## MODEL_BINDFACE で活用

`MODEL_BINDFACE` コマンドで、指定したグローバル変数の値を常時監視して
モーフ値に反映するようにできる。なお、ここで参照するキーに対して
セットしてよい値は 0.0 から 1.0 の間の数値のみ。

```text
MODEL_BINDFACE|(key name)|(min)|(max)|(model alias)|(face name)|rate1|rate2
```

`MODEL_UNBINDFACE` で解除。

```text
MODEL_UNBINDFACE|(model alias)|(face name)
```

## 特別なグローバル変数

システムで利用されているグローバル変数。

### MS版のみ

`Avatar_mode` 通常 0.0、遠隔操作中のみ 1.0 の値になる
