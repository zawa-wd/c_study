#include <stdio.h>

#define BUFFER_SIZE 3
typedef struct{
    double buffer[BUFFER_SIZE];
    int index;
    int count;

} Sensor;

double update_sensor(Sensor *s,double val){
    s->buffer[s->index] = val;
    s->index = (s->index + 1) % BUFFER_SIZE;
    if(s->count < BUFFER_SIZE) s->count++;

    double sum = 0;
    for (int i = 0; i < s->count; i++){
        sum += s->buffer[i];

    }
    return sum / s->count;
}

int main(){
    Sensor wheel_speed = {{0}, 0, 0};
    Sensor gps_speed = {{0}, 0, 0};
    double w_in, g_in;

    printf("センサー入力を開始：\n");

    while(1){
        printf("> ");
        if(scanf("%lf %lf", &w_in,&g_in) != 2) break;

        double avg_w = update_sensor(&wheel_speed, w_in);
        double avg_g = update_sensor(&gps_speed, g_in);

        printf(" [Average] Wheel:%.2f, GPS: %.2f\n", avg_w, avg_g);

        double diff = avg_w - avg_g;

        if(diff < 0) diff = -diff;

        if(diff > 5.0){
            printf("[WARNING](%2.f)\n",diff);
        }

        else{
            printf("[FUSION]:統合速度＝%.2f km/h\n",(avg_w + avg_g)/2.0);
        }
    }
    return 0;
}