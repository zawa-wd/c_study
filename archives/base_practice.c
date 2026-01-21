#include <stdio.h>

int main(){
    //五回分の速度データ
    double speeds[5] = {40.2,42.5,39.8,44.1,41.4};
    double sum = 0.0;

    printf("---記録されたデータ---\n");
    for(int i=0;i<5;i++){
        printf("%d回目:%.1f km/h\n",i+1,speeds[i]);
        sum += speeds[i];//合計を計算

    }

    int count = sizeof(speeds)/sizeof(speeds[0]);
    double average = sum / count;
    printf("\n平均速度:%.2f km/h\n",average);

    return 0;    
}
