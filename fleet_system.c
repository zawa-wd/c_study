/**********************
 *C言語練習用プログラム
 *作成日:20260121
 *更新日:20260126
 ***********************/
#include <stdio.h>
#include <stdlib.h>

/*--- 設定 ---*/
#define LOG_FILE          "fleet_log.txt"
#define CSV_FILE          "fleet_log.csv"
#define SPEEDING_CSV_FILE "fleet_speeding_log.csv"
#define REPORT_FILE       "analysis_report.txt"
#define SPEED_LIMIT  100.0  //最高速度
#define TEMP_LIMIT   80.0   //温度チェック
                            //maloc対応で不要になる可能性あり
#define MAX_LOGS     999    //999件までメモリに載せる

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
int  load_all_logs       (struct Vehicle logs[]); //全ログ数カウント20260122
void run_input_mode();      //入力用関数
void run_analysis_mode();   //出力用関数
void run_search_mode();     //検索用関数20260122
void run_reset_mode();      //初期化関数20260122
void analyze_top_speed();   //最高速度を検出20260122
void save_as_csv();         //CSVファイル出力20260122
void run_speed_ranking();   //最高速度ランキング20260122
void save_as_speeding_csv();//速度違反車リスト出力(CSV)20260122
void run_id_summary();      //車両ID毎の走行回数表示20260122

/*==========================
 ****** メインルーチン *****
 *==========================*/
