#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define RUN 32 // Минимальный размер сегмента для TimSort

typedef struct {
    int* array;
    int left;
    int right;
} ThreadData;

// Вспомогательная функция для сортировки вставками
void insertionSort(int arr[], int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        int temp = arr[i];
        int j = i - 1;
        while (j >= left && arr[j] > temp) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = temp;
    }
}

// Слияние двух отсортированных частей массива
void merge(int arr[], int left, int mid, int right) {
    int len1 = mid - left + 1, len2 = right - mid;
    int* leftArr = (int*)malloc(len1 * sizeof(int));
    int* rightArr = (int*)malloc(len2 * sizeof(int));

    for (int i = 0; i < len1; i++) leftArr[i] = arr[left + i];
    for (int i = 0; i < len2; i++) rightArr[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < len1 && j < len2) {
        if (leftArr[i] <= rightArr[j]) arr[k++] = leftArr[i++];
        else arr[k++] = rightArr[j++];
    }

    while (i < len1) arr[k++] = leftArr[i++];
    while (j < len2) arr[k++] = rightArr[j++];

    free(leftArr);
    free(rightArr);
}

// Основная реализация TimSort
void timSort(int arr[], int n) {
    for (int i = 0; i < n; i += RUN)
        insertionSort(arr, i, (i + RUN - 1 < n - 1) ? i + RUN - 1 : n - 1);

    for (int size = RUN; size < n; size = 2 * size) {
        for (int left = 0; left < n; left += 2 * size) {
            int mid = left + size - 1;
            int right = (left + 2 * size - 1 < n - 1) ? left + 2 * size - 1 : n - 1;

            if (mid < right)
                merge(arr, left, mid, right);
        }
    }
}

// Потоковая функция для сортировки части массива
void* sortThread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    timSort(data->array + data->left, data->right - data->left + 1);
    return NULL;
}

// Основная программа
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Введите: %s <number_of_threads> <number_of_array>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[2]);
    if (n <= 0){
        printf("Неправильно задано количество чисел в массиве. Число дожно быть больше 0.\n");
        return 2;
    }
    int maxThreads = atoi(argv[1]);
    if (maxThreads <= 0) {
        printf("Неправильно задано число потоков. Число должно быть больше 0.\n");
        return 3;
    }

    int* arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100;
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int chunkSize = (n + maxThreads - 1) / maxThreads;
    pthread_t threads[maxThreads];
    ThreadData threadData[maxThreads];

    for (int i = 0; i < maxThreads; i++) {
        int left = i * chunkSize;
        int right = (i + 1) * chunkSize - 1;
        if (right >= n) right = n - 1;

        threadData[i].array = arr;
        threadData[i].left = left;
        threadData[i].right = right;

        pthread_create(&threads[i], NULL, sortThread, &threadData[i]);
    }

    for (int i = 0; i < maxThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Объединение всех частей
    for (int size = chunkSize; size < n; size *= 2) {
        for (int left = 0; left < n; left += 2 * size) {
            int mid = left + size - 1;
            int right = (left + 2 * size - 1 < n - 1) ? left + 2 * size - 1 : n - 1;

            if (mid < right)
                merge(arr, left, mid, right);
        }
    }

    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Отсортированный массив: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\nВремя выполнения: %.6f секунд\n", elapsed_time);

    free(arr);
    return 0;
}
