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
    int error_count;    //連続でエラーした回数をカウント

} Sensor;

/**************
 * 構造体：kalmanFilter
 * カルマンフィルタで利用する構造体
****************/
typedef struct{
    double speed;
    double var;
    double q;
}KalmanFilter;

/*--- プロトタイプ宣言 ---*/
double update_kalman(KalmanFilter *kf, double observation, double sensor_var);
double update_sensor(Sensor *s,double val);         //センサーのアップデート
int check_sensor_status(Sensor *s,double raw_diff); //センサーの故障判断

/*--- 関数 ---*/
/*************
*カルマンフィルタ
* 　引数:double observation、sensor_var
* 戻り値:
**************/
double update_kalman(KalmanFilter *kf, double observation, double sensor_var){

    //step1 予測(共通)
    double predict_speed = kf->speed;//前回の速度をベースにする
    double predict_var = kf->var + kf->q;//時間が立つほど、予測は少しずつ不正確になる。(誤差を増やす)
    //step2 更新
    //重み付けを予測とセンサーの間でおこなう
    double k_gain = predict_var / (predict_var + sensor_var);//カルマンゲイン
    kf->speed = predict_speed + k_gain * (observation - predict_speed);
    //次回のための結果を保存
    kf->var = (1.0 - k_gain) * predict_var;//情報を得たので誤差は小さくなる

    return kf->speed;
}

