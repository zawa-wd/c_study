#include <stdio.h>

int judge_vehicle(double s, double t){
    if(s > 100.0 || t > 100.0){
        return 1;
    }
    else{
        return 0;
    }

}

int main(){
    double speed = 105.5;
    double temp = 95.0;

    printf("車両診断中...\n");

    int result = judge_vehicle(speed, temp);

    if(result == 1){
    printf("WARNING\n");
    }
    else{
    printf("TRUE\n");
    }
    return 0;
}
