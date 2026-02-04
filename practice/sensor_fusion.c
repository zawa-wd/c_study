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
#define PROCESS_NOISE 0.2   //カルマンフィルタの初期値　予測がどれくらいズレているか
#define SIMULATOR_VIRTUAL_SPEED 0   //[SIMULATOR]初速度
#define SIMULATOR_W_NOISE 0.2       //[SIMULATOR]WHEELのノイズ
#define SIMULATOR_G_NOISE -0.5      //[SIMULATOR]GPSのノイズ
#define SIMULATOR_A_NOISE 1.0       //[SIMULATOR]ACCELのノイズ

/**************************************************
 * 構造体：buffer
 * センサーのデータ構造を一定にすることで処理を重複させないようにする
**************************************************/
typedef struct{
    double buffer[BUFFER_SIZE];
    int index;
    int count;
    int error_count;    //連続でエラーした回数をカウント

} Sensor;

/******************************
 * 構造体：kalmanFilter
 * カルマンフィルタで利用する構造体
******************************/
typedef struct{
    double speed;   //推定速度
    double var;     //推定誤差
    double q;       //プロセスノイズ
}KalmanFilter;

/*--- プロトタイプ宣言 ---*/
double update_kalman(KalmanFilter *kf, double observation, double sensor_var); //カルマンフィルタ
double update_sensor(Sensor *s,double val);         //センサーのアップデート
int check_sensor_status(Sensor *s,double raw_diff); //センサーの故障判断
int select_best_pair(double d_wg, double d_ga, double d_aw, int s_wg, int s_ga, int s_aw); //センサーの正しいらしいベアを判断
void process_sensor_fusion(double in1, double in2, double in3, double var1, double var2, Sensor *s1, Sensor *s2, Sensor *s3, double *obs, double *cur_var); //センサーステータス更新用関数
double simulate_sensor(double target_speed, double noise_level);//センサーの値をシミュレート生成する関数

/*--- 関数 ---*/
/************************************************************
 *選択されたペアに基づいて、実測値の計算と各センサーのステータス更新をおこなう関数
 *引　数：in1,2,3:入力された値、var1,2:選択されたペアのそれぞれの分散、s1,2,3:センサーの値
　　　　 [書き込み用]obs:統合された実測値、cur_var:選択されたペアの信頼性（分散）
 *返り値：void
 ************************************************************/
void process_sensor_fusion(double in1, double in2, double in3, double var1, double var2, Sensor *s1, Sensor *s2, Sensor *s3, double *obs, double *cur_var){

    //センサー同士の分散(誤差)に基づいて、どちらをより信じるか重みを計算
    double w1 = var2 / (var1 + var2);
    double w2 = var1 / (var1 + var2);
    
    *obs = (in1 * w1 + in2 * w2) / (w1 + w2); //センサーのペアを混ぜ合わせて一つのもっともらしい実測値をつくる
    *cur_var = var2;    //カルマンフィルタに伝える今回の実測値の誤差

    //正常ペアはエラーカウントをリセット
    check_sensor_status(s1,0);
    check_sensor_status(s2,0);

    //仲間はずれのS3のズレを計算してチェック
    double diff3 = (*obs > in3) ? (*obs - in3) : (in3 - *obs);

    //以下ズレの疑いがあるためチェック
    //diff3のチェック結果より１が返ってきていた場合、故障判定
    if(check_sensor_status(s3, diff3)==1){
        printf("\n ->[ERROR]:センサーの故障を検知\n");
    }
}

/****************************************
*カルマンフィルタ
* 引　数:double observation、sensor_var
* 戻り値:kf-speed 推定速度
****************************************/
double update_kalman(KalmanFilter *kf, double observation, double sensor_var){
    //step1 予測
    double predict_speed = kf->speed;       //速度をベースにする
    double predict_var = kf->var + kf->q;   //時間が立つほど、予測は少しずつ不正確になる。(誤差を増やす)

    //step2 更新
    //重み付けを予測とセンサーの間でおこなう
    double k_gain = predict_var / (predict_var + sensor_var);   //カルマンゲイン
    kf->speed = predict_speed + k_gain * (observation - predict_speed);
    kf->var = (1.0 - k_gain) * predict_var; //情報を得たので誤差は小さくなる

    return kf->speed;
}

/****************************************
* センサーのアップデートをおこなう関数(リングバッファ)
* 引　数:double val(センサー情報)
* 戻り値:バッファ内の平均値
****************************************/
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

/**************************************************
* 故障判定関数
*   MAX_DIFF以上の差がある場合は構造体への侵入を弾く
* 引　数:Sensor *s(判定したいセンサー)、double raw_diff(現在の差)
* 戻り値:1で故障判定。0で正常
**************************************************/
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
//WGチェック

