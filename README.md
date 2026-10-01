# Kalpavriksha - C Programming

This repo has my assignments for the Kalpavriksha program, where I am learning C with a focus on Data Structures and Operating Systems.

## Topics Covered

- main() function, compiling and running a program
- Variables, data types, constants
- printf() and scanf()
- Operators, if-else, switch-case
- Loops, break and continue
- Functions and recursion
- Structures and scope rules

## Assignments

Each assignment is on its own branch and in its own folder.

| Branch | Folder | Program |
|---|---|---|
| `assignment1` | `Assignment1/` | Calculator |
| `assignment2` | `Assignment2/` | CRUD on a file |

### Assignment 1: Calculator

A calculator that takes an expression like `3+4*2` and prints the answer. It handles `+`, `-`, `*`, `/`, ignores spaces and follows the order of operations. It uses a stack to evaluate the expression, and it shows an error for division by zero or an invalid expression.

### Assignment 2: CRUD on a file

A menu program to add, show, edit and remove users (ID, name, age). The users are stored in `users.txt` using a structure. For edit and remove I copy the records into a temp file and then replace the original file.

## How to run

```
git checkout assignment1
gcc Assignment1/Assignment1.c -o Assignment1
Assignment1
```

Do the same for `assignment2`, using `Assignment2`.