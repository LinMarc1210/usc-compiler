#include <stdio.h>

int main() {
    int i = 1;

    // Label to jump back to
    loop_start:

    if (i <= 5) {
        printf("Iteration %d\n", i);
        i++;
        goto loop_start;  // Jump back to the label
    }

    return 0;
}
