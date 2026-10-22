#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NAME_SIZE 100

//Structure declaration
typedef struct Employee Employee;
struct Employee 
{
    int id;
    char name[NAME_SIZE];
    char designation[NAME_SIZE];
    Employee *manager;
    Employee **reports;
    int reportCount;
    int reportCapacity;
};
Employee **employees = NULL;
int employeeCount = 0;
int employeeCapacity = 0;

// Function declarations
Employee *findEmployee(int id);
int  ensureEmployeeCapacity();
int  ensureReportCapacity(Employee *employee, int needed);
int  addReport(Employee *manager, Employee *report);
void removeReport(Employee *manager, Employee *report);
void addEmployee();
void removeEmployee();
void displayEmployeeTree(Employee *employee, int level);
void displayEmployeeHierarchy();
void displayCompleteHierarchy();
void freeAllMemory();

int main() 
{
    int choice;

    do 
    {
        printf("\n===== Employee Hierarchy Management =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
            case 1:
                addEmployee();
                break;
            case 2:
                removeEmployee();
                break;
            case 3:
                displayEmployeeHierarchy();
                break;
            case 4:
                displayCompleteHierarchy();
                break;
            case 5:
                freeAllMemory();
                printf("Exiting program.\n");
                break;

            default:
                printf("Invalid choice. Please select 1 to 5.\n");
        }

    } while (choice != 5);

    return 0;
}

// Find an employee using their ID
Employee *findEmployee(int id) 
{
    int i;
    for (i = 0; i < employeeCount; i++) 
    {
        if ((*employees[i]).id == id) 
        {
            return employees[i];
        }
    }
    return NULL;
}

// Increase capacity of the employee array
int ensureEmployeeCapacity()
{
    Employee **temp;
    int newCapacity;
    if (employeeCount < employeeCapacity) 
    {
        return 1;
    }
    newCapacity = (employeeCapacity == 0) ? 4 : employeeCapacity * 2;
    temp = realloc(employees, newCapacity * sizeof(Employee *));
    if (temp == NULL) 
    {
        printf("Memory allocation failed.\n");
        return 0;
    }
    employees = temp;
    employeeCapacity = newCapacity;

    return 1;
}

// Ensure enough space for direct reports
int ensureReportCapacity(Employee *employee, int needed) 
{
    Employee **temp;
    int newCapacity;
    if ((*employee).reportCapacity >= needed) 
    {
        return 1;
    }

    newCapacity = ((*employee).reportCapacity == 0) ? 2 : (*employee).reportCapacity;
    while (newCapacity < needed) 
    {
        newCapacity *= 2;
    }
    temp = realloc((*employee).reports, newCapacity * sizeof(Employee *));
    if (temp == NULL) 
    {
        printf("Memory allocation failed.\n");
        return 0;
    }
    
    (*employee).reports = temp;
    (*employee).reportCapacity = newCapacity;

    return 1;
}

// Add a direct report to a manager
int addReport(Employee *manager, Employee *report) 
{
    if (manager == NULL || report == NULL)
    {
        return 0;
    }
    if (!ensureReportCapacity(manager, (*manager).reportCount + 1)) 
    {
        return 0;
    }
    
    (*manager).reports[(*manager).reportCount] = report;
    (*manager).reportCount++;
    (*report).manager = manager;

    return 1;
}

// Remove a direct report from a manager's list
void removeReport(Employee *manager, Employee *report) 
{
    int i, j;
    if (manager == NULL || report == NULL) 
    {
        return;
    }

    for (i = 0; i < (*manager).reportCount; i++)
    {
        if ((*manager).reports[i] == report) 
        {
            for (j = i; j < (*manager).reportCount - 1; j++) 
            {
                (*manager).reports[j] = (*manager).reports[j + 1];
            }
            (*manager).reportCount--;
            return;
        }
    }
}

// Add a new employee
void addEmployee() 
{
    Employee *employee;
    Employee *manager = NULL;
    int id;
    int managerId;
    if (!ensureEmployeeCapacity()) 
    {
        return;
    }

    employee = malloc(sizeof(Employee));

    if (employee == NULL) 
    {
        printf("Memory allocation failed.\n");
        return;
    }
    (*employee).manager = NULL;
    (*employee).reports = NULL;
    (*employee).reportCount = 0;
    (*employee).reportCapacity = 0;
    
    printf("Enter employee ID: ");
    scanf("%d", &id);

    if (findEmployee(id) != NULL) 
    {
        printf("Employee ID already exists.\n");
        free(employee);
        return;
    }

    (*employee).id = id;

    printf("Enter employee name: ");
    scanf(" %99[^\n]", (*employee).name);
    
    printf("Enter designation: ");
    scanf(" %99[^\n]", (*employee).designation);

    printf("Enter manager ID (0 only for CEO): ");
    scanf("%d", &managerId);

    if (employeeCount == 0) 
    {
        // The first employee must be the CEO
        if (managerId != 0) 
        {
            printf("The first employee must be the CEO.\n");
            free(employee);
            return;
        }
    } 
    else 
        {
        // Every non-CEO employee must have a manager
        if (managerId == 0) 
        {
            printf("Every non-CEO employee must have a manager.\n");
            free(employee);
            return;
        }
        manager = findEmployee(managerId);
        if (manager == NULL)
        {
            printf("Manager not found.\n");
            free(employee);
            return;
        }
        if (!ensureReportCapacity(manager, (*manager).reportCount + 1)) 
        {
            free(employee);
            return;
        }
    }
    employees[employeeCount] = employee;
    employeeCount++;
    if (manager != NULL) 
    {
        if (!addReport(manager, employee)) 
        {
            employeeCount--;
            free(employee);
            return;
        }
    }
    printf("Employee added successfully.\n");
}

