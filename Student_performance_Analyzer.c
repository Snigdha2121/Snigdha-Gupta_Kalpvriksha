#include<stdio.h>

typedef struct
{
    int rollNumber;
    char name[100];
    float marks1, marks2, marks3, totalMarks, averageMarks;
    char grade;
    char *stars;
} student;

void inputData(student *s, int numberOfStudents)
{
    char buffer[200];
    for(int i = 0; i<numberOfStudents; i++)
    {
        printf("Enter the Roll Number, Name, Marks1, Marks2, Marks3 for %d student: ", i+1);
        fgets(buffer, sizeof(buffer), stdin);
        int itemsRead = sscanf(buffer, "%d %[^\t\n0123456789] %f %f %f", &s[i].rollNumber, s[i].name, &s[i].marks1, &s[i].marks2, &s[i].marks3);
        
        if(itemsRead !=5)
        {
            printf("\nInvalid input format please enter all the values in space seperated format\n");
            i--;
            continue;
        }
        
        if((s[i].marks1 < 0 || s[i].marks1 > 100) || (s[i].marks2 < 0 || s[i].marks2 > 100) || (s[i].marks3 < 0 || s[i].marks3 > 100))
        {
            printf("\nInvalid Marks. Marks cannot be more than 100 or less than 0. Kindly re-enter\n");
            i--;
            continue;
        }

        int duplicate = 0;
        for (int j = 0; j < i; j++) 
        {
            if (s[i].rollNumber == s[j].rollNumber) 
            {
                printf("\nRoll number cannot repeat. Please re-enter.\n");
                duplicate = 1;
                break;
            }
        }
        if (duplicate) 
        {
            i--;
            continue;
        }
    }    
}

void calculateTotalMarks(student *s, int numberOfStudents)
{
    for(int i = 0; i<numberOfStudents; i++)
    {
        s[i].totalMarks = s[i].marks1 + s[i].marks2 + s[i].marks3;
    }
}

void calculateAverage(student *s, int numberOfStudents)
{
    for(int i = 0; i<numberOfStudents; i++)
    {
        s[i].averageMarks = s[i].totalMarks / 3.0;
    }
}

void assigningGrade(student *s, int numberOfStudents)
{
    for(int i = 0; i<numberOfStudents; i++)
    {
        if(s[i].averageMarks >= 85)
        s[i].grade = 'A';

        else if(s[i].averageMarks >= 70)
        s[i].grade = 'B';

        else if(s[i].averageMarks >= 50)
        s[i].grade = 'C';

        else if(s[i].averageMarks >= 35)
        s[i].grade = 'D';

        else if(s[i].averageMarks < 35)
        s[i].grade = 'F';
    }
}

void assigningStars(student *s, int numberOfStudents)
{
    for(int i = 0; i<numberOfStudents; i++)
    {
        switch(s[i].grade)
        {
            case 'A': 
            {
                s[i].stars = "*****"; 
                break;
            }
            case 'B':
            {
                s[i].stars = "****";
                break;
            }
            case 'C':
            {
                s[i].stars = "***";
                break;
            }
            case 'D':
            {
                s[i].stars = "**";
                break;
            }
            case 'F':
            {
                s[i].stars = "";
                break;
            }
        }
    }
}

void outputData(student *s, int numberOfStudents)
{
    for(int i = 0; i< numberOfStudents; i++)
    {
        printf("\n");
        printf("Roll Number : %d\n", s[i].rollNumber);
        printf("Name : %s\n", s[i].name);
        printf("Total : %.2f\n", s[i].totalMarks);
        printf("Average : %.2f\n", s[i].averageMarks);
        printf("Grade : %c\n", s[i].grade);
        printf("Performance : %s\n", s[i].stars);
    }
    printf("\nThe list of Roll Numbers : ");

    for(int i = 0; i<numberOfStudents; i++)
    {
        printf("%d ", s[i].rollNumber);
    }
}

int main()
{
    int numberOfStudents;
    char choice;
    do
    {
        printf("\nEnter the number of students you want to enter the record for: ");
        scanf("%d", &numberOfStudents);
        if(numberOfStudents <= 0)
        {
            printf("\nNumber of students can not be 0 or negative\n");
            continue;
        }
        getchar();

        student s[numberOfStudents];

        inputData(s, numberOfStudents);       
        calculateTotalMarks(s, numberOfStudents);
        calculateAverage(s, numberOfStudents);
        assigningGrade(s, numberOfStudents);
        assigningStars(s, numberOfStudents);
        outputData(s, numberOfStudents);
        
        printf("\nDo you want to continue (y/n): ");
        scanf(" %c", &choice);
    }while(choice != 'n');

    return 0;
}