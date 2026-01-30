#include <stdio.h>

#define BUFFER_SIZE 3 //直近3回分を保持

int main(){
    double buffer[BUFFER_SIZE] = {0};
    int index = 0;
    int count = 0;

    double input;

    printf("センサー値を入力してください(-1で終了):\n");

    while(1){
        printf("> ");
        if(scanf("%lf", &input) != 1 || input == -1) break;

        buffer[index] = input;

        index = (index + 1)% BUFFER_SIZE;

        if (count < BUFFER_SIZE) count++;
        
        double sum = 0;
        for(int i = 0; i < count; i++){
            sum += buffer[i];
        }
        
        printf("[Buffer状況]: %.1f, %.1f, %.1f\n", buffer[0],buffer[1],buffer[2]);
        printf("[　移動平均　]:%.2f\n", sum / count);
    }
    return 0;
}
