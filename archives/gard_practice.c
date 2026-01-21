#include <stdio.h>

int main(){
    //センターデータ(4番目に999.0という異常値が混ざっている想定)
    double speeds[5] = {40.2, 42.5, 39.8, 999.0, 41.4};
    double sum = 0.0;
    int valid_count = 0;//ただしいデータの数を入れる変数

    printf("---センサーデータ検証開始---\n");

    for(int i=0;i<5;i++){
        //ガード処理
        if(speeds[i] >= 0.0 && speeds[i] <= 150.0){
            printf("%d回目: %.1f km/h(正常) \n",i+1,speeds[i]);
            sum += speeds[i];
            valid_count++;//正常な時だけカウントアップ

        }
        else{
            printf("%d回目:%.1f km/h(異常値を検知:スキップします)\n",i+1,speeds[i]);
        }
    }
    
    if(valid_count > 0){
        double average = sum / valid_count;
        printf("\n:有効データ数:%d, 平均速度:%.2f km/h\n",valid_count, average);
    }
    else{
        printf("\n有効なデータがありませんでした。\n");
    }

    return 0;

}