/*************
*センサーのアップデートをおこなう関数(リングバッファ)
* 　引数:double val(センサー情報)
* 戻り値:バッファ内の平均値
**************/
double update_sensor(Sensor *s,double val){
    s->buffer[s->index] = val;  //index番号に新しい値を入れる
    s->index = (s->index + 1) % BUFFER_SIZE;    //次に書き込む場所をずらす3%3 になると余りが0になり戻る
                                                //これによって常に最新の3つの情報のみを保持できる。(古いデータから消していく)
    if(s->count < BUFFER_SIZE) s->count++;      //バッファが埋まったかを数える(BAFFER_SIZEまでカウントして、それ以降は増えない)

    double sum = 0;
    for (int i = 0; i < s->count; i++){
        sum += s->buffer[i];//バッファ内にある合計の計算
    }
    return sum / s->count;//カウントした数で割って、平均値を出す
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
        printf("[WARNING]\n");
        s->error_count++;
    }
    if(s->error_count >= ERROR_THRESHOLD){
        printf("[SYSTEM FAILURE]\n");
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
    KalmanFilter kf = {0.0, 1.0, 0.2};    //カルマンフィルタの初期化
    double w_in = 0, g_in = 0, a_in = 0;    //入力した値
    double raw_diff = 0,avg_diff = 0,avg_w = 0,avg_g = 0,avg_a = 0;//平均値
    double fusion_speed =0; //合計したスピード
    double w_weight = 0.4; //WHEELの信頼度
    double g_weight = 0.4; //GPSの信頼度
    double a_weight = 0.2; //ACCELの信頼度
    double w_var = 0.1;//1.0;  //WHEELは正確(誤差１)
    double g_var = 0.5;//25.0; //GPSはフラフラ(誤差25)
    double a_var = 1.0;//100.0; //加速度は結構ガタガタ(ドリフトしやすい想定)


    printf("センサー入力を開始：\n");

    while(1){
        printf("\n[WHEEL][GPS][ACCEL]>>\n");
        if(scanf("%lf %lf %lf", &w_in, &g_in, &a_in) != 3) break; //3種類のセンサーデータを入力

        double diff_wg = (w_in > g_in) ? (w_in - g_in) : (g_in - w_in);
        double diff_ga = (g_in > a_in) ? (g_in - a_in) : (a_in - g_in);
        double diff_aw = (a_in > w_in) ? (a_in - w_in) : (w_in - a_in);
        double observation = 0;
        double current_sensor_var = 0;        

        if(diff_wg <= MAX_DIFF){
            //今のセンサー同士の重み付けを計算
            w_weight = g_var / (w_var + g_var);
            g_weight = w_var / (w_var + g_var);
            //まずはセンサー同士で「今の正解」を出す(実測値を出す)
            observation = (w_in * w_weight + g_in * g_weight) / (w_weight + g_weight);
            current_sensor_var = g_var;//代表してGPSの誤差を使用

            //WHEELとGPSが近いならその平均を信じる
            //Accelだけが仲間外れなので、aにだけ「ズレ(diff_a)」を教えてあげる
            double diff_a = (observation > a_in) ? (observation - a_in) : (a_in - observation);
    
            check_sensor_status(&wheel_speed, 0);      // 潔白
            check_sensor_status(&gps_speed, 0);        // 潔白
            //以下疑いあり
            //diff_aのチェック結果より1が返ってきていたら故障判定
            if(check_sensor_status(&accel_speed, diff_a) == 1){
                printf("\n[ERROR]：ACCELセンサー故障！\n");
                break;
            }
        }
        
        else if(diff_ga <= MAX_DIFF){
            /* カルマンフィルタ(GPS/ACCEL Ver) */
            g_weight = a_var / (g_var + a_var);
            a_weight = g_var / (g_var + a_var);
            //GPSとaccelが近いならその平均を信じる
            observation = (g_in * g_weight + a_in * a_weight) / (g_weight + a_weight); //判定ロジック
            current_sensor_var = a_var;//代表してACCELの誤差を使用

            //Wheelだけが仲間外れなので、wにだけ「ズレ(diff_w)」を教えてあげる
            double diff_w = (observation > w_in) ? (observation - w_in) : (w_in - observation);
    
            check_sensor_status(&gps_speed, 0);        // 潔白
            check_sensor_status(&accel_speed, 0); // 潔白
            // 以下疑いあり
            //diff_wのチェック結果より1が返ってきていたら故障判定
            if(check_sensor_status(&wheel_speed, diff_w) == 1){
                printf("\n[ERROR]：WHEELセンサー故障！\n");
                break;
            }
        }
        else if(diff_aw <= MAX_DIFF){
            /* カルマンフィルタ(ACEEL/WHEEL ver) */
            a_weight = w_var / (a_var + w_var);
            w_weight = a_var / (a_var + w_var);
            //accelとwheelが近いならその平均を信じる
            observation = (a_in * a_weight + w_in * w_weight) / (a_weight + w_weight); //判定ロジック
            current_sensor_var = w_var;//代表してWHEELの誤差を使用
            // GPSだけが仲間外れなので、gにだけ「ズレ(diff_g)」を教えてあげる
            double diff_g = (observation > g_in) ? (observation - g_in) : (g_in - observation);
    
            check_sensor_status(&wheel_speed, 0);      // 潔白
            check_sensor_status(&accel_speed, 0); // 潔白
            // 以下疑いあり
            //diff_gのチェック結果より1が返ってきていたら故障判定
            if(check_sensor_status(&gps_speed, diff_g) == 1){
                printf("\n[ERROR]：GPSセンサー故障！\n");
                break;
            }
        }
        else{
            //全ての値(GPW,WHEEL,ACCLEの値)がバラバラ
            printf("\n[ERROR]:正しい値が不明です。終了します\n");
            break;
        }
        
        fusion_speed = update_kalman(&kf, observation, current_sensor_var);//カルマンフィルタに掛ける

        //check_sensor_statusで問題ないことが判断できたため、アップデートしていく
        avg_w = update_sensor(&wheel_speed, w_in);
        avg_g = update_sensor(&gps_speed, g_in);
        avg_a = update_sensor(&accel_speed, a_in);

        //構造体内の平均情報をそれぞれ出力
        printf("[Average] WHEEL:%.2f[km/h], GPS: %.2f[km/h], ACCEL: %.2f[km/h]\n", avg_w, avg_g, avg_a);

        printf("[FUSION] 統合速度：%.2f[km/h]\n", fusion_speed);
    }
    return 0;
}