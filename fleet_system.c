/**********************
 *C言語練習用プログラム
 *作成日:20260121
 *更新日:20260122
 ***********************/
#include <stdio.h>

/*--- 設定 ---*/
#define LOG_FILE     "fleet_log.txt"
#define REPORT_FILE  "analysis_report.txt"
#define SPEED_LIMIT  100.0
#define TEMP_LIMIT   80.0

/*--- データ構造の定義 ---*/
struct Vehicle{
    int    id;
    double speed;
    double temp;
};

/* --- プロトタイプ宣言 ---*/
int  is_driving          (double speed);     //走行中であるか判定
int  is_speeding         (double speed);     //スピードオーバーであるか判定
int  is_overheating      (double temp);      //温度の判定
int  save_to_file        (struct Vehicle v); //保存用関数
int  report_save_to_file (int count, int warning_count, double total_speed); //レポート作成用関数
void run_input_mode();      //入力用関数
void run_analysis_mode();   //出力用関数
void run_search_mode();      //検索用関数20260122

/*==========================
 ****** メインルーチン *****
 *==========================*/
int main(){
    int choice;

    while(1){
        printf("\n=== 車両管理システムメニュー ===\n");
        printf("1. 車両データの入力/保存\n");
        printf("2. ログの表示/分析\n");
        printf("3. 車両ID検索\n");
        printf("4. 終了\n");
        printf("   選択してください:");

        if(scanf("%d", &choice) != 1){
            while(getchar() != '\n'); //数字以外の入力対策
            continue;
        }

        switch(choice){
            case 1: run_input_mode(); break;
            case 2: run_analysis_mode(); break;
            case 3: run_search_mode(); break;
            case 4: printf("終了します\n"); return 0;
            default: printf("無効な選択です\n");

        }
    }
}

/*=====================
 ***** 各機能関数 *****
 *=====================*/

/****************************
 * 走行判定関数
 **[概要]
 *走行しているか(0km/hより速度が出ているか)を判定
 **[引数]
 * double speed :車速(km/h)
 **[戻り値]
 * int : 車速が0km/hより大きい場合1、0km/hの場合0を返す
 ****************************/
int is_driving(double speed){

    return speed > 0;
}

/*****************************
 * スピードオーバー判定関数
 **[概要]
 *SPEED_LIMIT以上である場合速度オーバーを判定する
 **[引数]
 * double speed :車速(km/h)
 **[戻り値]
 * int : 車速がSPEED_LIMITより大きい場合1、小さい場合0を返す
 ****************************/
int is_speeding(double speed){

    return speed >= SPEED_LIMIT;
}

/*****************************
 * 温度判定
 **[概要]
 *TEMP_LIMITより大きい場判定判定
 **[引数]
 * double temp :温度(度)
 **[戻り値]
 * int : 温度がTEMP_LIMITより大きい場合1、小さい場合0を返す
******************************/
int is_overheating(double temp){

    return temp >= TEMP_LIMIT;
}

/*******************************
 *ログファイル保存関数
 **[概要]
 * LOGFILE名のファイルに入力した内容を保存をする
 **[引数]
 * struct Vehicle v :保存対象の車両データ構造体(ID、車速、温度を含む)
 **[戻り値]
 * int : 保存成功時、1を返す
 *******************************/
int save_to_file(struct Vehicle v){
    FILE *file = fopen(LOG_FILE, "a");
    if (file == NULL) return 0;

    fprintf(file, "ID:%d | SPEED:%.1f | TEMP:%.1f |\n", v.id, v.speed, v.temp);
    fclose(file);
    return 1;
}

/********************************
 *レポートファイル作成/保存 関数
 **[概要]
 * 分析された統計データを受け取り、REPORT_FILEにフォーマットして書き出す
 **[引数]
 * int  count         :集計対象となった車両の総数 
 * int  warning_count :速度制限を超過した警告の発生回数
 * double total_speed :集計対象車両の速度の合計(km/h)
 **[戻り値]
 * int : 保存成功時は1、ファイルオープン失敗時は0を返す
 ********************************/
