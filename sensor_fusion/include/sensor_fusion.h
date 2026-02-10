#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

#include <stdio.h>
#include <math.h>

/* --- 実験シナリオ定義 --- */
#define SCENARIO_NORMAL      0  //全てのセンサーが正常に動作するシナリオ
#define SCENARIO_FAULT_DEMO  1  //GPS固着やジャイロ異常を発生させるシナリオ
#define ACTIVE_SCENARIO      SCENARIO_FAULT_DEMO    //実行するシナリオ

/* --- システム基本定数 --- */
#define DT              0.1 //制御周期(10Hz = 0.1s)
#define TOTAL_STEPS     100 //シミュレーションの回数
#define BUFFER_SIZE     3   //移動平均用バッファサイズ(最新の3件の平均を取得)
#define STOP_THRESHOLD  0.1 //停止判定しきい値(これ以下は速度0とする)

/* --- 判定しきい値 --- */
#define MAX_DIFF        10.0 //センサー間の最大許容差分。これを超えると不一致とする
#define ERROR_THRESHOLD 5    //故障判定
#define MAX_YAW_RATE    1.5   //旋回率の限界値(rad/s) これを超えるとジャイロの異常と判断

/* --- センサー信頼度（分散値） --- */
#define W_VAR           0.01 //車輪速度センサー(高精度)
#define G_VAR           0.5  //GPS(ふらつきがあるが位置関係に強い)
#define A_VAR           1.0  //加速度(ノイズがのりやすい)
#define Y_VAR           0.05 //ジャイロ(高精度だが時間とともにズレが発生)

/* --- カルマンフィルタ設定 --- */
#define INITIAL_VAR     1.0  //推定誤差の初期値
#define PROCESS_NOISE_V 10.0 //速度変化の激しさ(加速・減速の予測しにくさ)
#define PROCESS_NOISE_Y 0.01 //方位変化の穏やかさ

/* --- シミュレータ用設定 --- */
#define SIM_START_SPEED  40.0 //シミュレーション開始時の時速(km/h)
#define SIM_W_NOISE      0.2  //車輪速に混ぜるランダムノイズの幅
#define SIM_G_NOISE     -0.5  //GPSにわざと乗せる一定のオフセット誤差
#define SIM_A_NOISE      1.0  //加速度計に乗せる大きめのスパイクノイズ
#define SIM_Y_NOISE      0.02 //ジャイロに乗せる微細なノイズ

/* --- 構造体定義 --- */
typedef struct {
    double buffer[BUFFER_SIZE]; //移動平均用のリングバッファ
    int index;                  //バッファの書き込み位置
    int count;                  //溜まったデータの数
    int error_count;            //異常検知された回数
    double current_val;         //移動平均後の現在値
    double variance;            //センサー固有の信頼度(分散)
    char name[10];              //センサー名
} Sensor;

typedef struct {
    double speed; //推定車速(km/h)
    double yaw;   //推定方位(rad)
    double x;     //推定座標(m)
    double y;     
    double var_x; //速度推定の不確かさ
    double var_y; //方位推定の不確かさ
    double q_x;   //速度のプロセスノイズ
    double q_y;   //方位のプロセスノイズ
} KalmanFilter2D;

/* --- 関数プロトタイプ宣言 --- */
// simulator.c
void run_simulator(int step, double *v_speed, Sensor sns[]);

// sensor_fusion.c
void get_fused_observation(Sensor sns[], double current_kf_speed, double *obs, double *var);
double calculate_kalman(double current_val, double *current_var, double obs, double obs_var, double q);
void update_sensor(Sensor *s, double val);

#endif
