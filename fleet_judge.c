#include <stdio.h>

//車両データの構造定義
struct Vehicle{
    int id;
    double speed;
};

int main(){
    //3台の車両データを準備
    struct Vehicle fleet[3] = {
        {101,120.5}, //高速
        {102, 45.0}, //通常
        {103, 15.2}  //低速
    };

    printf("===車両ステータス診斷レポート===\n\n");

    for(int i = 0;i<3;i++){
        printf("管理ID: %d | 現在速度: %.1f km/h\n",fleet[i].id, fleet[i].speed);
        
        //判定ロジックの合体
        printf("診斷結果:");
        if(fleet[i].speed >= 100.0){
            printf("[高速]\n");
        }
        else if(fleet[i].speed>=40.0){
            printf("[通常]\n");
        }
        else{
            printf("[低速]\n");
        }
        printf("------------------\n");
    }
    return 0;
};
