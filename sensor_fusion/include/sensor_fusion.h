#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

#include <stdio.h>
#include <math.h>

/* --- 実験シナリオ定義 --- */
#define SCENARIO_NORMAL      0
#define SCENARIO_FAULT_DEMO  1
#define ACTIVE_SCENARIO      SCENARIO_FAULT_DEMO

/* --- システム基本定数 --- */
#define DT              0.1
#define TOTAL_STEPS     100
#define BUFFER_SIZE     3       // ★これがないと構造体定義でエラーになります
#define STOP_THRESHOLD  0.1

/* --- 判定しきい値 --- */
#define MAX_DIFF        10.0    // ★多数決ロジックで使用
#define ERROR_THRESHOLD 5       // ★故障判定で使用
#define MAX_YAW_RATE    1.5

/* --- センサー信頼度（分散値） --- */
#define W_VAR           0.01
#define G_VAR           0.5
#define A_VAR           1.0
#define Y_VAR           0.05

/* --- カルマンフィルタ設定 --- */
#define INITIAL_VAR     1.0
#define PROCESS_NOISE_V 10.0
#define PROCESS_NOISE_Y 0.01

/* --- シミュレータ用設定 --- */
#define SIM_START_SPEED  40.0
#define SIM_W_NOISE      0.2
#define SIM_G_NOISE     -0.5
#define SIM_A_NOISE      1.0
#define SIM_Y_NOISE      0.02

/* --- 構造体定義 --- */
typedef struct {
    double buffer[BUFFER_SIZE];
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
#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

#include <stdio.h>
#include <math.h>

/* --- 実験シナリオ定義 --- */
#define SCENARIO_NORMAL      0
#define SCENARIO_FAULT_DEMO  1
#define ACTIVE_SCENARIO      SCENARIO_FAULT_DEMO

/* --- システム基本定数 --- */
#define DT              0.1
#define TOTAL_STEPS     100
#define BUFFER_SIZE     3       // ★これがないと構造体定義でエラーになります
#define STOP_THRESHOLD  0.1

/* --- 判定しきい値 --- */
#define MAX_DIFF        10.0    // ★多数決ロジックで使用
#define ERROR_THRESHOLD 5       // ★故障判定で使用
#define MAX_YAW_RATE    1.5

/* --- センサー信頼度（分散値） --- */
#define W_VAR           0.01
#define G_VAR           0.5
#define A_VAR           1.0
#define Y_VAR           0.05

/* --- カルマンフィルタ設定 --- */
#define INITIAL_VAR     1.0
#define PROCESS_NOISE_V 10.0
#define PROCESS_NOISE_Y 0.01

/* --- シミュレータ用設定 --- */
#define SIM_START_SPEED  40.0
#define SIM_W_NOISE      0.2
#define SIM_G_NOISE     -0.5
#define SIM_A_NOISE      1.0
#define SIM_Y_NOISE      0.02

/* --- 構造体定義 --- */
typedef struct {
    double buffer[BUFFER_SIZE];
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
