#include "sensor_fusion.h"

int main() {
    /* --- 1. センサー構造体の初期化 --- */
    // ヘッダーで定義した分散値(W_VARなど)を使用して初期設定を行います
    Sensor sns[4] = {
        {{0}, 0, 0, 0, 0.0, W_VAR, "WHEEL"},
        {{0}, 0, 0, 0, 0.0, G_VAR, "GPS"},
        {{0}, 0, 0, 0, 0.0, A_VAR, "ACCEL"},
        {{0}, 0, 0, 0, 0.0, Y_VAR, "GYRO"}
    };

    /* --- 2. カルマンフィルタの初期化 --- */
    // 初期速度やプロセスノイズを設定します
    KalmanFilter2D kf = {
        SIM_START_SPEED,   // speed: 初期速度
        0.0,               // yaw: 方位
        0.0,               // x: 座標
        0.0,               // y: 座標
        INITIAL_VAR,       // var_x: 推定誤差(分散)
        INITIAL_VAR,       // var_y: 推定誤差(分散)
        PROCESS_NOISE_V,   // q_x: 速度のプロセスノイズ
        PROCESS_NOISE_Y    // q_y: 方位のプロセスノイズ
    };

    double virtual_speed = SIM_START_SPEED; // シミュレーション上の真の速度
    double fusion_speed = 0.0;

    /* --- 3. ログファイル（CSV）の準備 --- */
    FILE *fp = fopen("logs/sensor_log.csv", "w");
    if (fp == NULL) {
        printf("エラー：ログファイルを開けませんでした。'logs'ディレクトリが存在するか確認してください。\n");
        return 1;
    }
    // ヘッダー行の書き込み
    fprintf(fp, "Step,Virtual_speed,Fusion_Speed,X,Y,Yaw,Error\n");

    printf("=== 自動航行シミュレーション開始 (シナリオ: %d) ===\n", ACTIVE_SCENARIO);

    /* --- 4. メインループ --- */
    for (int i = 0; i < TOTAL_STEPS; i++) {
        // [シミュレータ層] 仮想センサーデータの生成
        run_simulator(i, &virtual_speed, sns);

        // [ロジック層] センサーフュージョン（多数決とペア選択）
        double observation = 0;
        double sensor_var = 0;
        get_fused_observation(sns, kf.speed, &observation, &sensor_var);

        // [ロジック層] カルマンフィルタによる速度推定の更新
        kf.speed = calculate_kalman(kf.speed, &kf.var_x, observation, sensor_var, kf.q_x);
        fusion_speed = kf.speed;

        // [ロジック層] ジャイロデータの処理と異常検知
        double gyro_in = sns[3].current_val;
        if (fabs(gyro_in) > MAX_YAW_RATE) {
            printf(" ->[GYRO WARNING]: 異常な旋回率(%.2f)を検知。無視します。\n", gyro_in);
            gyro_in = 0.0; // 異常値は「直進」として扱うフェイルセーフ
            sns[3].error_count++;
        }

        // 停止時のドリフト防止
        if (fusion_speed < STOP_THRESHOLD) {
            gyro_in = 0.0;
        }

        /* --- 5. 航法計算（座標更新） --- */
        kf.yaw += gyro_in * DT;
        double v_ms = fusion_speed / 3.6; // km/h -> m/s 変換
        kf.x += v_ms * cos(kf.yaw) * DT;
        kf.y += v_ms * sin(kf.yaw) * DT;

        // センサーの移動平均バッファを更新
        for (int s = 0; s < 4; s++) {
            update_sensor(&sns[s], sns[s].current_val);
        }

        /* --- 6. 画面表示とログ保存 --- */
        double error = fusion_speed - virtual_speed;
        printf("[%02d] POS(%.1f, %.1f) YAW:%.2f SPEED:%.2f (ERR:%.3f)\n", 
               i, kf.x, kf.y, kf.yaw, fusion_speed, error);

        fprintf(fp, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.3f\n", 
                i, virtual_speed, fusion_speed, kf.x, kf.y, kf.yaw, error);
    }

    fclose(fp);
    printf("=== シミュレーション終了：'logs/sensor_log.csv' に保存しました ===\n");

    return 0;
}
