void main() {
    char c;
    int a, b;
    printf("18231169");
    scanf(a);
    scanf(c);
    scanf(b);
    switch (c) {
        case '+': printf(a + b);
        case '-': printf(a - b);
        case '*': printf(a * b);
        case '/': printf(a / b);
        default: printf("error!");
    }
    switch (a) {
        case 1: printf(1);
        default: printf("a is not 1!");
    }
    for (a = 0; a < 6; a = a + 1) {
        scanf(b);
        printf(b);
    }
    printf("file3 over!");
}