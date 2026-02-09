#include "sensor_fusion.h"

/*--- プロトタイプ宣言 ---*/
void get_fused_observation(Sensor sns[], double current_kf_speed, double *obs, double *var);    //多数決とペアの選択関数
void process_sensor_fusion(double in1, double in2, double in3, double var1, double var2, Sensor *s1, Sensor *s2, Sensor *s3, double *obs, double *cur_var); //センサーステータス更新用関数
int check_sensor_status(Sensor *s,double raw_diff); //センサーの故障判断
int select_best_pair(double d_wg, double d_ga, double d_aw, int s_wg, int s_ga, int s_aw); //センサーの正しいらしいベアを判断
double calculate_kalman(double current_val, double *current_var, double obs, double obs_var, double q); //汎用カルマンフィルタ
void update_sensor(Sensor *s,double val);         //センサーのアップデート
double find_best_sensor(Sensor sensors[], int num_sensors, double predicted_speed);//救済関数

/*--- 関数 ---*/

/************************************************************
 *多数決とペアの選択関数(センサーフュージョンの中核)
 *引　数：sns:センサーデータ
         current_kf_speed:現在の推定速度(カルマンフィルタが前回算出した予測値)…これまでの流れに近いものと比較するために利用
         obs:観測値の器。最終的にこの速度という報告をするために利用
         var:分散・不安の器。自身のなさを書き込む。こんかいのカルマンフィルタについて、信用していいのかを伝えるために利用
 *返り値：void
 ************************************************************/
void get_fused_observation(Sensor sns[], double current_kf_speed, double *obs, double *var){
    //多数決の準備
    double w_in = sns[0].current_val;
    double g_in = sns[1].current_val;
    double a_in = sns[2].current_val;
    //差分の計算　fabs…数値を+にする。
    double diff_wg = fabs(w_in - g_in);
    double diff_ga = fabs(g_in - a_in);
    double diff_aw = fabs(a_in - w_in);
    //ペアごとのエラー合計を計算
    int score_wg = sns[0].error_count + sns[1].error_count;
    int score_ga = sns[1].error_count + sns[2].error_count;
    int score_aw = sns[2].error_count + sns[0].error_count;
    //モードの選択
    int mode = select_best_pair(diff_wg, diff_ga, diff_aw, score_wg, score_ga, score_aw);

    //それぞれの選ばれたモードでも、急激な速度変化(20km/h)がある場合、モードを-1として[EMERGENCY]を宣言する(最後のELSEへ飛ぶ)
    if(mode == 0 && fabs(w_in - current_kf_speed) > 20.0) mode = -1;
    if(mode == 1 && fabs(g_in - current_kf_speed) > 20.0) mode = -1;
    if(mode == 2 && fabs(a_in - current_kf_speed) > 20.0) mode = -1;

    //WHEELとGPSのセンサーペアを採用する場合
    if(mode == 0){
        process_sensor_fusion(w_in, g_in, a_in, sns[0].variance, sns[1].variance, &sns[0], &sns[1], &sns[2], obs, var);
    }
    //GPSとACCELのセンサーペアを採用する場合
    else if(mode == 1){
        process_sensor_fusion(g_in, a_in, w_in, sns[1].variance, sns[2].variance, &sns[1], &sns[2], &sns[0], obs, var);
    }
    //ACCELとWHEELのセンサーペアを採用する場合
    else if(mode == 2){
        process_sensor_fusion(a_in, w_in, g_in, sns[2].variance, sns[0].variance, &sns[2], &sns[0], &sns[1], obs, var);
    }

    //全ての値のどれが正しいかが分からない場合
    else{
        printf("\n ->[EMERGENCY]:汎用関数で救済を試みます\n");
        *obs = find_best_sensor(sns, 3, current_kf_speed);

        //　ACCELのズレがWHEELのズレよりも小さく、かつGPSのズレよりも小さく、
        //　更にそのズレが20km/h以内(加速度として20km/hはないとの判断)である場合にしている
        // 　　->よって、ACCELがこの中でマシな数値を持っていると判断できる
        if(fabs(*obs - current_kf_speed) < MAX_DIFF * 2){
            *var = A_VAR * 10.0;//市立でセンサ信頼度低め
            printf(" ->[RESCUE]: %f を採用\n",*obs);
        }
        //全てがだめな場合、とりあえず予測速度を提示
        else{
            *obs = current_kf_speed;
            *var = 100.0;
            printf(" ->[RESCUE]:全センサーを拒否。予測のみで維持\n");
        }
    }
}

