#include<stdio.h>
#include<stdlib.h>
#include<time.h>

void generate(int **matrix, int n) //function to generate random values in the matrix
{
    for(int i=0; i < n;i++)
    {
        for(int j=0; j < n;j++)
        {
            *(*(matrix+i)+j) = rand() % 256;
        }
    }
}

void display(int **matrix, int n) //function to display the matrix
{
    for(int i=0; i < n;i++)
    {
        for(int j=0; j < n;j++)
        {
            printf("%d ", *(*(matrix+i)+j));
        }
        printf("\n");
    }
}

void swap(int *a, int *b) //function to swap
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void rotate(int **matrix, int n) //function to rotate the matrix by 90 degrees clockwise
{
    // Transpose the matrix
    for(int i=0; i < n;i++){
        for(int j = i+1; j <n; j++){
            swap(*(matrix+i)+j, *(matrix+j)+i);
        }
    }

    // Reverse each row
    for(int i=0; i < n;i++)
    {
        int *start = *(matrix+i);
        int *end = *(matrix+i) + (n - 1);
        while(start < end)
        {
            swap(start, end);
            start++;
            end--;
        }
    }
}

void smoothing(int **matrix, int n) //function to smooth the matrix
{
    int *previous = (int *)malloc(n * sizeof(int));
    int *current = (int *)malloc(n * sizeof(int));
    for(int i = 0; i < n;i++)
    {
        for(int j = 0; j < n; j++)
        {
            *(current + j) = *(*(matrix + i) + j);
        }
        for(int j = 0; j < n; j++)
        {
            int sum = 0;
            int count = 0;
            for(int k=  -1; k <=1; k++)
            {
                int current_index = j + k;
                if(current_index <0 || current_index >= n)
                {
                    continue;
                }
                if(i > 0)
                {
                    sum += *(previous + current_index);
                    count++;
                }
                sum += *(current + current_index);
                count++;
                if(i < n - 1)
                {
                    sum += *(*(matrix + i + 1) + current_index);
                    count++;
                }
            }
            *(*(matrix + i) + j) = sum / count;
        }
        for(int j = 0; j < n; j++)
            {
                *(previous + j) = *(current + j);
            }
    }
    free(previous);
    free(current);
}

int main()
{
    int n;
    printf("Enter the size of the matrix (2-10)\n");
    scanf("%d", &n);
    if(n < 2 || n > 10)
    {
        printf("Invalid size. Please enter a number between 2 and 10.\n");
        return 1;
    }
    srand((unsigned)(time(NULL)));
    int **matrix = (int **)malloc(n * sizeof(int *));
    for(int i=0; i < n; i++)
    {
        *(matrix + i) = (int *)malloc(n * sizeof(int));
    }

    generate(matrix, n);
    printf("Original Matrix:\n");
    display(matrix, n);
    printf("\n");

    rotate(matrix, n);
    printf("Matrix after 90-degree clockwise rotation:\n");
    display(matrix, n);
    printf("\n");

    smoothing(matrix, n);
    printf("Matrix after smoothing:\n");
    display(matrix, n);
    printf("\n");

    for(int i=0; i < n; i++)
    {
        free(*(matrix + i));
    }
    free(matrix);

}