#include <stdio.h>

int main() {                            // ❌ Syntax: missing closing parenthesis for function parameters
    int a, b c;                        // ❌ Syntax: missing comma between variables
    a = 10; 
    b = "20.5";                          // ❌ Semantic: assigning string to int
    c = a + b;                         // ❌ Semantic: result of incompatible addition (int + string)

    printf("Sum is: " %d, c)           // ❌ Syntax: missing comma between format string and variable, missing semicolon

    if (a > b)                          // ❌ Syntax: missing closing parenthesis for if condition
        printf("A is greater\n);      // ❌ Syntax: missing braces or parenthesis mismatch
    else
        printf("B is greater\n"        // ❌ Syntax: missing closing parenthesis in printf
}

