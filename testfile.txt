
void a1d(int x) {
    int arr[5];
    arr[0] = 817;
    arr[1] = 1109;
    arr[2] = arr[0] + arr[1];
    arr[4] = 9193;
    arr[3] = 1284;
    printf("The magic happens at ", arr[x]);
}

void b1d() {
    const int xxx = 5;
    const char asdf = 'm';
    int i;
    char arr[5] = {'-', 'a', 'i', 'v', 'e'};
    i = +1 + asdf;
    arr[0] = 'n';
    i = -5 + xxx;
    while (i < xxx) {
        printf("Too young, too simple, sometimes ", arr[i]);
        i = i + 1;
    }
}

void a2d(int x, int y) {
    char arr2d[3][3] = {{'a', 'f', 'c'}, {'1', '2', 'r'}, {'+', '-', 'g'}};
    arr2d[1][1] = 'o';
    printf("Oh ", arr2d[x][y]);
}

void main() {
    int a, b;

    printf("18373444");
    scanf(a);
    a1d(a);
    a1d(a-2);
    b1d();
    scanf(b);
    a2d(a - 2, b - 1);
    a2d(1, 1);
}
