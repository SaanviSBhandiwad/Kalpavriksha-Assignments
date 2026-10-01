//CRUD operations on a file
#include <stdio.h>
#include <string.h>
#define MAX 100

struct User{
    int id;
    char name[MAX];
    int age;
};

void  createUser();
void  readUser();
void  updateUser();
void  deleteUser();
int idExists(int key);

int main(){
    int op;
    do{
        printf("1. Create User\n");
        printf("2. Read User\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &op);
        switch(op){
            case 1:
                createUser();
                break;
            case 2:
                readUser();
                break;
            case 3:
                updateUser();
                break;
            case 4:
                deleteUser();
                break;
            case 5:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid option. Please try again.\n");
        }
    }while(op != 5);
    return 0;


}

// Function to check if a user ID already exists in the file
int idExists(int key){
    FILE *f;
    struct User u;
    f = fopen("users.txt","r");
    if(f == NULL){
        return 0;
    }
    while (fscanf(f, "%d|%99[^|]|%d\n", &u.id, u.name, &u.age) == 3)
    {
        if (u.id == key)
        {
            fclose(f);
            return 1;
        }
    }
 
    fclose(f);
    return 0;

}
// Function to create a new user and save it to the file
void createUser(){
    FILE *f;
    struct User u;
    f=fopen("users.txt","a");
    if(f == NULL){
        printf("Error opening file.\n");
        return;
    }
    printf("Enter user ID: ");
    scanf("%d", &u.id);
    if(idExists(u.id)){
        printf("User ID already exists. Please try again.\n");
        fclose(f);
        return;
    }
    printf("Enter user name: ");
    scanf(" %99[^\n]", u.name);
    printf("Enter user age: ");
    scanf("%d", &u.age);
    fprintf(f, "%d|%s|%d\n", u.id, u.name
, u.age);
    fclose(f);
    printf("User created successfully.\n");
}
// Function to read and display all users from the file
void readUser(){
    FILE *f;
    struct User u;
    int count = 0;
    f=fopen("users.txt","r");
    if(f == NULL){
        printf("Error opening file.\n");
        return;
    }
    printf("ID\tName\tAge\n");
    while (fscanf(f, "%d|%99[^|]|%d\n", &u.id, u.name, &u.age) == 3)
    {
        printf("%d\t%s\t%d\n", u.id, u.name, u.age);
        count++;
    }
    if (count == 0)
    {
        printf("No users found.\n");    
    }
    fclose(f);
}
// Function to update an existing user's information in the file
void updateUser(){
    FILE *f, *t;
    struct User u;
    int key, found = 0;
    printf("Enter user ID to update: ");
    scanf("%d", &key);
    f=fopen("users.txt","r");
    t=fopen("temp.txt","w");
    if(f == NULL || t == NULL){
        printf("Error opening file.\n");
        return;
    }
    while (fscanf(f, "%d|%99[^|]|%d\n", &u.id, u.name, &u.age) == 3)
    {
        if (u.id == key)
        {
            found = 1;
            printf("Enter new user name: ");
            scanf(" %99[^\n]", u.name);
            printf("Enter new user age: ");
            scanf("%d", &u.age);
        }
        fprintf(t, "%d|%s|%d\n", u.id, u.name, u.age);
    }
    fclose(f);
    fclose(t);
    if (found== 0){
        printf("User ID not found.\n");
        remove("temp.txt");
    }
    else{
        remove("users.txt");
        rename("temp.txt", "users.txt");
        printf("User updated successfully.\n");
    }
}
// Function to delete a user from the file
void deleteUser(){
    FILE *f, *t;
    struct User u;
    int key, found = 0;
    printf("Enter user ID to delete: ");
    scanf("%d", &key);
    f=fopen("users.txt","r");
    t=fopen("temp.txt","w");
    if(f == NULL || t == NULL){
        printf("Error opening file.\n");
        return;
    }
    while (fscanf(f, "%d|%99[^|]|%d\n", &u.id, u.name, &u.age) == 3)
    {
        if (u.id == key)
        {
            found = 1;
            continue; // Skip writing this user to the temp file
        }
        fprintf(t, "%d|%s|%d\n", u.id, u.name, u.age);
    }
    fclose(f);
    fclose(t);
    if (found== 0){
        printf("User ID not found.\n");
        remove("temp.txt");
    }
    else{
        remove("users.txt");
        rename("temp.txt", "users.txt");
        printf("User deleted successfully.\n");
    }
}   



