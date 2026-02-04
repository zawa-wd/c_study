#include <stdio.h>

/*--- 定数設定 ---*/
//補足：_varは分散…分散は数値が大きいほど信頼できないというイメージ
#define MAX_DIFF 10.0        //平均値が左数値以上は正しくない可能性があると考え、他のセンサーデータを利用する
#define ERROR_THRESHOLD 5    //Errorが連続して左の数回あった場合、故障とみなす
#define BUFFER_SIZE 3        //構造体のサイズ
#define W_VAR 0.1            //WHEELの分散は正確(誤差１)
#define G_VAR 0.5            //GPSの分散はフラフラ(誤差25)
#define A_VAR 1.0            //加速度の分散は結構ガタガタ(ドリフトしやすい想定)
#define INITIAL_VAR 1.0     //カルマンフィルタの初期値　最初の自身のなさ
#define PROCESS_NOISE 5.0//0.2   //カルマンフィルタの初期値　予測がどれくらいズレているか

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
    double speed;   //推定速度
    double var;     //推定誤差
    double q;       //プロセスノイズ
}KalmanFilter;

/*--- プロトタイプ宣言 ---*/
double update_kalman(KalmanFilter *kf, double observation, double sensor_var); //カルマンフィルタ
double update_sensor(Sensor *s,double val);         //センサーのアップデート
int check_sensor_status(Sensor *s,double raw_diff); //センサーの故障判断

/*--- 関数 ---*/
/*************
*カルマンフィルタ
* 引　数:double observation、sensor_var
* 戻り値:kf-speed 推定速度
**************/
double update_kalman(KalmanFilter *kf, double observation, double sensor_var){
    //step1 予測
    double predict_speed = kf->speed;//速度をベースにする
    double predict_var = kf->var + kf->q;//時間が立つほど、予測は少しずつ不正確になる。(誤差を増やす)

    //step2 更新
    //重み付けを予測とセンサーの間でおこなう
    double k_gain = predict_var / (predict_var + sensor_var);   //カルマンゲイン
    kf->speed = predict_speed + k_gain * (observation - predict_speed);
    kf->var = (1.0 - k_gain) * predict_var;//情報を得たので誤差は小さくなる

    return kf->speed;
}