/************************************************************
*ベストなセンサーペアを選択する関数
*引　数：d_wg,ga,aw:それぞれのズレ計算結果、s_wg,ga,aw:ペアごとのエラー総数
*戻り値：0(W&G),1(G&A)2(A&W),-1(全滅)
************************************************************/
int select_best_pair(double d_wg,double d_ga, double d_aw, int s_wg, int s_ga, int s_aw){
    //まずは差が許容範囲のペアを絞る
    int best_mode = -1;
    int main_score = 999;

    //W&Gチェック
    if(d_wg <= MAX_DIFF && s_wg < main_score){
        main_score = s_wg;
        best_mode = 0;

    }
    //G&Aチェック
    if(d_ga <= MAX_DIFF && s_ga < main_score){
        main_score = s_ga;
        best_mode = 1;
    }
    //A&Wチェック
    if(d_aw <= MAX_DIFF && s_aw < main_score){
        main_score = s_aw;
        best_mode = 2;
    }
    return best_mode;
}

/******************************
*センサーの値をシミュレーション生成する関数
*引　数：target_speed:本来あるべき車速、
　　　　 noise_level:センサーのガタツキ具合(本来乱数を使うが今回は固定値)
*戻り値：
******************************/
double simulate_sensor(double target_speed, double noise_level){
    return target_speed + noise_level;
}

/**********
*メインルーチン 
**********/
int main(){
    Sensor wheel_speed = {{0}, 0, 0, 0}; //ホイールの回転数で車速を確認している想定
    Sensor gps_speed =   {{0}, 0, 0, 0}; //GPS情報を利用して車速を確認している想定
    Sensor accel_speed = {{0}, 0, 0, 0}; //加速度から計算した速度を想定
    KalmanFilter kf =   {0.0, INITIAL_VAR, PROCESS_NOISE}; //カルマンフィルタの初期化
    double w_in = 0, g_in = 0, a_in = 0;             //入力値(センサーからの値)
    double avg_w = 0,avg_g = 0,avg_a = 0;            //平均値
    double w_weight = 0, g_weight = 0, a_weight = 0; //信頼度
    double fusion_speed =0;     //カルマンフィルタを考慮したスピード

    printf("センサー入力を開始：\n");
    /* ＝＝＝ SIMULATOR ＝＝＝ */
    printf("＝＝＝ SIMULATOR START ＝＝＝\n");
    double virtual_speed = SIMULATOR_VIRTUAL_SPEED;//シミュレーション時の初速度

    for(int i = 0; i < 50; i++){
        virtual_speed += 1.0;//マイステップ1km/hずつ加速

        w_in = simulate_sensor(virtual_speed, SIMULATOR_W_NOISE);//ノイズを加えたWHEELの入力値
        a_in = simulate_sensor(virtual_speed, SIMULATOR_A_NOISE);//ノイズを加えたACCELの入力値

        //【シナリオ】20回目からGPSが故障して150km/hに固定される
        if(i >= 20){
            g_in = 150;
        }
        else{
            g_in = simulate_sensor(virtual_speed, SIMULATOR_G_NOISE);//ノイズを加えたGPSの入力値
        }
    /* ＝＝＝ SIMULATOR ＝＝＝ */
    //シミュレーターは以下の4行、｝、wile(1)、print、ifだけコメントアウトして使用
//    }
//    while(1){
//        printf("\n入力：[WHEEL],[GPS],[ACCEL]>>\n");
//        if(scanf("%lf %lf %lf", &w_in, &g_in, &a_in) != 3) break; //3種類のセンサーデータを入力

        //各センサーのズレを計算(多数決の準備)
        double diff_wg = (w_in > g_in) ? (w_in - g_in) : (g_in - w_in);
        double diff_ga = (g_in > a_in) ? (g_in - a_in) : (a_in - g_in);
        double diff_aw = (a_in > w_in) ? (a_in - w_in) : (w_in - a_in);

        double observation = 0;         //統合された「今この瞬間の実測値」
        double current_sensor_var = 0;  //採用されたセンサーペアの「信頼性(分散)」

        //ペアごとのエラー合計を計算
        int score_wg = wheel_speed.error_count + gps_speed.error_count;
        int score_ga = gps_speed.error_count + accel_speed.error_count;
        int score_aw = accel_speed.error_count + wheel_speed.error_count;

        int mode = select_best_pair(diff_wg, diff_ga, diff_aw, score_wg, score_ga, score_aw);

        //WHEELとGPSのセンサーペアを採用する場合
        if(mode == 0){
            process_sensor_fusion(w_in, g_in, a_in, W_VAR, G_VAR, &wheel_speed, &gps_speed, &accel_speed, &observation, &current_sensor_var);
        }
        
        //GPSとACCELのセンサーペアを採用する場合
        else if(mode == 1){
            process_sensor_fusion(g_in, a_in, w_in, G_VAR, A_VAR, &gps_speed, &accel_speed, &wheel_speed, &observation, &current_sensor_var);
        }
        //ACCELとWHEELのセンサーペアを採用する場合
        else if(mode == 2){
            process_sensor_fusion(a_in, w_in, g_in, A_VAR, W_VAR, &accel_speed, &wheel_speed, &gps_speed, &observation, &current_sensor_var);
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