/************************************************************
 *選択されたペア(正しいと判断されたペア)に基づいて、実測値の計算と各センサーのステータス更新をおこなう関数
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

/************************************************************
*ベストなセンサーペアを選択する関数
*引　数：d_wg,ga,aw:それぞれのズレ計算結果、s_wg,ga,aw:ペアごとのエラー総数
*戻り値：0(W&G),1(G&A)2(A&W),-1(全滅)
************************************************************/
int select_best_pair(double d_wg,double d_ga, double d_aw, int s_wg, int s_ga, int s_aw){
    //まずは差が許容範囲のペアを絞る
    int best_mode = -1;
    int main_score = 999;

    //WHEEL&GPSチェック
    if(d_wg <= MAX_DIFF && s_wg < main_score){
        main_score = s_wg;
        best_mode = 0;
    }
    //GPS&ACCELチェック
    if(d_ga <= MAX_DIFF && s_ga < main_score){
        main_score = s_ga;
        best_mode = 1;
    }
    //ACCEL&WHEELチェック
    if(d_aw <= MAX_DIFF && s_aw < main_score){
        main_score = s_aw;
        best_mode = 2;
    }
    return best_mode;
}

/****************************************
* 汎用カルマンフィルタ計算関数
* 引　数:double current_val: 前回の計算結果
        double *current_var:前回の自身のなさ→計算後に更新するためポインタ渡し
        double obs:         今回の実測値→フュージョンで作った速度
        double obs_var:     センサーの自身のなさ
        double q:           時間の経過による不安度合い
* 戻り値:updated_val:推定速度
****************************************/
double calculate_kalman(double current_val, double *current_var, double obs, double obs_var, double q){
    //step1 予測　前回の値を渡し、時間がたった分だけの不安度を追加
    double predict_val = current_val;
    double predict_var = (*current_var) + q; //時間が立つほど、予測は少しずつ不正確になる。(誤差を増やす→q)

    //step2 更新　predict_varとobs_varを比べてどっちがマシかな？
    //重み付けを予測とセンサーの間でおこなう
    double k_gain = predict_var / (predict_var + obs_var);           //カルマンゲインの計算(どのくらいセンサーの言うことを聞くのか)
    double updated_val = predict_val + k_gain * (obs - predict_val); //予測と実測のズレを足す(ゲインでどのくらい足すかを調整している)
    *current_var = (1.0 - k_gain) * predict_var;                     //情報を得たので誤差は小さくなる

    return updated_val;
}

/**************************************************
* センサー値のアップデート（移動平均用バッファ更新）
* 引　数: Sensor *s, double val
* 戻り値: 平均値
**************************************************/
void update_sensor(Sensor *s, double val) {
    s->current_val = val;
    s->buffer[s->index] = val;
    s->index = (s->index + 1) % BUFFER_SIZE;
    if (s->count < BUFFER_SIZE) s->count++;
    
    double sum = 0;
    for (int i = 0; i < s->count; i++) sum += s->buffer[i];
    s->current_val = sum / s->count;
}


/************************************************************
*多数決が崩壊した時、予測速度と類似するセンサーを探し出す関数（汎用化）
*引　数：sensors         センサーそれぞれの値
        num_sensors     センサーの数
        predicted_speed 予測の速度
*戻り値：利用したいセンサーまたは予測値の速度を返す
************************************************************/
double find_best_sensor(Sensor sensors[], int num_sensors, double predicted_speed){
    //最悪の事態に備え、初期値の設定
    //センサーのどれもが信じれない場合、前回の予測を返すようにする
    double best_val = predicted_speed;
    double min_diff = 999.0;

    //全てのセンサーを順番にチェック
    for(int i = 0; i < num_sensors; i++){
        double diff = fabs(sensors[i].current_val - predicted_speed);//今と予測の速度差を計算
        //一番マシなやつの更新(ただしMAX_DIFFの２倍以内)
        if(diff < min_diff && diff < MAX_DIFF * 2){
            min_diff = diff;
            best_val = sensors[i].current_val;
        }
    }
    return best_val;
}
