#include <stdio.h>
/*--- 設定 ---*/
#define LOG_FILE "fleet_log.txt"
#define REPORT_FILE "analysis_report.txt"
#define SPEED_LIMIT 100.0
#define TEMP_LIMIT 80.0

/*--- データ構造の定義 ---*/
struct Vehicle{
    int id;
    double speed;
    double temp;
};

/* --- プロトタイプ宣言 ---*/
int is_driving(double speed);       //走行中であるか判定
int is_speeding(double speed);      //スピードオーバーであるか判定
int is_overheating(double temp);    //温度の判定
int save_to_file(struct Vehicle v); //保存用関数
int report_save_to_file(int count, int warning_count, double total_speed);         //レポート作成用関数
void run_input_mode();              //入力用関数
void run_analysis_mode();           //出力用関数

/*=====================
 *メインルーチン
 *=====================*/
int main(){
    int choice;

    while(1){
        printf("\n=== 車両管理システムメニュー ===\n");
        printf("1. 車両データの入力/保存\n");
        printf("2. ログの表示/分析\n");
        printf("3. 終了\n");
        printf("   選択してください(1-3):");

        if(scanf("%d", &choice) != 1){
            while(getchar() != '\n');//数字以外の対策
            continue;
        }

        switch(choice){
            case 1: run_input_mode(); break;
            case 2: run_analysis_mode(); break;
            case 3: printf("終了します\n"); return 0;
            default: printf("無効な選択です\n");

        }
    }
}

/*========================
 *各機能
 *========================*/

/****************************
 * 走行中であるか判定
 * 0より大きい場合1、0以下の場合0を返す
 ****************************/
int is_driving(double speed){

    return speed > 0;
}

/*****************************
 * スピードオーバーの判定
 * SPEED_LIMITより大きい場合1、小さい場合0を返す
 ****************************/
int is_speeding(double speed){

    return speed >= SPEED_LIMIT;
}

/*****************************
 * 温度判定
 * TEMP_LIMITより大きい場合1、小さい場合0を返す
******************************/
int is_overheating(double temp){

    return temp >= TEMP_LIMIT;
}

/*******************************
 *保存関数
 *LOGFILE名のファイルに入力した内容を保存をする
 *******************************/
int save_to_file(struct Vehicle v){
    FILE *file = fopen(LOG_FILE, "a");
    if (file == NULL) return 0;

    fprintf(file, "ID:%d | SPEED:%.1f | TEMP:%.1f |\n", v.id, v.speed, v.temp);
    fclose(file);
    return 1;
}

/********************************
 *reportさくせいよう
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
 * 車両ID、速度、温度を入力
 ****************************/
void run_input_mode(){
    struct Vehicle v;
    while(1){
        printf("\n車両ID(-1で終了):");
        scanf("%d", &v.id);
        if(v.id == -1) break;

        printf("速度:"); scanf("%lf", &v.speed);
        printf("温度:"); scanf("%lf", &v.temp);
        
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
