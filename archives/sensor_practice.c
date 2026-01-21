#include <stdio.h>

struct VehicleStatus {
	int id;
	double speed;
	double temp;

};

//2.データを更新する関数（ポインタを使用）
void update_status(struct VehicleStatus *v, double new_speed, double new_temp){
    v->speed = new_speed;
    v->temp = new_temp;
    printf("---データを更新しました---\n");
}

int main(){
    //構造体の実体を作る
    struct VehicleStatus car1 = {101, 0.0, 25.0};

    printf("更新前:ID:%d, 速度:%.1f, 温度:%.1f\n", car1.id, car1.speed, car1.temp);

    //関数の呼び出し（アドレスを渡すのがポイント！）
    update_status(&car1, 60.5, 85.3);

    printf("更新後:ID:%d, 速度:%.1f, 温度:%.1f\n",car1.id, car1.speed, car1.temp);
    return 0;
}
