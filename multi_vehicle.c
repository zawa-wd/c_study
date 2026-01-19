#include <stdio.h>

struct Vehicle {
    int id;
    double speed;
    double temp;

};

int main(){
    //3vehicles data
    struct Vehicle fleet[3] = {
        {101, 60.5,85.0},
        {102, 1200.0, 90.0},
        {103, 55.2, 115.0}
    };
    
    double speed_sum = 0.0;
    int valid_count = 0;

    printf("===Fleet Management System:START ===\n\n");

    for(int i=0;i<3;i++){
        printf("Checking Vehicle ID: %d...\n",fleet[i].id);
        
        //Guard Logic: Speed(0-180) AND Temp(max100)
        if (fleet[i].speed <= 180.0 && fleet[i].temp <= 100.0) {
            printf(" -> [Status: OK] Speed:%.1f, Temp:%.1f\n", fleet[i].speed, fleet[i].temp);
            speed_sum += fleet[i].speed;
            valid_count++;    
        }
        else{
            //error headling
            if(fleet[i].speed > 180.0) printf(" ->[WARNING] Speed Limit Exceeded!\n");
            if(fleet[i].temp > 100.0) printf(" ->[WARNING] Overheat Detected!\n");
            printf(" -> Skip this vehicle data.\n");
        }   
        printf("-------------------------------------\n");
    }
    
    if(valid_count > 0){
        printf("\n[Final Report]\n");
        printf("Vaild Vehicles: %d\n", valid_count);
        printf("Average Speed: %.2f km/h\n", speed_sum / valid_count);
    }
    else{
        printf("\n[Final Report] No vaild data found\n");
    }

    return 0;
}
