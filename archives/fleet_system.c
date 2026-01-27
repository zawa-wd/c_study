/***********************
 *C言語練習用プログラム
 *作成日:20260121
 *更新日:20260126
 ***********************/
#include <stdio.h>
#include <stdlib.h>//20260122 mallocとfreeを使うため追加(atoi,exit)
#include <string.h>//20260126 コマンドライン引数を利用するため追加

/*--- 設定 ---*/
#define LOG_FILE          "fleet_log.txt"
#define CSV_FILE          "fleet_log.csv"
#define SPEEDING_CSV_FILE "fleet_speeding_log.csv"
#define REPORT_FILE       "analysis_report.txt"
#define SPEED_LIMIT  100.0  //最高速度
#define TEMP_LIMIT   80.0   //温度チェック

char log_file_path[256]     = LOG_FILE;
char csv_file_path[256]     = CSV_FILE;
char speeding_csv_path[256] = SPEEDING_CSV_FILE;

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
int  load_all_logs       (struct Vehicle logs[], int limit); //全ログ数カウント20260122
int  get_log_count();       //ログ件数カウント20260126
void handle_args(int argc, char *argv[]);//コマンドライン引数用20260126
void print_menu          (int current_coount);//メニュー表示用20260126
void run_input_mode();      //入力用関数
void run_analysis_mode();   //出力用関数
void run_search_mode();     //メニューからの検索用関数20260122
void run_reset_mode();      //初期化関数20260122
void analyze_top_speed();   //最高速度を検出20260122
void save_as_csv();         //CSVファイル出力20260122
void run_speed_ranking();   //最高速度ランキング20260122
void save_as_speeding_csv();//速度違反車リスト出力(CSV)20260122
void run_id_summary();      //車両ID毎の走行回数表示20260122
void search_by_id(int target_id);   //車両ID検索関数20260127
struct Vehicle* get_all_logs_dynamic(int *out_count);   //動的関数20260127

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
            strcpy(log_file_path, argv[i+1]);
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
    char *NO_DATA = (current_count == 0)? "[!] No Data":"";

    printf("\n===============================\n");
    printf("    車両データ管理システム\n");
    printf("   [現在の登録件数：%3d 件]\n",current_count);
    printf("===============================\n");

    printf("\n【記録・登録】\n");
    printf("  01.走行データの新規入力\n");
    printf(" %s 02. LOG初期化\n", NO_DATA);
    
    printf("\n【分析・検索】\n");
    printf(" %s 11. ログの表示/分析\n", NO_DATA);
    printf(" %s 12. 車両ID検索\n", NO_DATA);
    printf(" %s 13. 最高速度検出\n", NO_DATA);
    printf(" %s 14. ランキング出力\n", NO_DATA);
    printf(" %s 15. 走行回数出力\n", NO_DATA);

    printf("\n【外部出力・保存】\n");
    printf(" %s 21. CSVファイル出力\n",NO_DATA);
    printf(" %s 22. 速度違反車の出力\n",NO_DATA);

    printf("\n 【0. 終了】\n");

    printf("\n===============================\n");

    printf("\n >> 選択してください:");
}

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
    FILE *file = fopen(log_file_path, "a");//LOG_FILEのデータ全てをみるのは大変だから、住所だけを渡しているイメージ

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
int load_all_logs(struct Vehicle logs[], int limit){
    FILE *file = fopen(log_file_path,"r");
    int count = 0;
    
    if(file == NULL) return 0;
    
    while(count < limit && fscanf(file, "ID:%d | SPEED:%lf | TEMP:%lf |\n", &logs[count].id, &logs[count].speed, &logs[count].temp) != EOF){
        count++;
    }

    fclose(file);
    return count;
}

/*******************************
 *ログ件数カウント関数
 **[概要]
 * ファイルを開き、データが何件あるかを数える
 **[引数]
 * void
 **[戻り値]
 * int : count(読み込んだ件数を返す)
 *******************************/
