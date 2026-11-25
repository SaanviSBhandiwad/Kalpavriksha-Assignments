#include<stdio.h>
#include<stdlib.h>
#include<string.h>
struct travel{
    char source[100];
    char destination[100];
};
int main(){
    struct travel *ptr;
    int n = 1;
    char city[100];
    char line[5000];
    printf("Enter the travels: ");
    fgets(line, sizeof(line), stdin);
    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] == ',')
            n++;
    }
    ptr = malloc(n * sizeof(struct travel));
    if(ptr == NULL){
        printf("Memory allocation error");
        return 0;
    }
    char *token = strtok(line, ",");
    for(int i = 0; i < n && token != NULL; i++){
        sscanf(token, " %99[^-]-> %99[^\n]", ptr[i].source, ptr[i].destination);
        token = strtok(NULL, ",");
    }
    int start = -1;
    for(int i = 0; i < n; i++){
        int found = 0;
        for(int j = 0; j < n; j++){
            if(strcmp(ptr[i].source, ptr[j].destination) == 0){
                found = 1;
                break;
            }
        }
        if(found == 0){
            strcpy(city, ptr[i].source);
            start = i;
            break;
        }
    }

    //next step is to print the path from the starting city which was found

    return 0;
}


