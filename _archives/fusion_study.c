#include <stdio.h>

//車両データの構造体
struct VehicleData {
    int id;
    double speed;
    char *source; //でーたの出所

};

//2つのでーたをとうごうするふゅーじょんかんすう
void fuse_velocity(struct VehicleData *ego, struct VehicleData *sensor){
    printf("---ふゅーじょんしょり (%s + %s)---\n",ego->source, sensor->source);

    //たんじゅんなへいきんによるふゅーじょん
    double fused_speed = (ego->speed + sensor -> speed )/2.0;

    //自車のそくどをこうしん
    ego->speed = fused_speed;
}

int main(){
    //自車ないのせんさーでーた
    struct VehicleData my_car = {1, 50.0, "Internal"};
    struct VehicleData obs_data = {1, 55.0, "External"};

    printf("とうごうまえ：そくど %.1f km/h\n", my_car.speed);

    //じぶんのでーたをがいぶでーたでほせい する
    fuse_velocity(&my_car, &obs_data);
    
    printf("とうごうご：さいしゅうそくど %.1f km/h\n", my_car.speed);

    return 0;

}