int report_save_to_file(int count,int warning_count, double total_speed){
    FILE *report = fopen(REPORT_FILE, "w");
    if (report == NULL) return 0;

    fprintf(report, "=== 車両分析レポート ===\n");
    fprintf(report, "集計対象件数: %d 件\n", count);

    if(count > 0){
        fprintf(report, "全体の平均速度: %.1f km/h\n", total_speed / count);
    }
    else{
        fprintf(report, "走行データなし\n");
    }

    fprintf(report, "速度超過警告数: %d 件\n", warning_count);
    fprintf(report, "===================\n");

    fclose(report);
    return 1;
}

/*****************************
 * 入力関数
 **[概要]
 * 車両ID、速度(km/h)、温度(度)の入力 
 **[引数]
 * void
 **[戻り値]
 * void
 ****************************/
void run_input_mode(){
    struct Vehicle v;
    while(1){
        printf("\n車両ID(-1で終了):");
        scanf("%d", &v.id);
        if(v.id == -1) break;

        printf("速度(km/h):"); scanf("%lf", &v.speed);
        printf("温度(度)  :"); scanf("%lf", &v.temp);
        
        //入力内容を保存
        if(save_to_file(v)){
            printf("->[SUCCESS]データを保存しました\n");
        }
        else{
            printf("->[ERROR]保存に失敗しました\n");            
        }
    }
}

/*****************************
 * 読み込み/分析 関数 
 **[概要]
 * LOG_FILEを読み込みし、表示
 * それぞれ分析している関数に渡す
 **[引数]
 * void
 **[戻り値]
 * void
 *****************************/
void run_analysis_mode(){
    FILE *file = fopen(LOG_FILE, "r");
    int id;
    double s, t;

    //--- 分析用の変数を追加 ---
    double total_speed = 0.0;
    int count = 0;
    int warning_count = 0;

    if(file == NULL){
        printf("ログファイルが見つかりません\n");
        return;
    }

    printf("\n--- 走行ログ一覧 ---\n");
    while (fscanf(file, "ID:%d | SPEED:%lf | TEMP:%lf |\n", &id, &s, &t) !=EOF){
        printf("ID:%03d | 速度:%5.1f | 温度:%5.1f  ", id, s, t);

        //走行中か判定
        if(is_driving(s)){
            total_speed += s;
            count++;
        }
        //スピードオーバーか判定
        if(is_speeding(s)){
            printf("[SPEED_OVER]");
            warning_count++;
        }
        //温度判定
        if(is_overheating(t)){
            printf("[HOT]");
        }

        printf("\n");

    }
    fclose(file);

    printf("--- 分析結果 ---");
    if(count > 0) printf("平均速度: %.1f km/h\n", total_speed / count);//is_driving
    printf("警告件数: %d件\n", warning_count);//is_speeding

    if(report_save_to_file(count, warning_count, total_speed)){
        printf("レポートを作成しました\n");
    }
    else{
        printf("レポート作成失敗\n");
    }        
}

/*****************************
 * 検索機能関数 
 **[概要]
 *入力した車両IDを検索して表示する
 **[引数]
 * void
 **[戻り値]
 * void
 *****************************/
void run_search_mode(){
    FILE *file = fopen(LOG_FILE, "r");
    int target_id, log_id;
    double s, t;
    int found_count = 0;
    double total_speed = 0.0;

    if(file == NULL){
        printf("[ERROR] Not Log File\n");
        return;
    }

    printf("検索したい車両IDを入力してください:");
    scanf("%d", &target_id);

    printf("\n--- ID:%03d の結果確認---\n", target_id);
    
    while (fscanf(file, "ID:%d | SPEED:%lf | TEMP:%lf |\n", &log_id, &s, &t) != EOF){
        if(log_id == target_id){
            printf("速度:%5.1f | 温度:%5.1f", s, t);

            if(is_speeding(s)) printf("[SPEED_OVER]");
            if(is_overheating(t)) printf("[HOT]");
            printf("\n");

            total_speed += s;
            found_count++;
        }
    }
    fclose(file);
    if(found_count > 0){
        printf("------------\n");
        printf("該当データ件数: %d 件\n", found_count);
        printf("該当車両の平均速度: %.1f km/h\n", total_speed / found_count);
    }
    else{
        printf("該当車両は見つかりませんでした\n");
    }
}
