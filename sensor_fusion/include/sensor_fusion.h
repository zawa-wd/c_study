#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

#include <stdio.h>
#include <math.h>

/* --- 実験シナリオ定義 --- */
#define SCENARIO_NORMAL      0  // 正常航行
#define SCENARIO_FAULT_DEMO  1  // GPSとジャイロの故障デモ
#define ACTIVE_SCENARIO      SCENARIO_FAULT_DEMO // ★ここを切り替えて実験

/* --- システム定数 --- */
#define DT              0.1
#define TOTAL_STEPS     100
#define MAX_DIFF        10.0    // センサー間誤差しきい値
#define ERROR_THRESHOLD 5       // 故障確定までのエラー回数
#define STOP_THRESHOLD  0.1     // 停止判定しきい値
#define MAX_YAW_RATE    1.5     // ジャイロ限界値

/* --- シミュレータ用設定 --- */
#define SIM_START_SPEED  40.0
#define SIM_W_NOISE      0.2
#define SIM_G_NOISE     -0.5
#define SIM_A_NOISE      1.0
#define SIM_Y_NOISE      0.02

/* --- 構造体定義 --- */
typedef struct {
    double buffer[3];
    int index;
    int count;
    int error_count;
    double current_val;
    double variance;
    char name[10];
} Sensor;

typedef struct {
    double speed;
    double yaw;
    double x;
    double y;
    double var_x;
    double var_y;
    double q_x;
    double q_y;
} KalmanFilter2D;

/* --- 関数プロトタイプ宣言 --- */
// simulator.c
void run_simulator(int step, double *v_speed, Sensor sns[]);

// sensor_fusion.c
void get_fused_observation(Sensor sns[], double current_kf_speed, double *obs, double *var);
double calculate_kalman(double current_val, double *current_var, double obs, double obs_var, double q);
void update_sensor(Sensor *s, double val);

#endif
