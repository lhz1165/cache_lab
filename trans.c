/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"
//cache 32个 每个32B

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
//0 1 2 3 4 5 6 7|8 9 10 11 12 13 14 15|16 17 18 19 20 21 22 23|24 25 26 27 28 29 30 31   4个cache块
//0 1 2 3 4 5 6 7|8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//...
//0 1 2 3 4 5 6 7|8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31   8行塞满cache

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
//
char transpose_submit_desc[] = "Transpose submission";

void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    for (int ii = 0; ii < N; ii += 8) {
        for (int jj = 0; jj < M; jj += 8) {
            //当A 加载cache小块0号-> B->0 4 8 12 16 20 24 28
            //由于A的0和B的0块 冲突了
            if (ii == jj) {
                for (int i = ii; i < ii + 8; ++i) {
                    if (i >= N) {
                        continue;
                    }
                    int a1, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0, a7 = 0, a8 = 0;
                    a1 = A[i][jj];
                    if (jj + 1 < M) {
                        a2 = A[i][jj + 1];
                    }
                    if (jj + 2 < M) {
                        a3 = A[i][jj + 2];
                    }
                    if (jj + 3 < M) {
                        a4 = A[i][jj + 3];
                    }
                    if (jj + 4 < M) {
                        a5 = A[i][jj + 4];
                    }
                    if (jj + 5 < M) {
                        a6 = A[i][jj + 5];
                    }
                    if (jj + 6 < M) {
                        a7 = A[i][jj + 6];
                    }
                    if (jj + 7 < M) {
                        a8 = A[i][jj + 7];
                    }

                    for (int j = jj; j < jj + 8; j += 8) {
                        B[jj][i] = a1;
                        if (jj + 1 < M) {
                            B[jj + 1][i] = a2;
                        }
                        if (jj + 2 < M) {
                            B[jj + 2][i] = a3;
                        }
                        if (jj + 3 < M) {
                            B[jj + 3][i] = a4;
                        }
                        if (jj + 4 < M) {
                            B[jj + 4][i] = a5;
                        }
                        if (jj + 5 < M) {
                            B[jj + 5][i] = a6;
                        }
                        if (jj + 6 < M) {
                            B[jj + 6][i] = a7;
                        }
                        if (jj + 7 < M) {
                            B[jj + 7][i] = a8;
                        }
                    }
                }
            } else {
                for (int i = ii; i < ii + 8; ++i) {
                    for (int j = jj; j < jj + 8; ++j) {
                        if (j >= M || i >= N) {
                            continue;
                        }
                        B[j][i] = A[i][j];
                    }
                }
            }
        }
    }


}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";

void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

