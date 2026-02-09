#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

#include <stdio.h>
#include <math.h>

/* --- 定数設定 --- */
#define MAX_DIFF 10.0
#define ERROR_THRESHOLD 5
#define BUFFER_SIZE 3
#define DT 0.1
#define W_VAR 0.01
#define G_VAR 0.5
#define A_VAR 1.0
#define Y_VAR 1.0
#define INITIAL_VAR 1.0
#define PROCESS_NOISE 10.0
#define SIMULATOR_VIRTUAL_SPEED 40.0
#define SIMULATOR_VIRTUAL_YAW_RATE 0.5
#define SIMULATOR_W_NOISE 0.2
#define SIMULATOR_G_NOISE -0.5
#define SIMULATOR_A_NOISE 1.0
#define SIMULATOR_Y_NOISE 0.02
#define MAX_YAW_RATE 1.5
#define STOP_THRESHOLD 0.1

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
void run_simulator(int step, double *v_speed, Sensor sns[]);
void prosecess_sensor_fusion(double);
void get_fused_observation(Sensor sns[], double current_kf_speed, double *obs, double *var);    //多数決とペアの選択関数                               
void process_sensor_fusion(double in1, double in2, double in3, double var1,double var2, Sensor *s1, Sensor *s2, Sensor *s3, double *obs, double *cur_var); //センサーステータス更新用関数                                        
int check_sensor_status(Sensor *s,double raw_diff); //センサーの故障判断    
int select_best_pair(double d_wg, double d_ga, double d_aw, int s_wg, int s_ga, int s_aw); //センサーの正しいらしいベアを判断                          
double calculate_kalman(double current_val, double *current_var, double obs, double obs_var, double q); //汎用カルマンフィルタ                         
double update_sensor(Sensor *s,double val); //センサーのアップデート                                                                          
double find_best_sensor(Sensor sensors[], int num_sensors, double predicted_speed);//救済関数
double simulate_sensor(double target_speed, double noise_level);//センサーの値をシミュレート生成する関数
#endif
