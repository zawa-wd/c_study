#include <stdio.h>

int main(){
    FILE *file;
    int id;
    int count = 0;
    int warning_count = 0;
    double speed;
    double total_speed =0;

    //読み込み
    file = fopen("fleet_log.txt", "r");

    if(file == NULL){
        printf("FILE OPEN 失敗\n");
        return 1;
    }
    
    printf("=== ログ読み込み開始 ===\n");

    while(fscanf(file, "ID:%d | STATUS:%*s | Speed:%lf",&id, &speed)!= EOF){
        printf("読み込み成功-> ID: %03d, 速度: %.1f\n", id, speed);

        char temp_buffer[100];
        fgets(temp_buffer, 100, file);
        
        if(speed > 0){
            total_speed += speed;
            count ++;
        }
        if(speed >= 100.0){
            warning_count++;
        }
    }
    fclose(file);
    printf("=== 読み込み完了 ===\n");
    printf("平均速度: %lf(km/h) \n",total_speed/count );
    printf("[WARNING]スピードオーバー:%d(台)\n",warning_count);
    return 0;
}
