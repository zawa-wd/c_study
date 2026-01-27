# --- 設定エリア ---
CC 		= gcc
CFLAGS 	= -Wall -Wextra -g
TARGET 	= fleet_manager
# .c ファイルを自動ですべてリストアップ
SRCS 	= $(wildcard *.c)
# .c のリストを元に .o(オブジェクトファイル)のリストを作成
OBJS 	= $(SRCS:.c=.o)

# --- 実行ルール ---

#1. 最終成果物を作るルール
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

#2 .cファイルから.oファイルを作る共通ルール
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#3 掃除用
clean:
	rm -f $(TARGET) $(OBJS)

#4 make cleanなど、名前がファイルと被っても動くようにする
.PHONY: clean
