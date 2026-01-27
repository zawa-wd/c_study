#ifndef FLEET_SYSTEM_H
#define FLEET_SYSTEM_H

#include <stdio.h>

/*--- 定数設定 ---*/
#define SPEED_LIMIT  100.0
#define TEMP_LIMIT   80.0

/*--- 構造体定義 ---*/
struct Vehicle {
    int    id;
    double speed;
    double temp;
};

/* --- 外部変数の宣言--- */
extern char log_file_path[256];
extern char csv_file_path[256];
extern char speeding_csv_path[256];

/* --- プロトタイプ宣言 ---*/

/*main.c*/
void print_menu (int current_coount);     //メニュー表示用20260126
void handle_args(int argc, char *argv[]); //コマンドライン引数用20260126

/*fleet_logic.c*/
int  is_driving          (double speed);        //走行中であるか判定
int  is_speeding         (double speed);        //スピードオーバーであるか判定
int  is_overheating      (double temp);         //温度の判定
int  save_to_file        (struct Vehicle v);    //保存用関数              
int  get_log_count       ();                    //ログ件数カウント20260126
int  report_save_to_file (int count, int warning_count, double total_speed); //レポート作成用関数
int  load_all_logs       (struct Vehicle logs[], int limit);                 //全ログ数カウント20260122
void run_input_mode      ();              //入力用関数
void run_analysis_mode   ();              //出力用関数
void run_search_mode     ();              //メニューからの検索用関数20260122
void run_reset_mode      ();              //初期化関数20260122
void analyze_top_speed   ();              //最高速度を検出20260122
void save_as_csv         ();              //CSVファイル出力20260122
void run_speed_ranking   ();              //最高速度ランキング20260122
void save_as_speeding_csv();              //速度違反車リスト出力(CSV)20260122
void run_id_summary      ();              //車両IDの走行回数表示20260122
void search_by_id        (int target_id); //車両ID検索関数20260127
struct Vehicle* get_all_logs_dynamic(int *out_count);   //動的関数20260127

#endif
