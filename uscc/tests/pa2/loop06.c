#include <stdio.h>

int main() {
    int i = 1;

    // Outer loop
    outer_loop:

    // Inner loop
    while (i <= 5) {
        printf("Outer Loop: Iteration %d\n", i);

        int j = 1;

        while (j <= 3) {
            printf("\tInner Loop: Iteration %d\n", j);

            // Complicated computation
            int result = i * j;
            printf("\tResult of computation: %d\n", result);

            if (result > 5) {
                printf("\tJumping back to Outer Loop\n");
                i++;
                goto outer_loop; // Jump to the label
            }

            j++;
        }

        i++;
    }

    return 0;
}

