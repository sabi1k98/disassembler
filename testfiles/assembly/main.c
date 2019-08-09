#include <stdio.h>

extern int npown(int base, int exp);

int main(void) {
    printf("5 to the power of 7 is %d\n", npown(5, 7));
    return 0;
}
