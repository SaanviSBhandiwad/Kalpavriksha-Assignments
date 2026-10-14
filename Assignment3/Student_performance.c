#include <stdio.h>
#include <math.h>
int cnt = 0;
struct Pupil {
    int rid;
    char nm[50];
    int s1, s2, s3;
};
int addUp(int x, int y, int z) {//function to add marks
    return x + y + z;
}
float meanOf(int sum) {//function to calculate average
    return sum / 3.0;
}
char gradeOf(float avg) {//function to assign grade
    if (avg >= 85)
        return 'A';
    else if (avg >= 70)
        return 'B';
    else if (avg >= 50)
        return 'C';
    else if (avg >= 35)
        return 'D';
    else
        return 'F';
}
void showRolls(struct Pupil g[], int pos, int n) {//function to display roll numbers recursively
    if (pos == n)
        return;
    printf("%d ", g[pos].rid);
    showRolls(g, pos + 1, n);
}
int main() {
    int n, i, j;
    struct Pupil cls[100];
    scanf("%d", &n);
    for (i = 0; i < n; i++) {
        scanf("%d %s %d %d %d", &cls[i].rid, cls[i].nm, &cls[i].s1, &cls[i].s2, &cls[i].s3);
    }
    for (i = 0; i < n; i++) {
        int sum;      
        float avg;
        char gd;
        int sc = 0;
        sum = addUp(cls[i].s1, cls[i].s2, cls[i].s3);
        avg = meanOf(sum);
        gd = gradeOf(avg);
        cnt++;
        printf("Roll: %d\n", cls[i].rid);
        printf("Name: %s\n", cls[i].nm);
        printf("Total: %d\n", sum);
        printf("Average: %.2f\n", avg);
        printf("Grade: %c\n", gd);
        if (avg < 35)
            continue; // skip the stars for failed students
        switch (gd) {
            case 'A': sc = 5; break;
            case 'B': sc = 4; break;
            case 'C': sc = 3; break;
            case 'D': sc = 2; break;
        }
        printf("Performance: ");
        for (j = 0; j < sc; j++)
            printf("*");
        printf("\n");
    }
    printf("List of Roll Numbers (via recursion): ");
    showRolls(cls, 0, n);
    printf("\n");
    return 0;
}