int get_log_count(){
    FILE *file = fopen(log_file_path, "r");
    if(file == NULL) return 0;
    
    int count = 0;
    int id;
    double s, t;

    while (fscanf(file, "ID:%d | SPEED:%lf | TEMP:%lf |\n", &id, &s, &t) != EOF){
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
    FILE *file = fopen(log_file_path, "r");
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
            //[HOT]の列をあわせるため
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
    int target_id;

    printf("\n検索したい車両IDを入力してください:");
    scanf("%d", &target_id);

    search_by_id(target_id);//検索ロジック呼出
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
        FILE *file = fopen(log_file_path, "w");

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
    int count;
    struct Vehicle *logs = get_all_logs_dynamic(&count);//動的読み込み関数

    //配列の中から最高速度を捜す
    if(logs == NULL){
        printf("");
        return;
    }
     
    int top_index = 0;
    for (int i = 1; i < count; i++){
        
        //現状の最高速度との比較し、更新をおこなう
        if(logs[i].speed > logs[top_index].speed){
            top_index = i;
        }
    }
    printf("\n[最高速度記録]\n");
    printf("車両ID:%03d | 速度:%.1f km/h\n",logs[top_index].id, logs[top_index].speed);
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
    //ログ件数のカウント
    int count;
    struct Vehicle *logs = get_all_logs_dynamic(&count);
        if(logs == NULL){
        printf("\n -> [INFO]:データがありません\n");
        return;
    }

    FILE *csv_file = fopen(csv_file_path, "w");//ポインタを利用してファイルの住所を教えているイメージ
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
    //ログ件数のカウント
    int count;
    struct Vehicle *logs = get_all_logs_dynamic(&count);
    if(logs == NULL){
        printf("\n -> [INFO]:データがありません\n");
        return;
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
    //ログ件数のカウント
    int count;
    struct Vehicle *logs = get_all_logs_dynamic(&count);

    if(logs == NULL){
        printf("\n -> [INFO]:データがありません\n");
        return;
    }

    FILE *s_csv_file = fopen(speeding_csv_path, "w");
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
 *走行回数出力関数 **[概要]
 * 車両ID毎に走行した回数を出力する
 **[引数]
 * void
 **[戻り値]
 * void
 *******************************/
void run_id_summary(){
    int count;
    struct Vehicle *logs = get_all_logs_dynamic(&count);

    //ログ件数のカウント
    if(logs == NULL){
        printf("\n -> [INFO]:データがありません\n");
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

/*******************************
 *検索ロジック関数
 **[概要]
 * ID車両の検索をおこなう関数
 **[引数]
 * int target_id(検索したい車両ID)
 **[戻り値]
 * void
 *******************************/
void search_by_id(int target_id){   
    FILE *file = fopen(log_file_path, "r");
    int log_id;
    double s, t;
    int found_count = 0;
    double total_speed = 0.0;

    if(file == NULL){
        printf("\n -> [WARNING]:NOT LOG FILE\n");
        return;
    }

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

/*******************************
 *全ログ動的読み込み関数
 **[概要]
 * 現在のログ件数を確認し、メモリを確保して全データを読み込む
 **[引数]
 * int *out_count : 読み込んだ件数を書き込むためのポインタ
 * 補足：引数は▼全てからっぽだが、これに読み込んだ件数を入れてもらうために引数を渡す
 *       戻り値は確保したメモリの場所(住所)を返すために使用しているため、この方法とする
 **[戻り値]
 * struct Vehicle* : 確保したメモリのポインタ(住所)
 *******************************/
struct Vehicle* get_all_logs_dynamic(int *out_count){

    int total = get_log_count();//ファイルを開きLOGの件数を出す

    //LOGの件数が0件の場合
    if(total == 0){
        *out_count = 0;
        return NULL;
    }
    
    //logがある場合、必要分のメモリを用意する
    struct Vehicle *logs = (struct Vehicle*)malloc(sizeof(struct Vehicle) * total);

    //logに読み込めるデータが無い場合
    if(logs == NULL){
        *out_count = 0;
        return NULL;
    }
    
    //読み込んだデータを書き出す
    *out_count = load_all_logs(logs, total);
    return logs;
}