/*************
* センサーのアップデートをおこなう関数(リングバッファ)
* 引　数:double val(センサー情報)
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
* 故障判定関数
*   MAX_DIFF以上の差がある場合は構造体への侵入を弾く
* 引　数:Sensor *s(判定したいセンサー)、double raw_diff(現在の差)
* 戻り値:1で故障判定。0で正常
**************/
int check_sensor_status(Sensor *s, double raw_diff){
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
    Sensor wheel_speed = {{0}, 0, 0, 0}; //ホイールの回転数で車速を確認している想定
    Sensor gps_speed =   {{0}, 0, 0, 0}; //GPS情報を利用して車速を確認している想定
    Sensor accel_speed = {{0}, 0, 0, 0}; //加速度から計算した速度を想定
    KalmanFilter kf =   {0.0, INITIAL_VAR, PROCESS_NOISE}; //カルマンフィルタの初期化
    double w_in = 0, g_in = 0, a_in = 0;             //入力値(センサーからの値)
    double avg_w = 0,avg_g = 0,avg_a = 0;            //平均値
    double w_weight = 0, g_weight = 0, a_weight = 0; //信頼度
    double fusion_speed =0;     //カルマンフィルタを考慮したスピード

    printf("センサー入力を開始：");

    while(1){
        printf("\n入力：[WHEEL],[GPS],[ACCEL]>>\n");
        if(scanf("%lf %lf %lf", &w_in, &g_in, &a_in) != 3) break; //3種類のセンサーデータを入力

        //各センサーのズレを計算(多数決の準備)
        double diff_wg = (w_in > g_in) ? (w_in - g_in) : (g_in - w_in);
        double diff_ga = (g_in > a_in) ? (g_in - a_in) : (a_in - g_in);
        double diff_aw = (a_in > w_in) ? (a_in - w_in) : (w_in - a_in);

        double observation = 0;         //統合された「今この瞬間の実測値」
        double current_sensor_var = 0;  //採用されたセンサーペアの「信頼性(分散)」

        //WHEELとGPSのセンサーペアを採用する場合
        if(diff_wg <= MAX_DIFF){
            //センサー同士の分散(誤差)に基づいて、どちらをより信じるか重みを計算
            w_weight = G_VAR / (W_VAR + G_VAR);
            g_weight = W_VAR / (W_VAR + G_VAR);
            
            observation = (w_in * w_weight + g_in * g_weight) / (w_weight + g_weight);//センサーのペアを混ぜ合わせて一つのもっともらしい実測値をつくる
            current_sensor_var = G_VAR;//カルマンフィルタに伝える今回の実測値の誤差

            //Accelだけが仲間外れなので、aにだけ「ズレ(diff_a)」を教えてあげる
            double diff_a = (observation > a_in) ? (observation - a_in) : (a_in - observation);

            //正常ペアはエラーカウントをリセット
            check_sensor_status(&wheel_speed, 0);
            check_sensor_status(&gps_speed, 0);

            //以下疑いあるためチェック
            //diff_aのチェック結果より1が返ってきていたら故障判定
            if(check_sensor_status(&accel_speed, diff_a) == 1){
                printf("\n ->[ERROR]：ACCELセンサー故障！\n");
                current_sensor_var = G_VAR;//正常なGPSの分散をそのままつかう
                //break;//breakするようにしていたが、正常な値を入れて回すように改造
            }
        }
        
        //GPSとACCELのセンサーペアを採用する場合
        else if(diff_ga <= MAX_DIFF){
            //センサー同士の分散(誤差)に基づいて、どちらをより信じるか重みを計算
            g_weight = A_VAR / (G_VAR + A_VAR);
            a_weight = G_VAR / (G_VAR + A_VAR);
            
            observation = (g_in * g_weight + a_in * a_weight) / (g_weight + a_weight); //センサーのペアを混ぜ合わせて一つのもっともらしい実測値をつくる
            current_sensor_var = A_VAR;//カルマンフィルタに伝える今回の誤差

            //Wheelだけが仲間外れなので、wにだけ「ズレ(diff_w)」を教えてあげる
            double diff_w = (observation > w_in) ? (observation - w_in) : (w_in - observation);
    
            //正常ペアのエラーカウントをリセット
            check_sensor_status(&gps_speed, 0);
            check_sensor_status(&accel_speed, 0);

            //以下疑いありのためチェック
            //diff_wのチェック結果より1が返ってきていたら故障判定
            if(check_sensor_status(&wheel_speed, diff_w) == 1){
                printf("\n ->[ERROR]：WHEELセンサー故障！\n");
                current_sensor_var = A_VAR;
                //break;
            }
        }

        else if(diff_aw <= MAX_DIFF){
            //センサー同士の分散(誤差)に基づいて、どちらをより信じるか重みを計算
            a_weight = W_VAR / (A_VAR + W_VAR);
            w_weight = A_VAR / (A_VAR + W_VAR);
            
            observation = (a_in * a_weight + w_in * w_weight) / (a_weight + w_weight); //センサーのペアを混ぜ合わせて一つのもっともらしい実測値をつくる
            current_sensor_var = W_VAR;//カルマンフィルタに伝える今回の誤差

            // GPSだけが仲間外れなので、gにだけ「ズレ(diff_g)」を教えてあげる
            double diff_g = (observation > g_in) ? (observation - g_in) : (g_in - observation);

            //正常ペアのエラーカウントをリセット
            check_sensor_status(&wheel_speed, 0);
            check_sensor_status(&accel_speed, 0);
 
            // 以下疑いありのためチェック
            //diff_gのチェック結果より1が返ってきていたら故障判定
            if(check_sensor_status(&gps_speed, diff_g) == 1){
                printf("\n ->[ERROR]：GPSセンサー故障！\n");
                current_sensor_var = W_VAR;
                //break;
            }
        }

        else{
            //全ての値(GPW,WHEEL,ACCLEの値)がバラバラ
            printf("\n ->[ERROR]:正しい値が不明です。終了します\n");
            break;
        }
        
        fusion_speed = update_kalman(&kf, observation, current_sensor_var);//カルマンフィルタに掛ける

        //check_sensor_statusで問題ないことが判断できたため、アップデートしていく
        avg_w = update_sensor(&wheel_speed, w_in);
        avg_g = update_sensor(&gps_speed, g_in);
        avg_a = update_sensor(&accel_speed, a_in);

        //BUFFER内の平均情報をそれぞれ出力
        printf("[Average]    WHEEL:%.2f[km/h], GPS: %.2f[km/h], ACCEL: %.2f[km/h]\n", avg_w, avg_g, avg_a);
        printf("[FUSION] 統合速度：%.2f[km/h]\n", fusion_speed);
    }
    return 0;
}