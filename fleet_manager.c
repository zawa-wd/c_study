#include <stdio.h>

/*データ構造の定義*/
struct Vehicle{
    int id;
    double speed;
    double temp;
    const char* status;
};

/*判定関数*/
//const char* check_safety(double s,double t){
int check_safety(double s, double t){
if(s > 100.0 || t > 100.0){

//        return "WARNING";
        return 1;\
    }
//    return "NORMAL"a;
    return 0;
};

/*ファイル保存関数*/
int save_file(struct Vehicle v){

    FILE *file = fopen("fleet_log.txt", "a");

    if(file != NULL){
        fprintf(file, "ID:%d | ", v.id);
        fprintf(file, "STATUS:%-7s | ",v.status);
        fprintf(file, "Speed:%.1f, Temp:%.1f\n",v.speed ,v.temp);
        fclose(file);
        printf("-> 記録しました。");
        return 1;
    }
    return 0;
};

int main(){
    struct Vehicle v;
    FILE *file;

    printf("===車両連続診断ツール(-1入力で終了)===\n\n");

    while(1){
        printf("\n車両ID(終了は-1):");
        scanf("%d", &v.id);
        
        if(v.id == -1){
            printf("入力を終了します。\n");
                break;
        }

    printf("速度(km/h):");
    scanf("%lf", &v.speed);
    printf("温度(度):");
    scanf("%lf", &v.temp);

//    v.status = check_safety(v.speed, v.temp);
    int result = check_safety(v.speed, v.temp);
    if(1 == result){
        v.status = "WARNING";
    }
    else{
        v.status = "NOEMAL";
    }

   result =  save_file(v);
    if(1 == result){
        printf("ファイル書き込み成功");
    }
    else{
        printf("ファイル書き込み失敗");
    }

    }
    return 0;
}
