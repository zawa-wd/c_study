#include <stdio.h>
#include <math.h>

/*--- 定数設定 ---*/
//補足：_varは分散…分散は数値が大きいほど信頼できないというイメージ
#define MAX_DIFF 10.0        //平均値が左数値以上は正しくない可能性があると考え、他のセンサーデータを利用する
#define ERROR_THRESHOLD 5    //Errorが連続して左の数回あった場合、故障とみなす
#define BUFFER_SIZE 3        //構造体のサイズ
#define DT 0.1               //1ステップあたりの時間(秒)
#define W_VAR 0.01           //WHEELの分散は正確(誤差１)
#define G_VAR 0.5            //GPSの分散はフラフラ(誤差25)
#define A_VAR 1.0            //加速度の分散は結構ガタガタ(ドリフトしやすい想定)
#define Y_VAR 0.05           //ジャイロ(角速度)の分散(信頼度)
#define INITIAL_VAR 1.0      //カルマンフィルタの初期値　最初の自身のなさ
#define PROCESS_NOISE 10.0   //カルマンフィルタの初期値　予測がどれくらいズレているか
#define SIMULATOR_VIRTUAL_SPEED 40.0   //[SIMULATOR]初速度
#define SIMULATOR_VIRTUAL_YAW_RATE 0.5 //[SIMULATOR]１秒間の旋回数
#define SIMULATOR_W_NOISE 0.2       //[SIMULATOR]WHEELのノイズ
#define SIMULATOR_G_NOISE -0.5      //[SIMULATOR]GPSのノイズ
#define SIMULATOR_A_NOISE 1.0       //[SIMULATOR]ACCELのノイズ
#define SIMULATOR_Y_NOISE 0.02      //[SIMULATOR]ジャイロのノイズ

/**************************************************
 * 構造体：Sensor
 * センサーのデータ構造を一定にすることで処理を重複させないようにする
**************************************************/
typedef struct{
    double buffer[BUFFER_SIZE];
    int index;
    int count;
    int error_count;    //連続でエラーした回数をカウント
    /* --- 汎用化 --- */
    double current_val; //今読み込んだ生値
    double variance;    //固有の分散(W_VARなど)
    char name[10];      //デバッグ表示用("WHEEL"など)
} Sensor;

/******************************
 * 構造体：KalmanFilter2D
 * 2Dのカルマンフィルタで利用する構造体
******************************/
typedef struct{
    double speed;   //GPSなど加速度計を組み合わせて算出した速度
    double yaw;     //車両の向いている方角
    double x;       //座標(スタート地点からの現在地)
    double y;
    double var_x;   //推定誤差・分散(自身のなさ)
    double var_y;   
    double q_x;     //プロセスノイズ（どれくらい予測を疑うか）
    double q_y;     
}KalmanFilter2D;

/*--- プロトタイプ宣言 ---*/
void run_simulator(int step, double *v_speed, Sensor sns[]);//シミュレーターシーン管理用の関数
void get_fused_observation(Sensor sns[], double current_kf_speed, double *obs, double *var);
void process_sensor_fusion(double in1, double in2, double in3, double var1, double var2, Sensor *s1, Sensor *s2, Sensor *s3, double *obs, double *cur_var); //センサーステータス更新用関数
int check_sensor_status(Sensor *s,double raw_diff); //センサーの故障判断
int select_best_pair(double d_wg, double d_ga, double d_aw, int s_wg, int s_ga, int s_aw); //センサーの正しいらしいベアを判断
double calculate_kalman(double current_val, double *current_var, double obs, double obs_var, double q); //汎用カルマンフィルタ
double update_sensor(Sensor *s,double val);         //センサーのアップデート
double find_best_sensor(Sensor sensors[], int num_sensors, double predicted_speed);//救済関数
double simulate_sensor(double target_speed, double noise_level);//センサーの値をシミュレート生成する関数

/*--- 関数 ---*/
/************************************************************
 *シミュレーターのシーン管理用関数
 *引　数：STEP：何回目であるか
         *v_speed：シミュレーション用に決めている速度(ポインタ渡し)
         sns：センサーの値(シーンに応じた計算値を渡す)
 *返り値：void
 ************************************************************/