int main(){
    int choice;

    while(1){
        printf("\n=== 車両管理システムメニュー ===\n");
        printf(" 1. 車両データの入力/保存\n");
        printf(" 2. ログの表示/分析\n");
        printf(" 3. 車両ID検索\n");
        printf(" 4. LOG初期化\n");
        printf(" 5. 最高速度検出\n");
        printf(" 6. CSVファイル出力\n");
        printf(" 7. ランキング出力\n");
        printf(" 8. 速度違反車の出力\n");
        printf(" 9. 走行回数出力\n");
        printf(" 0. 終了\n");
        printf("    -> [INFO] 選択してください:");

        if(scanf("%d", &choice) != 1){
            while(getchar() != '\n'); //数字以外の入力対策
            continue;
        }

        switch(choice){
            case 1: run_input_mode();       break;
            case 2: run_analysis_mode();    break;
            case 3: run_search_mode();      break;
            case 4: run_reset_mode();       break;
            case 5: analyze_top_speed();    break;
            case 6: save_as_csv();          break;
            case 7: run_speed_ranking();    break;
            case 8: save_as_speeding_csv(); break;
            case 9: run_id_summary();       break;
            case 0: printf("    -> [INFO] 終了します\n"); return 0;
            default: printf("    -> [WARNING] 無効な選択です\n");

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
    FILE *file = fopen(LOG_FILE, "a");//LOG_FILEのデータ全てをみるのは大変だから、住所だけを渡しているイメージ

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
    fprintf(report, "========================\n");

    fclose(report);
    return 1;
}

/*******************************
 *関数
 **[概要]
 * ファイルから全データを配列に読み込み
 **[引数]
 * struct Vehicle logs[] :保存対象の車両データ構造体(ID、車速、温度を含む)
 **[戻り値]
 * int : count(読み込んだ件数を返す)
 *******************************/
int load_all_logs(struct Vehicle logs[]){
    FILE *file = fopen(LOG_FILE,"r");
    int count = 0;
    
    if(file == NULL) return 0;
    
    while(count < MAX_LOGS && fscanf(file, "ID:%d | SPEED:%lf | TEMP:%lf |\n", &logs[count].id, &logs[count].speed, &logs[count].temp) != EOF){
        count++;
    }

    fclose(file);
    return count;
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
            printf("\n -> [SUCCESS]:データを保存しました\n");
        }
        else{
            printf("\n -> [ERROR]:保存に失敗しました\n");            
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
        printf("\n -> [WARNING]:NOT LOG FILE\n");
        return;
    }

    printf("\n--- 走行ログ一覧 ---\n");

    while (fscanf(file, "ID:%d | SPEED:%lf | TEMP:%lf |\n", &id, &s, &t) !=EOF){
        printf("ID:%3d | 速度:%5.1f | 温度:%5.1f  ", id, s, t);

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
        else{
            printf("            ");
        }
        //温度の判定
        if(is_overheating(t)){
            printf("[HOT]");
        }
        else{
            printf("     ");
        }

        printf("\n");

    }

    fclose(file);

    printf("--- 分析結果 ---\n");

    if(count > 0) printf("平均速度: %.1f km/h\n", total_speed / count);//is_driving

    printf("警告件数: %d件\n", warning_count);//is_speeding

    if(report_save_to_file(count, warning_count, total_speed)){
        printf("\n -> [SUCCESS]:レポートを作成しました\n");
    }
    else{
        printf("\n -> [WARNING]:レポート作成失敗\n");
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
        printf("\n -> [WARNING]:NOT LOG FILE\n");
        return;
    }

    printf("\n検索したい車両IDを入力してください:");
    scanf("%d", &target_id);

    printf("\n--- ID:%3d の結果確認---\n", target_id);
    
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

    printf("---------------------------------\n");

    if(found_count > 0){
        printf("該当データ件数: %d 件\n", found_count);
        printf("該当車両の平均速度: %.1f km/h\n", total_speed / found_count);
    }
    else{
        printf("該当車両は見つかりませんでした\n");
    }
}

/*****************************
 * 初期化関数 
 **[概要]
 *LOG_FILEを初期化する機能
 **[引数]
 * void
 **[戻り値]
 * void
 *****************************/
void run_reset_mode(){
    char confirm;
    printf("\n【警告】すべての走行ログが削除されます\n");
    printf("         本当によろしいですか？(Y/N):");
    scanf(" %c", &confirm);

    if(confirm == 'y' || confirm == 'Y'){
        FILE *file = fopen(LOG_FILE, "w");

        if(file != NULL){
            fclose(file);
            printf("\n -> [SUCCESS]:DELET LOG FILE\n");
        }
        else{
            printf("\n -> [ERROR] - \n");
        }
    }
    else{
        printf("\n -> [INFO]:キャンセルしました\n");
    }
}

/*****************************
 * 全データ読み込み関数
 **[概要]
 *全データを読み込みし、最高速度の車両IDをみつける機能
 **[引数]
 * void
 **[戻り値]
 * void
 *****************************/
void analyze_top_speed(){
    struct Vehicle *logs = (struct Vehicle*)malloc(sizeof(struct Vehicle)* MAX_LOGS);
    if(logs == NULL) return;

    int count = load_all_logs(logs);//全データ読み出しのカウント

    if(count == 0){
        free(logs);
        return;
    }
    //配列の中から最高速度を捜す
    if(count > 0){
        int top_index = 0;

        for (int i = 1; i < count; i++){
            
            //現状の最高速度との比較し、更新をおこなう
            if(logs[i].speed > logs[top_index].speed){
                top_index = i;
            }
        }
        printf("\n[最高速度記録]\n");
        printf("車両ID:%03d | 速度:%.1f km/h\n",logs[top_index].id, logs[top_index].speed);
    }
    free(logs);
}

/*******************************
 *CSVログファイル保存関数
 **[概要]
 * CSVFILE名のファイルに入力した内容を保存をする
 **[引数]
 * void
 **[戻り値]
 * void
 *******************************/
void save_as_csv(){
    struct Vehicle *logs = (struct Vehicle *)malloc(sizeof(struct Vehicle) *MAX_LOGS);

    if(logs == NULL) return;

    int count = load_all_logs(logs);//全データ読み出しのカウント

    if(count == 0){
        free(logs);
        return;
    }

    FILE *csv_file = fopen(CSV_FILE, "w");//ポインタを利用してファイルの住所を教えているイメージ
    if (csv_file == NULL){
        free(logs);
        return;
    }

    fprintf(csv_file, "ID,SPEED,TEMP\n");

    //配列をCSVファイルに保存する
    for (int i = 0; i < count; i++){
        fprintf(csv_file, "%d,%lf,%lf\n", logs[i].id, logs[i].speed, logs[i].temp);
    }
    fclose(csv_file);
    free(logs);
    printf("\n -> [SUCCESS]:CSVファイルへの保存が成功しました\n");
}

/*******************************
 *最高速度ランキング関数
 **[概要]
 * ログ内の速度を比較し、ランキングを作成する
 **[引数]
 * void
 **[戻り値]
 * void
 *******************************/
//2重ループをよく理解する。イメージは植木算しながら右端をどんどん決定しているいめーじ
//隣同士でそれぞれ比較して、おそい車をどんどん移動するイメージ
void run_speed_ranking(){
    //struct Vehicle logs[MAX_LOGS];
    struct Vehicle *logs = (struct Vehicle*)malloc(sizeof(struct Vehicle) *MAX_LOGS);

    if(logs == NULL) return;

    int count = load_all_logs(logs);//空箱の作成(ここに詰めていく

    if(count == 0){
        free(logs);
        return;//データが無場合終了
    }
    for(int i = 0; i < count -1; i++){
        for (int j = 0; j < count -1 -i; j++){
            if(logs[j].speed < logs[j + 1].speed){
                struct Vehicle temp = logs[j];//構造体まるごとコピー(ID,速度、温度)
                logs[j] = logs[j + 1];
                logs[j + 1] = temp;
            }
        }
    }
    printf("\n--- 速度ランキング ---\n");
    for (int i = 0; i < count; i++){
        printf("%2d位:ID:%3d | 速度:%5.1f km/h\n", i + 1, logs[i].id, logs[i].speed);
    }
    free(logs);
}

/*******************************
 *速度違反車CSV出力関数
 **[概要]
 * 速度違反車両をまとめてCSVを作成する
 **[引数]
 * void
 **[戻り値]
 * void
 *******************************/
void save_as_speeding_csv(){
    struct Vehicle *logs = (struct Vehicle *)malloc(sizeof(struct Vehicle) * MAX_LOGS);
    
    //最初にチェック
    if(logs == NULL) return;
    //安全確認後、読み込み
    int count = load_all_logs(logs);

    if(count == 0){
        free(logs);
        printf("[ERROR] データが見つかりませんでした\n");
        return;//データが無場合終了
    }

    FILE *s_csv_file = fopen(SPEEDING_CSV_FILE, "w");
    if (s_csv_file == NULL){
        free(logs);
        return;
    }

    fprintf(s_csv_file, "--- 速度違反車リスト ---\n");
    fprintf(s_csv_file, "ID,SPEED\n");

    for(int i = 0; i < count; i++){
        if(is_speeding(logs[i].speed)){
            fprintf(s_csv_file, "%d,%lf\n", logs[i].id, logs[i].speed);

        }
    }
    fclose(s_csv_file);
    printf("\n -> [SUCCESS]:速度違反車のCSVファイルを作成しました\n");
    free(logs);
}

/*******************************
 *走行回数出力関数
 **[概要]
 * 車両ID毎に走行した回数を出力する
 **[引数]
 * void
 **[戻り値]
 * void
 *******************************/
void run_id_summary(){

    //今までは固定値
    //struct Vehicle logs[MAX_LOGS];
    //ここからは動的(malloc)
    struct Vehicle *logs = (struct Vehicle *)malloc(sizeof(struct Vehicle) * MAX_LOGS);
    if(logs == NULL) return;

    int count = load_all_logs(logs);

    if (count == 0){
        free(logs);//データが空でも返す必要がある。
        printf("[ERROR] メモリの確保に失敗しました\n");
        return;
    }
    //ID順への並び替え
    for(int i = 0; i < count -1; i++){
        for(int j = 0; j < count - 1 - i; j++){
            if(logs[j].id > logs[j + 1].id){
                struct Vehicle temp = logs[j];
                logs[j] = logs[j + 1];
                logs[j + 1] = temp;
            }
        }
    }
    //IDが何回出現するかカウント
    printf("\n--- 車両別走行回数集計 ---\n");
    int i = 0;
    while(i < count){
        int current_id = logs[i].id;
        int id_count = 0;
        double id_total_speed = 0.0;//車速合計用

        //カウントアップ
        while(i < count && logs[i].id == current_id){
            id_total_speed += logs[i].speed;
            id_count++;
            i++;
        }

        printf("車両ID:%3d | 走行回数:%3d回 | 平均速度:%5.1f km/h\n", current_id, id_count, id_total_speed / id_count);
    }

    free(logs);
}
