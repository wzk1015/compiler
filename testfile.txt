int n, m;
int a[100];

void main() {
    int i, j;
    int op, l, r, delta;
    int sum;
    int prs = 1;
    int t;
    printf("18373806");
    scanf(n);
    scanf(m);
    for (i = n; i > 0; i = i - 1) {
        scanf(t);
        a[n - i + 1] = t;
    }
    for (i = 1; i <= m; i = i + 1) {
        scanf(op);
        scanf(l);
        scanf(r);
        switch (op) {
            case 1: {
                scanf(delta);
                for (j = r; j >= l; j = j - 1) {
                    a[j] = a[j] + delta;
                }
            }
            case 2: {
                scanf(delta);
                for (j = r; j >= l; j = j - 1) {
                    a[j] = a[j] - delta;
                }
            }
            case 3: {
                scanf(delta);
                for (j = r; j >= l; j = j - 1) {
                    a[j] = a[j] * delta;
                }
            }
            case 4: {
                sum = 0;
                for (j = r; j >= l; j = j - 1) {
                    sum = sum + a[j];
                }
                if (prs < 10) {
                    printf(sum);
                    prs = prs + 1;
                }
            }
            default: {
                if (prs < 10) {
                    printf("Undefined operation.");
                    prs = prs + 1;
                }
            }
        }
    }
}