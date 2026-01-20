#include <stdio.h>

struct Vehicle {
    int id;
    double speed;
    double temp;

};

int main(){

    struct Vehicle my_car;

    printf("=== 車両データ手動入力 ===\n");

    printf("Please my car ID (ex:101):");
    scanf("%d",&my_car.id);
 
    printf("Please my car speed (km/h):");
    scanf("%lf", &my_car.speed);

    printf("現在の温度を入力してください(度): ");
    scanf("%lf", &my_car.temp);

    printf("\n--- 診断結果 ---\n");
    printf("車両ID: %d を診断しました。\n", my_car.id);

    if(my_car.speed > 100.0 || my_car.temp > 100.0){
        printf("結果：[WARNING]異常値を検知しました。");

    }

    else{
        printf("結果：[正常]問題ありません。");
    }

    return 0;
}
