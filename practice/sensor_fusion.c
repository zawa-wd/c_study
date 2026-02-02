#include <stdio.h>

/*--- 定数設定 ---*/
#define MAX_DIFF 10.0           //平均値が左数値以上はErrorとする
#define ERROR_THRESHOLD 5       //Errorが連続して左の数回あった場合、故障とみなす
#define BUFFER_SIZE 3           //構造体のサイズ

/**************
 * 構造体：buffer
 * センサーのデータ構造を一定にすることで処理を重複させないようにする
****************/
typedef struct{
    double buffer[BUFFER_SIZE];
    int index;
    int count;
    int error_count;

} Sensor;

/*--- プロトタイプ宣言 ---*/
double update_sensor(Sensor *s,double val);         //センサーのアップデート
int check_sensor_status(Sensor *s,double raw_diff); //センサーの故障判断


/*--- 関数 ---*/

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

/*************
*故障判定関数
* 引　数:Sensor *s(判定したいセンサー)、double raw_diff(現在の差)
* 戻り値:1で故障判定。0で正常
**************/
int check_sensor_status(Sensor *s, double raw_diff){

    //MAX_DIFF以上の差がある場合は構造体への侵入を弾く
    if(raw_diff <= MAX_DIFF){
        s->error_count = 0;
    }
    else{
        printf("\n[SKIP]\n");
        s->error_count++;
    }
    if(s->error_count >= ERROR_THRESHOLD){
        printf("\n[SYSTEM FAILURE]\n");
        return 1;
    }
    return 0;

}

/**************
*メインルーチン 
***************/
int main(){
    Sensor wheel_speed = {{0}, 0, 0, 0};   //ホイールの回転数で車速を確認している想定
    Sensor gps_speed = {{0}, 0, 0, 0};     //GPS情報を利用して車速を確認している想定
    Sensor accel_speed = {{0}, 0, 0, 0};    //加速度から計算した速度を想定
    double w_in = 0, g_in = 0, a_in = 0;    //入力した値
    double raw_diff = 0,avg_diff = 0,avg_w = 0,avg_g = 0,avg_a = 0;//平均値
    double fusion_speed =0; //合計したスピード

    printf("センサー入力を開始：\n");

    while(1){
        printf(">>\n");
        if(scanf("%lf %lf %lf", &w_in, &g_in, &a_in) != 3) break; //3種類のセンサーデータを入力

        double diff_wg = (w_in > g_in) ? (w_in - g_in) : (g_in - w_in);
        double diff_ga = (g_in > a_in) ? (g_in - a_in) : (a_in - g_in);
        double diff_aw = (a_in > w_in) ? (a_in - w_in) : (w_in - a_in);

        if(diff_wg <= MAX_DIFF){
            //WHEELとGPSが近いならその平均を信じる
            fusion_speed = (w_in + g_in) /2.0;
            // Accelだけが仲間外れなので、aにだけ「ズレ(diff_a)」を教えてあげる
            double diff_a = (fusion_speed > a_in) ? (fusion_speed - a_in) : (a_in - fusion_speed);
    
            check_sensor_status(&wheel_speed, 0);      // 潔白
            check_sensor_status(&gps_speed, 0);        // 潔白
            // 以下疑いあり
            //diff_aのチェック結果より1が返ってきていたら故障判定
            if(check_sensor_status(&accel_speed, diff_a) == 1){
                printf("[WARNING]：ACCELセンサー故障！\n");
                break;
            }
        }
        else if(diff_ga <= MAX_DIFF){
            //GPSとaccelが近いならその平均を信じる
            fusion_speed = (g_in + a_in) /2.0;
            // Wheelだけが仲間外れなので、wにだけ「ズレ(diff_w)」を教えてあげる
            double diff_w = (fusion_speed > w_in) ? (fusion_speed - w_in) : (w_in - fusion_speed);
    
            check_sensor_status(&gps_speed, 0);        // 潔白
            check_sensor_status(&accel_speed, 0); // 潔白
            // 以下疑いあり
            //diff_wのチェック結果より1が返ってきていたら故障判定
            if(check_sensor_status(&wheel_speed, diff_w) == 1){
                printf("[WARNING]：WHEELセンサー故障！\n");
                break;
            }
        }
        else if(diff_aw <= MAX_DIFF){
            //accelとwheelが近いならその平均を信じる
            fusion_speed = (a_in + w_in) /2.0;
            // GPSだけが仲間外れなので、gにだけ「ズレ(diff_g)」を教えてあげる
            double diff_g = (fusion_speed > g_in) ? (fusion_speed - g_in) : (g_in - fusion_speed);
    
            check_sensor_status(&wheel_speed, 0);      // 潔白
            check_sensor_status(&accel_speed, 0); // 潔白
            // 以下疑いあり
            //diff_gのチェック結果より1が返ってきていたら故障判定
            if(check_sensor_status(&gps_speed, diff_g) == 1){
                printf("[WARNING]：GPSセンサー故障！\n");
                break;
            }
        }
        else{
            //全ての値(GPW,WHEEL,ACCLEの値)がバラバラ
            printf("[WARNING]:正しい値が不明です\n");
            break;
        }

        //上記で問題ないことが判断できたため、アップデートしていく
        avg_w = update_sensor(&wheel_speed, w_in);
        avg_g = update_sensor(&gps_speed, g_in);
        avg_a = update_sensor(&accel_speed, a_in);

        //構造体内の平均情報をそれぞれ出力
        printf(" [Average] Wheel:%.2f, GPS: %.2f, Accel: %.2f\n", avg_w, avg_g, avg_a);

        printf("[FUSION] 統合速度：%.2f km/h\n", fusion_speed);
    }
    return 0;
}