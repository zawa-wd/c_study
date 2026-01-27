/***********************
 *C言語練習用プログラム
 *作成日:20260121
 *更新日:20260127
 ***********************/
#include <stdio.h>
#include <stdlib.h> //20260122 mallocとfreeを使うため追加(atoi,exit)
#include "fleet_system.h"

//列挙型の定義                                                         
enum MenuChoice {
    EXIT =          0,
    INPUT =         1,
    RESET =         2,
    ANALYSIS =     11,
    SEARCH =       12,
    TOP_SPEED =    13,
    RANKING =      14,
    SUMMARY =      15,
    CSV_OUT =      21,
    SPEEDING_CSV = 22
};

/*==========================
 ****** メインルーチン *****
 *==========================*/
int main(int argc, char *argv[]){
    //コマンドライン引数処理
    handle_args(argc, argv);

    int choice;

    while(1){
        //メニューを表示する前に件数を確認
        int current_count = get_log_count();
        
        //メニュー表示
        print_menu(current_count);

        if(scanf("%d", &choice) != 1){
            while(getchar() != '\n'); //数字以外の入力対策
            continue;
        }
       
        if(choice !=0 && choice != 1 && current_count == 0){
            printf("\n -> [WARNING]:No data\n"); //データなしの入力対策
            continue;
        }

        switch(choice){
            case INPUT:          run_input_mode();       break;
            case RESET:          run_reset_mode();       break;
            case ANALYSIS:       run_analysis_mode();    break;
            case SEARCH:         run_search_mode();      break;
            case TOP_SPEED:      analyze_top_speed();    break;
            case RANKING:        run_speed_ranking();    break;
            case SUMMARY:        run_id_summary();       break;
            case CSV_OUT:        save_as_csv();          break;
            case SPEEDING_CSV:   save_as_speeding_csv(); break;
            case EXIT: printf("    -> [INFO] 終了します\n"); return 0;
            default: printf("    -> [WARNING] 無効な数値です\n");

        }
    }
}

/*=====================
 ***** 各機能関数 *****
 *=====================*/

/****************************
 * メインメニュー関数
 **[概要]
 *
 **[引数]
 * int current_count :
 **[戻り値]
 * void
 ****************************/
void print_menu(int current_count){
    char *no_data = (current_count == 0)? "[!] NO DATA":"";

    printf("\n===============================\n");
    printf("    車両データ管理システム\n");
    printf("   [現在の登録件数：%3d 件]\n",current_count);
    printf("===============================\n");

    printf("\n【記録・登録】\n");
    printf("  01.走行データの新規入力\n");
    printf(" %s 02. LOG初期化\n", no_data);
    
    printf("\n【分析・検索】\n");
    printf(" %s 11. ログの表示/分析\n", no_data);
    printf(" %s 12. 車両ID検索\n", no_data);
    printf(" %s 13. 最高速度検出\n", no_data);
    printf(" %s 14. ランキング出力\n", no_data);
    printf(" %s 15. 走行回数出力\n", no_data);

    printf("\n【外部出力・保存】\n");
    printf(" %s 21. CSVファイル出力\n",no_data);
    printf(" %s 22. 速度違反車の出力\n",no_data);

    printf("\n【0. 終了】\n");

    printf("\n===============================\n");

    printf("\n >> 選択してください:");
}

/****************************
 * コマンドライン引数用関数
 **[概要]
 *コマンドライン引数を設定する
 **[引数]
 * void:
 **[戻り値]
 * void
 ****************************/
void handle_args(int argc, char *argv[]){

    //LOG FILEを指定利用可能に変更
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "--file") == 0 && i + 1 < argc){
            
            snprintf(log_file_path, sizeof(log_file_path), "%s", argv[i+1]);

            printf(" -> [INFO]: 読み込みファイルを %s に変更しました\n", log_file_path);
        }
    }

    //引数がなければ何もしない
    if(argc < 2) return;

    if(strcmp(argv[1], "--ranking") == 0){
        run_speed_ranking();
    }
    else if(strcmp(argv[1], "--summary") == 0){
        run_id_summary();
    }
    else if(strcmp(argv[1], "--csv") == 0){
        save_as_csv();
    }
    else if(strcmp(argv[1], "--search") == 0){
        if(argc < 3){
            printf("\n -> [ERROR]:IDを指定してください\n");
            printf("            (例：--search 101)\n");
            exit(1);
        };
        
        int target_id = atoi(argv[2]);  //atoiを利用してint型へ変換
        search_by_id(target_id);        //関数呼出
        
    }
    else{
        printf("Usage: %s [オプション]\n", argv[0]);
        printf("オプション:\n");
        printf(" --ranking     : 速度ランキングを表示\n");
        printf(" --summary     : 車両別集計を表示\n");
        printf(" --csv         : csvファイル出力\n");
        printf(" --search [ID] : 車両IDの車について検索します\n");
    }
    exit(0); //引数がある場合は、処理して終了
}
