#include <stdio.h>

struct Vehicle{
    int id;
    double speed;
    double temp;

};

int main(){
    struct Vehicle fleet[3] = {
        {101, 120.5, 105.0},
        {102, 80.0, 85.0},
        {103, 15.2, 70.0}
    };

    //
    FILE *file;
    file = fopen("report.txt", "w");

    if(file == NULL){
        printf("ERROR:ファイルが開けませんでした。\n");
        return 1;
    }

    //
    fprintf(file, "=== 車両診断レポート(ファイル出力版) ===\n\n");

    for (int i = 0; i < 3; i++){
        fprintf(file, "ID:%d | 速度:%.1f | 温度:%.1f\n",fleet[i].id, fleet[i].speed, fleet[i].temp);
    }

    fclose(file);

    printf("save report.txt\n");
    printf("Please command 'cat report.txt'\n");

    return 0;
}
