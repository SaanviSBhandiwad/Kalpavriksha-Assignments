//C program to calculate ticket price based on category
#include<stdio.h>
int main(){
    const float std_price = 150.0;
    const float pre_price = 250.0;
    const float rec_price = 400.0;
    char name[50];
    int ticket;
    int option;
    float price;
    float total;
    float discount;
    float bill;
    char type[20];
    scanf("%49s", name);
    scanf("%d", &ticket);
    scanf("%d", &option);
    if (ticket <= 0 || ticket > 20) {
        printf("Invalid ticket count\n");
        return 1;
    }
    switch (option) {
        case 1:
            price = std_price;
            strcpy(type, "Standard");
            break;
        case 2:
            price = pre_price;
            strcpy(type, "Premium");
            break;
        case 3:
            price = rec_price;
            strcpy(type, "Recliner");
            break;
        default:
            printf("Invalid option\n");
            return 1;
    }
    total = ticket * price;
    if (ticket >= 5) {
        discount = total * 0.10;
    } else {
        discount = 0.0;
    }
    bill = total - discount;
    printf("Customer: %s\n", name);
    printf("Ticket Category: %s\n", type);
    printf("Tickets: %d\n", ticket);
    printf("Subtotal: %.2f\n", total);
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", bill);

    return 0;

}
