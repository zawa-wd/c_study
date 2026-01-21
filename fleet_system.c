#include <stdio.h>

/*--- 設定 ---*/
#define LOG_FILE "fleet_log.txt"
#define SPEED_LIMIT 100.0

/*--- データ構造の定義 ---*/
struct Vehicle{
    int id;
    double speed;
    double temp;
};

/* --- プロトタイプ宣言 ---*/
int save_to_file(struct Vehicle v);
void run_input_mode();
void run_analysis_mode();

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
 * =======================*/

/*保存関数*/
int save_to_file(struct Vehicle v){
    FILE *file = fopen(LOG_FILE, "a");
    if (file == NULL) return 0;

    fprintf(file, "ID:%d | SPEED:%.1f | TEMP:%.1f |\n", v.id, v.speed, v.temp);
    fclose(file);
    return 1;
}

/*入力関数*/
void run_input_mode(){
    struct Vehicle v;
    while(1){
        printf("\n車両ID(-1で終了):");
        scanf("%d", &v.id);
        if(v.id == -1) break;

        printf("速度:"); scanf("%lf", &v.speed);
        printf("温度:"); scanf("%lf", &v.temp);

        if(save_to_file(v)){
            printf("->[SUCCESS]データを保存しました\n");
        }
        else{
            printf("->[ERROR]保存に失敗しました\n");            
        }
    }
}

/* 読み込み/分析 関数 */
void run_analysis_mode(){
    FILE *file = fopen(LOG_FILE, "r");
    int id;
    double s, t;
    if(file == NULL){
        printf("ログファイルが見つかりません\n");
        return;
    }

    printf("\n--- 走行ログ一覧 ---\n");
    while (fscanf(file, "ID:%d | SPEED:%lf | TEMP:%lf |\n", &id, &s, &t) !=EOF){
        printf("ID:%03d | 速度:%5.1f | 温度:%5.1f\n", id, s, t);
    }
    fclose(file);
}
