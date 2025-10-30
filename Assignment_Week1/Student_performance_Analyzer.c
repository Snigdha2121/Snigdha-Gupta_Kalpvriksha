#include<stdio.h>

typedef struct
{
    int RollNumber;
    char Name[100];
    float Marks[3];
} StudentInformation;

void inputData(StudentInformation *student, int numberOfStudents)
{
    for(int i = 0; i<numberOfStudents; i++)
    {
        printf("\nEnter the roll number of the Student %d: ", i+1);
        scanf("%d", &student[i].RollNumber);
        int duplicate = 0;
        for (int j = 0; j < i; j++) 
        {
            if (student[i].RollNumber == student[j].RollNumber) 
            {
                printf("\nRoll number cannot repeat. Please re-enter.\n");
                duplicate = 1;
                break;
            }
        }
        if(duplicate) 
        {
            i--;
            continue;
        }
        printf("\nEnter the Name of the Student %d: ", i+1);
        getchar();
        fgets(student[i].Name, sizeof(student[i].Name), stdin);

        for(int j = 0; j<3; j++)
        {
            printf("Enter the marks %d: ", j+1);
            scanf("%f", &student[i].Marks[j]);
            if(student[i].Marks[j] < 0 || student[i].Marks[j] > 100)
            {
                printf("\nInvalid Marks. Marks cannot be more than 100 or less than 0. Kindly re-enter\n");
                j--;
                continue;
            }
        }
    }
}

float calculateTotalMarks(StudentInformation *student)
{
    float totalMarks = 0;
    for(int i = 0; i<3; i++)
    {
        totalMarks += student->Marks[i];
    }
    return totalMarks;
}

float calculateAverage(float totalMarks)
{
    return totalMarks/3.0;
}

char assigningGrade(float averageMarks)
{
        if(averageMarks >= 85)
        return 'A';

        else if(averageMarks >= 70)
        return 'B';

        else if(averageMarks >= 50)
        return 'C';

        else if(averageMarks >= 35)
        return 'D';

        else if(averageMarks < 35)
        return 'F';
}

void assigningStars(char grade)
{
        switch(grade)
        {
            case 'A': 
            {
                printf("*****"); 
                break;
            }
            case 'B':
            {
                printf("****");
                break;
            }
            case 'C':
            {
                printf("***");
                break;
            }
            case 'D':
            {
                printf("**");
                break;
            }
        }
}

void printRollNumbers(StudentInformation *student, int numberOfStudents, int index) 
{
    if(index == numberOfStudents) 
    return;
    printf("%d ", student[index].RollNumber);
    printRollNumbers(student, numberOfStudents, index + 1);
}

void outputData(StudentInformation *student, int numberOfStudents)
{
    for(int i = 0; i< numberOfStudents; i++)
    {
        float totalMarks = calculateTotalMarks(&student[i]);
        float averageMarks = calculateAverage(totalMarks);
        char grade = assigningGrade(averageMarks);

        printf("\n");
        printf("\nRoll Number : %d", student[i].RollNumber);
        printf("\nName : %s", student[i].Name);
        printf("Total : %.2f", totalMarks);
        printf("\nAverage : %.2f", averageMarks);
        printf("\nGrade : %c", grade);
        if (grade != 'F') 
        {
            printf("\nPerformance : ");
            assigningStars(grade);
        }
        printf("\n");
    }
    printf("\nThe list of Roll Numbers : ");
    printRollNumbers(student, numberOfStudents, 0);
}

int main()
{
    int numberOfStudents;
    char choice;
    do
    {
        printf("\nEnter the number of Student you want to enter the record for: ");
        scanf("%d", &numberOfStudents);
        if(numberOfStudents <= 0)
        {
            printf("\nNumber of Student can not be 0 or negative\n");
            continue;
        }

        StudentInformation student[numberOfStudents];

        inputData(student, numberOfStudents);       
        outputData(student, numberOfStudents);
        
        printf("\n\nDo you want to continue (y/n): ");
        scanf(" %c", &choice);
    }while(choice != 'n');

    return 0;
}