void run_simulator(int step, double *v_speed, Sensor sns[]){

    sns[0].current_val = simulate_sensor(*v_speed, SIMULATOR_W_NOISE);     //ノイズを加えたWHEELの入力値
    sns[1].current_val = simulate_sensor(*v_speed, SIMULATOR_G_NOISE);     //ノイズを加えたGPSの入力値
    sns[2].current_val = simulate_sensor(*v_speed, SIMULATOR_A_NOISE);     //ノイズを加えたACCELの入力値
    sns[3].current_val = simulate_sensor(SIMULATOR_VIRTUAL_YAW_RATE, SIMULATOR_Y_NOISE);   //ノイズを加えたGYROの入力値

    /* --- シーン①20回目からGPSが故障、36回目からWHEELも故障 START--- */
    //【シナリオ】20回目からGPSが故障して150km/hに固定される
    /*
    if(step >= 36){
        sns[3].current_val = 150.0;
        sns[2].current_val = 150.0;
        sns[1].current_val = 150.0;
        sns[0].current_val = 150.0;
    }
    if(step >= 20){
        sns[1].current_val = 150.0;
        sns[0].current_val = 150.0;
    }
    */
    /* --- シーン①20回目からGPSが故障、36回目からWHEELも故障 END--- */
}

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

    else{
        //全ての値のどれが正しいかが分からない場合
        printf("\n ->[EMERGENCY]:汎用関数で救済を試みます\n");
        *obs = find_best_sensor(sns, 3, current_kf_speed);

        //ACCELのズレがWHEELのズレよりも小さく、かつGPSのズレよりも小さく、更にそのズレが20km/h以内(加速度として20km/hはないとの判断)である場合にしている
        // ->よって、ACCELがこの中でマシな数値を持っていると判断できる
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

/****************************************
* 汎用カルマンフィルタ計算関数
* 引　数:double current_val:前回の計算結果
        double *current_var:前回の自身のなさ→計算後に更新するためポインタ渡し
        double obs:今回の実測値→フュージョンで作った速度
        double obs_var:センサーの自身のなさ
        double q:時間の経過による不安度合い
* 戻り値:updated_val 推定速度
****************************************/
double calculate_kalman(double current_val, double *current_var, double obs, double obs_var, double q){
    //step1 予測　前回の値を渡し、時間がたった分だけの不安度を追加
    double predict_val = current_val;
    double predict_var = (*current_var) + q; //時間が立つほど、予測は少しずつ不正確になる。(誤差を増やす→q)

    //step2 更新　predict_varとobs_varを比べてどっちがマシかな？
    //重み付けを予測とセンサーの間でおこなう
    double k_gain = predict_var / (predict_var + obs_var);   //カルマンゲインの計算(どのくらいセンサーの言うことを聞くのか)
    double updated_val = predict_val + k_gain * (obs - predict_val); //予測と実測のズレを足す(ゲインでどのくらい足すかを調整している)
    *current_var = (1.0 - k_gain) * predict_var; //情報を得たので誤差は小さくなる

    return updated_val;
}

/**************************************************
* センサー値のアップデート（移動平均用バッファ更新）
* 引　数: Sensor *s, double val
* 戻り値: 平均値
**************************************************/
double update_sensor(Sensor *s, double val){
    s->buffer[s->index] = val;
    s->index = (s->index + 1) % BUFFER_SIZE;
    if(s->count < BUFFER_SIZE) s->count++;

    double sum = 0;
    for (int i = 0; i < s->count; i++){
        sum += s->buffer[i];
    }
    return sum / s->count;
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

    Sensor sns[4] = {
        {{0}, 0 , 0, 0, 0.0, W_VAR, "WHEEL"},
        {{0}, 0 , 0, 0, 0.0, G_VAR, "GPS"},
        {{0}, 0 , 0, 0, 0.0, A_VAR, "ACCEL"},
        {{0}, 0 , 0, 0, 0.0, Y_VAR, "GYRO"}
    };

    KalmanFilter2D kf =   {40.0, 0.0, 0.0, 0.0, INITIAL_VAR, INITIAL_VAR, PROCESS_NOISE, 0.01}; //2Dカルマンフィルタの初期化
    double fusion_speed = 0.0;

    //ファイルを開く準備
    FILE *fp;
    fp = fopen("sensor_log.csv", "w");//書き込みモードで開く
    if(fp == NULL){
        printf("ファイルが開けませんでした\n");
        return 1;
    }
    //ファイルへのヘッダー書き込み
    fprintf(fp,"Step,Virtual_speed,Fusion_Speed,X,Y,Yaw,Error\n");

    printf("センサー入力を開始：\n");
    /* ＝＝＝ SIMULATOR　START ＝＝＝ */
    printf("＝＝＝ SIMULATOR START ＝＝＝\n");
    double virtual_speed = SIMULATOR_VIRTUAL_SPEED;         //シミュレーション時の初速度

    for(int i = 0; i < 100; i++){
        //入力(シミュレーター)
        run_simulator(i, &virtual_speed, sns);

        //センサー統合(多数決 & フュージョン)
        double observation = 0;         //統合された「今この瞬間の実測値」
        double sensor_var = 0;  //採用されたセンサーペアの「信頼性(分散)」
        
        //多数決とペアの選択
        get_fused_observation(sns, kf.speed, &observation, &sensor_var);

        kf.speed = calculate_kalman(kf.speed, &kf.var_x, observation, sensor_var, kf.q_x);//カルマンフィルタに掛ける
        fusion_speed = kf.speed;
        
        //以下もし角度を更新する場合
        //double kf.yaw = calculate_kalman(kf.yaw, &kf.var_y, gyro_obs, gyro_var, kf.q_y);//カルマンフィルタに掛ける
        double gyro_in = sns[3].current_val;

        //航法計算…ジャイロから得た角度を使って、二次元座標を算出
        kf.yaw += gyro_in * DT;
        double v_ms =  fusion_speed / 3.6;
        kf.x += v_ms * cos(kf.yaw) * DT;
        kf.y += v_ms * sin(kf.yaw) * DT;

        printf("[POS] X:%.2f, Y:%.2f, Yaw:%.2f[rad]\n", kf.x, kf.y, kf.yaw);

        //check_sensor_statusで問題ないことが判断できたため、アップデートしていく
        for(int s=0; s<4; s++){
            update_sensor(&sns[s], sns[s].current_val);
        }

        double error = fusion_speed - virtual_speed;
        double abs_error = (error < 0) ? -error : error;
        printf("[FUSION] 推定速度：%.2f(誤差：%.3f)[km/h]\n", fusion_speed, abs_error);

        //データの書き出し
        fprintf(fp, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.3f\n",i,virtual_speed, fusion_speed, kf.x, kf.y, kf.yaw, error);
    }
    //ファイルを閉じる
    fclose(fp);
    printf("\n[出力] 'sensor_log.csv' を保存しました。\n");
    return 0;
}