// Remove an employee and transfer their direct reports
void removeEmployee() 
{
    int id;
    int i, j;
    int bestCount = -1;
    Employee *employee;
    Employee *manager;
    Employee *target = NULL;

    printf("Enter employee ID to remove: ");
    scanf("%d", &id);

    employee = findEmployee(id);
    if (employee == NULL) 
    {
        printf("Employee not found.\n");
        return;
    }
    manager = (*employee).manager;

    // Do not remove the CEO while other employees exist
    if (manager == NULL && employeeCount > 1) 
    {
        printf("Cannot remove the CEO while other employees exist.\n");
        return;
    }

    // Transfer direct reports if the employee has any
    if ((*employee).reportCount > 0) 
    {
    // Find the sibling with the fewest direct reports
        for (i = 0; i < (*manager).reportCount; i++) 
        {
            Employee *sibling = (*manager).reports[i];
            if (sibling != employee) 
            {
                if (bestCount == -1 || (*sibling).reportCount < bestCount) 
                {
                    bestCount = (*sibling).reportCount;
                    target = sibling;
                }
            }
        }

        // If no sibling exists, transfer to the superior
        if (target == NULL) 
        {
            target = manager;
        }

        // Allocate space before transferring any reports
        if (!ensureReportCapacity(target,(*target).reportCount + (*employee).reportCount))
        {
            return;
        }

        printf("Transferring reports to %s (ID: %d).\n", (*target).name, (*target).id);
        while ((*employee).reportCount > 0) 
        {
            Employee *report =(*employee).reports[(*employee).reportCount - 1];
            (*employee).reportCount--;
            (*target).reports[(*target).reportCount] = report;
            (*target).reportCount++;
            (*report).manager = target;
        }
    }

    // Detach employee from their manager
    if (manager != NULL) 
    {
        removeReport(manager, employee);
    }

    // Remove employee from the main employee array
    for (i = 0; i < employeeCount; i++) 
    {
        if (employees[i] == employee) 
        {
            for (j = i; j < employeeCount - 1; j++) 
            {
                employees[j] = employees[j + 1];
            }
            employeeCount--;
            break;
        }
    }
    free((*employee).reports);
    free(employee);
    printf("Employee removed successfully.\n");
}

// Display one employee and all their descendants
void displayEmployeeTree(Employee *employee, int level) 
{
    int i;
    for (i = 0; i < level; i++) 
    {
        printf("    ");
    }

    printf("|-- ID: %d, Name: %s, Designation: %s\n",
           (*employee).id,
           (*employee).name,
           (*employee).designation);

    for (i = 0; i < (*employee).reportCount; i++) 
    {
        displayEmployeeTree((*employee).reports[i], level + 1);
    }
}

// Display hierarchy under a particular employee
void displayEmployeeHierarchy() 
{
    int id;
    Employee *employee;

    printf("Enter employee ID to display: ");
    scanf("%d", &id);

    employee = findEmployee(id);

    if (employee == NULL) 
    {
        printf("Employee not found.\n");
        return;
    }
    printf("\nHierarchy under %s (ID: %d):\n", (*employee).name, (*employee).id);
    displayEmployeeTree(employee, 0);
}

// Display the complete company hierarchy
void displayCompleteHierarchy() 
{
    int i;
    int foundRoot = 0;
    if (employeeCount == 0) 
    {
        printf("No employees to display.\n");
        return;
    }

    printf("\n===== Complete Employee Hierarchy =====\n");

    for (i = 0; i < employeeCount; i++) 
    {
        if ((*employees[i]).manager == NULL) 
        {
            foundRoot = 1;
            displayEmployeeTree(employees[i], 0);
        }
    }

    if (!foundRoot) 
    {
        printf("No CEO found.\n");
    }
}

// Free allocated memory before exiting
void freeAllMemory() 
{
    int i;
    for (i = 0; i < employeeCount; i++) 
    {
        free((*employees[i]).reports);
        free(employees[i]);
    }
    free(employees);
    employees = NULL;
    employeeCount = 0;
    employeeCapacity = 0;
}