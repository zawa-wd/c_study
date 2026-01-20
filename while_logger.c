#include <stdio.h>

struct Vehicle{
    int id;
    double speed;
    double temp;
    
};

int main(){
    struct Vehicle v;
    FILE *file;

    printf("=== 車両連続診断ツール(-1入力で終了)===\n");

    while(1){
        printf("車両ID(-1で入力終了)：");
        scanf("%d", &v.id);

        //-1
        if(v.id == -1){
            printf("入力を終了します。\n");
            break;
        }

        printf("速度(km/h):");
        scanf("%lf", &v.speed);
        printf("温度(度):");
        scanf("%lf", &v.temp);

        file = fopen("fleet_log.txt", "a");
        if(file != NULL){
            fprintf(file, "ID:%d | ",v.id);
            
            if(v.speed > 100.0 || v.temp > 100.0){
                fprintf(file, "[WARNING]");
            }

            else{
                fprintf(file, "[NORMAL]");
            }
            fprintf(file, "Speed:%.1f, Temp:%.1f\n",v.speed, v.temp);
            fclose(file);
            printf("->記録しました。\n");
        }
    }
    return 0;
}
