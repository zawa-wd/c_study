#include <stdio.h>

#define BUFFER_SIZE 3
/**************
 * 構造体：buffer
 * センサーのデータ構造を一定にすることで処理を重複させないようにする
****************/
typedef struct{
    double buffer[BUFFER_SIZE];
    int index;
    int count;

} Sensor;

/*************
*センサーのアップデートをおこなう関数
* 　引数:double val(センサー情報)
* 戻り値:void
**************/
double update_sensor(Sensor *s,double val){
    s->buffer[s->index] = val;
    s->index = (s->index + 1) % BUFFER_SIZE;
    if(s->count < BUFFER_SIZE) s->count++;

    double sum = 0;
    for (int i = 0; i < s->count; i++){
        sum += s->buffer[i];

    }
    return sum / s->count;
}

/**************
*メインルーチン 
***************/
int main(){
    Sensor wheel_speed = {{0}, 0, 0};   //ホイールの回転数で車速を確認している想定
    Sensor gps_speed = {{0}, 0, 0};     //GPS情報を利用して車速を確認している想定
    double w_in = 0, g_in = 0;
    double raw_diff = 0,avg_diff = 0,avg_w = 0,avg_g = 0; 

    printf("センサー入力を開始：\n");

    while(1){
        printf("> ");
        if(scanf("%lf %lf", &w_in,&g_in) != 2) break; //2種類のセンサーデータを入力

        raw_diff = (w_in > g_in) ? (w_in - g_in) : (g_in - w_in);//生データの速度差を計算
        
        //生データの時点で速度差が10以下の場合は使えるデータ化
        //10以上の差がある場合は構造体への侵入を弾く
        if(raw_diff <= 10){
            avg_w = update_sensor(&wheel_speed, w_in);
            avg_g = update_sensor(&gps_speed, g_in);
            avg_diff = (avg_w > avg_g) ? (avg_w - avg_g) : (avg_g - avg_w);
        }
        else{
            printf("\n[SKIP]\n\n");
        }

        //構造体内の平均情報をそれぞれ出力
        printf(" [Average] Wheel:%.2f, GPS: %.2f\n", avg_w, avg_g);

        //構造体内の平均での差を確認する
        double diff = avg_w - avg_g;

        
        if(diff < 0) diff = -diff;

        //RAWデータでの差分が10以上であった場合、WARNINGとして警告
        if(raw_diff > 10.0){
            printf("[WARNING][raw_diff](%.2f)\n",raw_diff);
        }

        //構造体内の平均でも差が5以上ある場合はWARNINGとして警告
        if(avg_diff > 5.0){
            printf("[WARNING][avg_diff](%.2f)\n",avg_diff);
        }

        //WARNING以外の場合、統合速度を出力
        else{
            printf("[FUSION]:統合速度＝%.2f km/h\n",(avg_w + avg_g)/2.0);
            printf("[NEW]\n");
            printf("[FUSION]:統合速度＝%.2f km/h\n",avg_diff);
            printf("[FUSION]:RAW統合＝%.2f km/h\n",raw_diff);
        }
    }
    return 0;
}