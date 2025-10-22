#include <stdio.h>
#include <stdlib.h>

void displayMatrix(int size, int (*randomMatrix)[size])
{
    for(int i = 0; i<size; i++)
    {
        for(int j =0; j<size; j++)
        {
            printf("%4d", *(*(randomMatrix+i)+j));
        }
        printf("\n");
    }
}

void createMatrix(int size, int (*randomMatrix)[size])
{
    printf("\nOriginal Randomly Generated Matrix:\n");
    for(int i = 0; i<size; i++)
    {
        for(int j =0; j<size; j++)
        {
            *(*(randomMatrix+i)+j) = rand() % 256;
        }
    }
    displayMatrix(size, randomMatrix);
}

void swap(int *rowElement, int *columnElement)
{
    int temp = *rowElement;
    *rowElement = *columnElement;
    *columnElement = temp;
}

void reverse(int size, int (*randomMatrix)[size])
{
    for (int i = 0; i < size; i++)
    {
        int *start = *(randomMatrix + i);
        int *end = *(randomMatrix + i) + (size - 1);
        while (start < end)
        {
            swap(start, end);
            start++;
            end--;
        }
    }
}

void rotateMatrix(int size, int (*randomMatrix)[size])
{
    for (int i = 0; i < size; i++)
    {
        for (int j = i + 1; j < size; j++)
        {
            swap((*(randomMatrix + i) + j), (*(randomMatrix + j) + i));
        }
    }
    reverse(size, randomMatrix);

    printf("\nMatrix after 90 degree Clockwise Rotation:\n");
    displayMatrix(size, randomMatrix);
}

int calculateAverage(int size, int (*randomMatrix)[size], int i, int j)
{
    int sum = 0, count = 0;
    for (int row = i - 1; row <= i + 1; row++)
    {
        for (int column = j - 1; column <= j + 1; column++)
        {
            if (row >= 0 && row < size && column >= 0 && column < size)
            {
                sum += *(*(randomMatrix + row) + column);
                count++;
            }
        }
    }
    return sum / count;
}

void processRow(int size, int (*randomMatrix)[size], int i, int *currRow)
{
    for (int j = 0; j < size; j++)
    {
        currRow[j] = calculateAverage(size, randomMatrix, i, j);
    }
}

void filter(int size, int (*randomMatrix)[size])
{
    int prevRow[size];
    int currRow[size];

    for (int i = 0; i < size; i++)
    {
        processRow(size, randomMatrix, i, currRow);

        if (i > 0)
        {
            for (int j = 0; j < size; j++)
            {
                *(*(randomMatrix + i - 1) + j) = prevRow[j];
            }
        }

        for (int j = 0; j < size; j++)
        {
            prevRow[j] = currRow[j];
        }
    }

    for (int j = 0; j < size; j++)
    {
        *(*(randomMatrix + size - 1) + j) = prevRow[j];
    }

    printf("\nMatrix after Applying %d x %d Smoothing Filter:\n", size, size);
    displayMatrix(size, randomMatrix);
}

int main()
{
    int size;
    char choice;

    do
    {
        printf("\nEnter matrix size (2-10): ");
        int result = scanf("%d", &size);

        if (result != 1)
        {
            printf("\nInvalid input! Please enter an integer.\n");
            while (getchar() != '\n');
            continue;
        }

        if(size>10 || size <2)
        {
            printf("\nInvalid size! It should be in range (2-10)");
            continue;
        }

        int randomMatrix[size][size];

        createMatrix(size, randomMatrix);
        rotateMatrix(size, randomMatrix);
        filter(size, randomMatrix);

        printf("\nDo you want to continue? (y/n): ");
        scanf(" %c", &choice);
    } while(choice!='n');

    return 0;
}
