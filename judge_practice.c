#include <stdio.h>

int main(){
    //テスト用の速度データ
    double speeds[4] = {120.5, 80.0, 15.2, -5.0};

    printf("===走行モード半定システム===\n");

    for(int i=0;i<4;i++){
        double s = speeds[i];
        printf("データ[%d]:%.1f km/h ->",i,s);

        //多分岐の判定(else if を使います)
        if (s >= 100.0){
            printf("[高速走行]高速道路を巡航中です。\n");
        }
        else if(s >= 40.0){
            printf("[通常走行]街中を走行中です。\n");
        }
        else if(s >= 0.0){
            printf("[低速走行]除行、または停車に近いです。\n");
        }
        else {
            //0未満は異常値としてあつかう
            printf("[Error]不正な速度データです！\n");
        }
    }
    return 0;
}
