//Calculator Problem Statement 
#include <stdio.h>
#include <ctype.h>
#define MAX 1110

long long st[MAX];
int top = -1;

typedef struct {
    long long arr[MAX];
    int top;
} Stack;

void init(Stack *s) {
    s->top = -1;
}
int isEmpty(Stack *s) {
    return s->top == -1;
}
void push(Stack *s, long long x) {
    if (s->top < MAX - 1) {
        s->top++;
        s->arr[s->top] = x;
    }
}
long long pop(Stack *s) {
    if (isEmpty(s)) {
        return 0;
    }
    long long x = s->arr[s->top];
    s->top--;
    return x;
}
// Function to perform the calculation based on the operator and number
int calculate(Stack *s, char op, long long num) {
    if (op == '+') {
        push(s, num);
    } else if (op == '-') {
        push(s, -num);
    } else if (op == '*') {
        long long prev = pop(s);
        push(s, prev * num);
    } else if (op == '/') {
        if (num == 0) {
            return 0;
        }
        long long prev = pop(s);
        push(s, prev / num);
    }
    return 1;
}

int main() {
    char expr[MAX];
    Stack s;
    init(&s);
    if (fgets(expr, MAX, stdin) == NULL) {
        printf("Error: Invalid expression.\n");
        return 0;
    }
    long long num = 0;
    char op = '+';
    long long sign =  1;
    int hasSign = 0;
    int expectnum = 1; //To know if we are expecting a number next or not
    int prevdigit = 0;
    int divzero = 0; // to catch division by zero error
    for(int i = 0;expr[i]!= '\0';i++) {
        char ch = expr[i];
        if(isspace(ch)) {
            prevdigit = 0;
            continue;
        }
        if(isdigit(ch)){
            if(!expectnum && !prevdigit) {
                printf("Error: Invalid expression.\n");
                return 0;
            }

            num = num * 10 + (ch - '0');
            prevdigit = 1;
            expectnum = 0;
        }
        else if(ch == '+'|| ch == '-' || ch =='*' || ch == '/'){
            if(expectnum){
                if(ch == '-' && hasSign == 0){// Handle negative sign at the beginning or after an operator
                    sign = -1;
                    hasSign = 1;
                    prevdigit = 0;
                    continue;
                }
                else{
                    printf("Error: Invalid expression.\n");
                    return 0;
                }
            }
            if(!calculate(&s, op, sign * num)) {
                divzero = 1;
            }
            op = ch;
            num =  0;
            sign = 1;
            hasSign = 0;
            expectnum = 1; 
            prevdigit = 0;

        }
        else{
            printf("Error: Invalid expression.\n");
            return 0;
        }
        
    }
    if(expectnum){
        printf("Error: Invalid expression.\n");
        return 0;
    }
    if(!calculate(&s, op, sign * num)) { 
        divzero = 1;
    }
    if(divzero) {
        printf("Error: Division by zero.\n");
        return 0;
    }
    long long result = 0;
    while(!isEmpty(&s)) {// Sum up all the values in the stack
        result += pop(&s);
    }
    printf("%lld\n", result);
    return 0;
   
}





