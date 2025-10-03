#include <stdio.h>

typedef struct
{
    int id;
    char name[500];
    int age;
} user;

// Function to create user
void create()
{
    user u;
    FILE *fptr;
    FILE *check;
    FILE *check_id;
    
    check = fopen("users.txt", "r");
    if(check == NULL)
    {
        fptr = fopen("users.txt", "w");
        if(fptr == NULL)
        {
            printf("error in creating file");
            return;;
        }
        fprintf(fptr, "Id\tName\tAge\n");
        fclose(fptr);
    }
    else
    {
        fclose(check);
    }

    fptr = fopen("users.txt", "a");
    if(fptr == NULL)
    {
        printf("Error opening file");
        return;
    }

    printf("Enter id: ");
    scanf("%d", &u.id);

    check_id = fopen("users.txt", "r");
    if(check_id != NULL)
    {
        char header[100];
        fgets(header, sizeof(header), check_id);

        user temp;
        while (fscanf(check_id, "%d\t%499[^\t]\t%d\n", &temp.id, temp.name, &temp.age) == 3) 
        {
        if (temp.id == u.id) 
        {
            printf("Id already exists\n");
            fclose(check_id);
            fclose(fptr);
            return;
        }
        } 
    }

    printf("Enter name: ");
    scanf(" %[^\n]", u.name);
    printf("Enter age: ");
    scanf("%d", &u.age);

    fprintf(fptr, "%d\t%s\t%d\n", u.id, u.name, u.age);
    fclose(fptr);
    fclose(check_id);
}

// Function to read user
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

// Function to update user by "ID"
void update()
{
    int id, found = 0;
    printf("Enter the ID of user you want to update: ");
    scanf("%d", &id);

    FILE *fptr;
    FILE *temp;
    fptr = fopen("users.txt", "r");
    temp = fopen("temp.txt", "w");

    if(fptr == NULL || temp == NULL)
    {
        printf("error opening file");
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
            printf("Enter new name to update: ");
            scanf(" %[^\n]", u.name);
            printf("Enter new age to update: ");
            scanf("%d", &u.age);
        }
        fprintf(temp, "%d\t%s\t%d\n", u.id, u.name, u.age);
    }
    fclose(fptr);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    if(found == 0)
    {
        printf("User not found\n");
    }
}

// Function to delete user by "id"
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