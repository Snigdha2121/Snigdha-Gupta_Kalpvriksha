#include <stdio.h>

typedef struct
{
    int id;
    char name[500];
    int age;
} user;

void file_exists()
{
    FILE *check;
    check = fopen("users.txt", "r");
    if(check == NULL)
    {
        check = fopen("users.txt", "w");
        if(check == NULL)
        {
            printf("error in creating file");
            return;
        }
        fprintf(check, "Id\tName\tAge\n");
        fclose(check);
    }
    else
    {
        fclose(check);
    }
}

int id_exists(int id)
{
    FILE *fptr = fopen("users.txt", "r");
    if (!fptr) return 0;

    char header[100];
    fgets(header, sizeof(header), fptr);
    user temp;
    while (fscanf(fptr, "%d\t%499[^\t]\t%d\n", &temp.id, temp.name, &temp.age) == 3)
    {
        if (temp.id == id)
        {
            fclose(fptr);
            return 1;
        }
    }
    fclose(fptr);
    return 0;
}

void create()
{
    user u;
    FILE *fptr;

    file_exists();

    fptr = fopen("users.txt", "a");
    if(fptr == NULL)
    {
        printf("Error opening file");
        return;
    }

    printf("Enter id: ");
    scanf("%d", &u.id);
    if(id_exists(u.id))
    {
        printf("Id already exists\n");
        fclose(fptr);
        return;
    }

    printf("Enter name: ");
    scanf(" %[^\n]", u.name);
    printf("Enter age: ");
    int result = scanf("%d", &u.age);
    if(result != 1 || u.age < 0 || u.age > 120)
    {
        printf("Invalid age. Record did not create\n");
        while(getchar() != '\n');
        fclose(fptr);
        return;
    }
    while(getchar() != '\n');

    fprintf(fptr, "%d\t%s\t%d\n", u.id, u.name, u.age);
    fclose(fptr);
}

void read()
{
    user u;
    FILE *fptr;
    fptr = fopen("users.txt", "r"); 

    if(fptr == NULL)
    {
        printf("No user found");
        return;;
    }

    char buffer[200];
    while (fgets(buffer, sizeof(buffer), fptr) != NULL) 
    {
        printf("%s", buffer);
    }
    fclose(fptr);
}

void update()
{
    int id;
    printf("Enter the ID of user you want to update: ");
    if(scanf("%d", &id) != 1) {
        printf("Invalid id\n");
        while(getchar() != '\n');
        return;
    }
    while(getchar() != '\n');

    if(!id_exists(id)) {
        printf("User not found\n");
        return;
    }

    FILE *fptr = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if(fptr == NULL || temp == NULL) {
        printf("Error opening file\n");
        return;
    }

    char header[100];
    fgets(header, sizeof(header), fptr);
    fprintf(temp, "%s", header);

    user u;
    while(fscanf(fptr, "%d\t%499[^\t]\t%d\n", &u.id, u.name, &u.age) == 3) {
        if(u.id == id) {
            printf("Enter new name to update: ");
            scanf(" %[^\n]", u.name);
            while(getchar() != '\n');

            printf("Enter new age to update: ");
            int result = scanf("%d", &u.age);
            if(result != 1 || u.age < 0 || u.age > 120) {
                printf("Invalid age. Record did not update\n");
                while(getchar() != '\n');
                fclose(fptr);
                fclose(temp);
                remove("temp.txt");
                return;
            }
            while(getchar() != '\n');
        }
        fprintf(temp, "%d\t%s\t%d\n", u.id, u.name, u.age);
    }
    fclose(fptr);
    fclose(temp);
    remove("users.txt");
    rename("temp.txt", "users.txt");
}

void delete()
{
    int id, found = 0;
    printf("Enter the ID of user you want to delete: ");
    scanf("%d", &id);

    FILE *fptr;
    FILE *temp;
    fptr = fopen("users.txt", "r");
    temp = fopen("temp.txt", "w");

    if(fptr == NULL || temp == NULL)
    {
        printf("error opening file\n");
        return;;
    }

    char header[100];
    fgets(header, sizeof(header), fptr);
    fprintf(temp, "%s", header);

    user u;
    while(fscanf(fptr, "%d\t%499[^\t]\t%d\n", &u.id, u.name, &u.age) == 3)
    {
        if (u.id == id) 
        {
            found = 1;
            continue;
        }
        fprintf(temp, "%d\t%s\t%d\n", u.id, u.name, u.age);
    }
    fclose(fptr);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    if(found == 0)
    {
        printf("User not found.\n");
    }
}

int main()
{
    int choice;

    do
    {
        printf("1. Create User\n");
        printf("2. Read User\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch(choice)
        {
        case 1: create(); break;
        case 2: read(); break;
        case 3: update(); break;
        case 4: delete(); break;
        case 5: printf("exit"); break;
        default: printf("Invalid choice");
        }
    }while(choice != 5);

    return 0;
}