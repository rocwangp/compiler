#include <stdio.h>
int main()
{
    int a_ = 1000; /* hello world */
    for(int i = 0; i < 5; ++i) {
        printf("hello world!");
    }
    if(a_ < 999) {
        int b = a_;
        printf("%d", b);
    }
    return 0;
}
