# 車両データ管理システム (Fleet Manager)

C言語の学習用として作成した、車両の走行ログ管理・分析ツールです。

## 🛠 機能
- 走行データの入力・保存
- 平均速度、速度超過、温度異常の自動分析
- 車両IDによる検索機能
- 速度ランキング表示
- 各種CSV出力 (全データ / 速度違反車リスト)
- コマンドライン引数によるクイックアクセス

## 🚀 セットアップ
ビルドには `make` を使用します。

```bash
make
```

##使い方

```bash
./fleet_manager
```

#引数モードについて

```bash
./fleet_manager --ranking      # ランキング表示
./fleet_manager --search 101   # ID 101を検索
./fleet_manager --file log.txt # ログファイルを指定して起動
```

## 📂 ファイル構成

```text
.
├── Makefile              # ビルド自動化スクリプト
├── README.md             # プロジェクト説明書（本ファイル）
├── fleet_manager         # 実行ファイル（ビルド後に生成）
├── src/                  # ソースコードディレクトリ
│   ├── main.c            # UI・コマンドライン引数処理
│   ├── fleet_logic.c     # データ処理・分析ロジック
│   └── fleet_system.h    # 構造体定義・プロトタイプ宣言
├── data/                 # データ保存ディレクトリ
│   ├── fleet_log.txt     # 走行ログ（テキスト）
│   ├── fleet_log.csv     # 全データ出力用CSV
│   └── fleet_speeding_log.csv  # 速度超過データ専用CSV
└── _archives/            # 過去のバックアップや古いコード
```