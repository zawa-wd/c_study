#include <stdio.h>

struct Vehicle{
    int id;
    double speed;
    double temp;
};

int main(){
    struct Vehicle v;
    FILE *file;

    printf("=== 車両診断ロギングツール ===\n");

    printf("車両ID:");
    scanf("%d", &v.id);
    printf("速度(km/h):");
    scanf("%lf", &v.speed);
    printf("温度(度):");
    scanf("%lf", &v.temp);

    file = fopen("fleet_log.txt", "a");
    if(file==NULL){
        printf("ファイルを開けませんでした。\n");
        return 1;
    }
    
    //3
    fprintf(file,"ID:%d | ", v.id);
    if(v.speed > 100.0 || v.temp > 100.0){
        fprintf(file,"[WARNING]");
    }
    else{
        fprintf(file, "[NOMAL]");
    }
    fprintf(file, "Speed:%.1f, Temp:%.1f\n", v.speed, v.temp);

    //4
    fclose(file);
    printf("\n診断完了。 fleet_log.txt に追記しました。\n");

    return 0;
}
