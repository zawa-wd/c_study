#include "sensor_fusion.h"

/******************************
*センサーの値をシミュレーション生成する関数
*引　数：target_speed:本来あるべき車速、
　　　　 noise_level:センサーのガタツキ具合(本来乱数を使うが今回は固定値)
*戻り値：
******************************/
static double simulate_sensor(double target_speed, double noise_level) {
    return target_speed + noise_level;
}

/************************************************************
 *シミュレーターのシーン管理用関数
 *引　数：STEP：何回目であるか
         *v_speed：シミュレーション用に決めている速度(ポインタ渡し)
         sns：センサーの値(シーンに応じた計算値を渡す)
 *返り値：void
 ************************************************************/
 void run_simulator(int step, double *v_speed, Sensor sns[]) {
    // 1. 基本となる正常データの生成
    sns[0].current_val = simulate_sensor(*v_speed, SIM_W_NOISE);
    sns[1].current_val = simulate_sensor(*v_speed, SIM_G_NOISE);
    sns[2].current_val = simulate_sensor(*v_speed, SIM_A_NOISE);
    sns[3].current_val = simulate_sensor(0.5, SIM_Y_NOISE); // 0.5rad/sで旋回中と仮定

    // 2. シナリオに基づく異常注入（インジェクション）
    #if ACTIVE_SCENARIO == SCENARIO_FAULT_DEMO
        if (step >= 25) {
            sns[1].current_val = 150.0; // GPSが25ステップ目から150km/hで固まる
        }
        if (step >= 50) {
            sns[3].current_val = 99.0;  // ジャイロが50ステップ目から暴走
        }
    #endif
}
