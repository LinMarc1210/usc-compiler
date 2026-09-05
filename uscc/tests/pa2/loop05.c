#include <stdio.h>

int main() {
    int i, j, k;

    // Outer loop
    for (i = 1; i <= 5; i++) {
        printf("Outer Loop: Iteration %d\n", i);

        // Inner loop 1
        for (j = 1; j <= 3; j++) {
            printf("\tInner Loop 1: Iteration %d\n", j);

            // Inner loop 2
            for (k = 1; k <= 4; k++) {
                printf("\t\tInner Loop 2: Iteration %d\n", k);

                // Complicated computation
                int result = i * j + k;
                printf("\t\tResult of computation: %d\n", result);

                // Goto statement to jump back to Inner Loop 2
                if (result > 5) {
                    printf("\t\tJumping back to Inner Loop 2\n");
                    goto inner_loop_2;
                }
            }

            // Label for Inner Loop 2
            inner_loop_2:

            // Additional code after Inner Loop 2
            printf("\tAfter Inner Loop 2\n");
        }

        printf("\n");
    }

    return 0;
}

