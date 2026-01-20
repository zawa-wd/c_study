#include <stdio.h>

struct Vehicle {
    int id;
    double speed;
    double temp;
};

int main() {
    struct Vehicle fleet[3]={
        {101, 120.5, 105.0},
        {102, 80.0, 85.0},
        {103, 15.2, 70.0}
    };
    
    //▽集計用の変数
    int high_speed_count = 0;
    double total_temp = 0.0;

    printf("===車両診断開始===\n\n");

    for(int i = 0; i < 3; i++){
    //
    printf("ID:%d ",fleet[i].id);
    
    if(fleet[i].speed >= 100.0){
        printf("[高速]");
        high_speed_count++;
    }
    else{
        printf("[通常]");
    }
    printf("温度:%.1f度\n", fleet[i].temp);
    total_temp += fleet[i].temp;

    }
    //まとめ
    printf("\n===本日の集計レポート===\n");
    printf("高速走行中の車両:%d 台\n", high_speed_count);
    printf("全車両の平均温度:%.1f 度\n", total_temp / 3);

    return 0;
}
