#include "sensor_fusion.h"

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
    fp = fopen("./log/sensor_log.csv", "w");//書き込みモードで開く
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

        //旋回限界チェック
        if(fabs(gyro_in) > MAX_YAW_RATE){
            printf(" ->[GYRO WARNING]: 異常な旋回率(%.2f)を検知。無視します。\n", gyro_in);
            gyro_in = 0.0; // 以上は無視して「直進」とみなす
            sns[3].error_count++;
        }

        //静止時ドリフト対策
        if(fusion_speed < STOP_THRESHOLD){
            gyro_in = 0.0;
        }

        //故障判定
        if(sns[3].error_count >= ERROR_THRESHOLD){
            printf(" ->[GYRO FAILURE]: ジャイロが完全に故障しました。予測のみで航行します。\n");
            gyro_in = 0.0; //あるいは前回の値を少し維持するロジック
        }        

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
