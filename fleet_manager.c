#include <stdio.h>
/*データ構造の定義*/
struct Vehicle{
    int id;
    double speed;
    double temp;
};

/*ファイル 出力 関数*/
int output_file(struct Vehicle v){

    FILE *file = fopen("fleet_log.txt", "a");

    if(file != NULL){
        fprintf(file, "ID:%d | SPEED:%.1f | TEMP:%.1f |\n",v.id, v.speed, v.temp);
        fclose(file);
        printf("-> 記録しました。");
        return 1;
    }
    return 0;
}

/*ファイル 入力/保存 関数*/
void input_and_save(){
    struct Vehicle v;
    int result = 0;
    printf("===車両連続診断ツール(-1入力で終了)===\n\n");

    while(1){
        printf("\n車両ID(終了は-1):");
        scanf("%d", &v.id);
        
        if(v.id == -1){
            printf("\n入力を終了します。\n");
                break;
        }

        printf("\n速度(km/h):");
        scanf("%lf", &v.speed);
        printf("\n温度(度):");
        scanf("%lf", &v.temp);

        //入力した直後に、そのまま保存関数へ渡す
        result = output_file(v);

        if(result == 0){
            printf("[WARNING]できていません");
        }
    }
}

/*ファイル読み込み関数*/
int read_file(){
    FILE *file = fopen("fleet_log.txt","r");

    int id;
    double speed;
    double temp;

    if(file == NULL){
        printf("[ERROR]Not oepn file.\n");
        return 0;
    }

    printf("\n=== ログ読み込み開始 ===\n");

    while(fscanf(file, "ID:%d | SPEED:%lf | TEMP:%lf |\n",&id, &speed, &temp) != EOF){
        printf("-> ID:%03d | SPEED:%.1f | TEMP:%.1f |\n",id,speed,temp);
    }

    fclose(file);
    printf("=== 読み込み完了 ===\n");
    return 1;
}

int main(){
    struct Vehicle v;
    int choice = 0;
    int result = 0;

    while(1){
        printf("\n=== 車両管理システムメニュー ===\n");
        printf("1.車両データの入力/保存\n");
        printf("2.ログの表示/分析\n");
        printf("3.終了\n");
        printf("選択してください(1-3):");
        scanf("%d", &choice);

        if(choice == 1){
            printf("\n[データ入力/保存モード]\n");
            input_and_save();

        }
        else if(choice == 2){
            printf("\n[ログ表示モード]\n");
            result = read_file();
            if(result == 1){
                printf("[INFO]表示成功\n");
            }
            else{
                printf("[WARNING]表示失敗\n");
            }
        }
        else if(choice == 3){
            printf("\n[システムを終了します]\n");
            break;
        }
        else{
            printf("1から3の間で入力してください。\n");
        }
    }
    return 0;
}
