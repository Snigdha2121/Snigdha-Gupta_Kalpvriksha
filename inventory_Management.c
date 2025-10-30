#include<stdio.h>
#include<stdlib.h>

typedef struct
{
    int ProductId;
    char ProductName[51];
    float Price;
    int Quantity;
} ProductDetails;

void showMenu()
{
    printf("\n========= INVENTORY MENU =========\n");
    printf("1. Add New Product\n");
    printf("2. View All Products\n");
    printf("3. Update Quantity\n");
    printf("4. Search Product by ID\n");
    printf("5. Search Product by Name\n");
    printf("6. Search Product by Price Range\n");
    printf("7. Delete Product\n");
    printf("8. Exit\n");
    printf("Enter your choice: ");
}

int getValidateInt(const char *prompt, int min, int max) 
{
    char input[50];
    int valid, value;
    while(1) 
    {
        printf("%s", prompt);
        scanf("%49s", input);
        valid = 1;

        for(int i = 0; input[i] != '\0'; i++) 
        {
            if(input[i] < '0' || input[i] > '9') 
            {
                valid = 0;
                break;
            }
        }

        if(!valid) 
        {
            printf("Invalid input. Digits only.\n");
            continue;
        }

        value = atoi(input);
        if(value < min || value > max) 
        {
            printf("Out of range (%d-%d). Try again.\n", min, max);
            continue;
        }
        return value;
    }
}

int isEmptyOrSpaces(const char *str) {
    for(int i = 0; str[i] != '\0'; i++) 
    {
        if(str[i] != ' ')
        return 0;
    }
    return 1;
}

int validateProductId(ProductDetails *product, int count) 
{
    int id, duplicate;
    while (1) {
        id = getValidateInt("Product ID: ", 1, 10000);
        duplicate = 0;

        for (int i = 0; i < count; i++) {
            if (product[i].ProductId == id) {
                printf("Duplicate ID. Enter a new one.\n");
                duplicate = 1;
                break;
            }
        }
        if (!duplicate) return id;
    }
}

void validateProductName(char *name)
{
    char ch;
    int namePos, overflow;

    while(1)
    {
        printf("Product Name: ");
        namePos = 0;
        overflow = 0;
        while ((ch = getchar()) == '\n');
        if (ch == EOF) continue;

        name[namePos++] = ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            if (namePos < 50) name[namePos++] = ch;
            else overflow = 1;
        }
        name[namePos] = '\0';

        if(overflow)
        {
            printf("Name too long (max 50 chars). Re-enter.\n");
            while ((ch = getchar()) != '\n' && ch != EOF);
            continue;
        }

        if(namePos == 0 || isEmptyOrSpaces(name)) 
        {
            printf("Product name cannot be empty. Re-enter.\n");
            continue;
        }
        break;
    }
}

float validateProductPrice() 
{ 
    char input[50]; 
    int dotCount, valid; 
    float value; 

    while(1) 
    { 
        printf("Product Price: "); 
        scanf("%49s", input); 
        dotCount = 0; 
        valid = 1; 

        for(int i = 0; input[i] != '\0'; i++) 
        { 
            if(input[i] == '.') dotCount++; 
            else if(input[i] < '0' || input[i] > '9') valid = 0; 
            if(dotCount > 1) valid = 0; 
        } 

        if(!valid) 
        { 
            printf("Invalid price. Only digits and one dot allowed.\n"); 
            continue; 
        } 

        value = atof(input); 
        if(value < 0 || value > 100000) 
        { 
            printf("Out of range (0-100000). Try again.\n"); 
            continue; 
        } 
        return value; 
    } 
}

int validateProductQuantity() 
{ 
    char input[50]; 
    int valid, value; 

    while(1) 
    { 
        printf("Product Quantity: "); 
        scanf("%49s", input); 
        valid = 1; 
        
        for(int i = 0; input[i] != '\0'; i++) 
        {
            if(input[i] < '0' || input[i] > '9') 
            {
                valid = 0; 
                break;
            }
        }

        if(!valid) 
        {
            printf("Invalid input. Please enter an integer number.\n"); 
            continue; 
        } 

        value = atoi(input); 
        if(value < 0 || value > 1000000) 
        { 
            printf("Out of range (0-1000000). Try again.\n"); 
            continue; 
        }
        return value; 
    } 
}

void getInitialProducts(ProductDetails **product, int *count, int maxProducts)
{
    *count = getValidateInt("\nEnter initial number of products: ", 1, maxProducts);

    *product = (ProductDetails *)calloc(*count, sizeof(ProductDetails));
    if(*product == NULL)
    {
        printf("\nMemory allocation failed.\n");
        *count = 0;
        return;
    }

    for(int i = 0; i < *count; i++)
    {
        printf("\nEnter details for product %d:\n", i + 1);
        (*product)[i].ProductId = validateProductId(*product, i);
        validateProductName((*product)[i].ProductName);
        (*product)[i].Price = validateProductPrice();
        (*product)[i].Quantity = validateProductQuantity();
    }
}

void addProduct(ProductDetails **product, int *count, int maxProducts)
{
    if(*count >= maxProducts)
    {
        printf("\nCannot add more products. Maximum limit reached.\n");
        return;
    }

    ProductDetails *temp = realloc(*product, (*count + 1) * sizeof(ProductDetails));
    if(temp == NULL) 
    {
    printf("\nMemory reallocation failed.\n");
    return;
    }
    *product = temp;

    printf("\nEnter new product details:\n");
    (*product)[*count].ProductId = validateProductId(*product, *count);
    validateProductName((*product)[*count].ProductName);
    (*product)[*count].Price = validateProductPrice();
    (*product)[*count].Quantity = validateProductQuantity();

    (*count)++;
}

void viewAllProducts(ProductDetails *product, int count)
{
    if(count == 0)
    {
        printf("\nNo products available.\n");
        return;
    }

    printf("\n========= PRODUCT LIST =========\n");
    for(int i = 0; i < count; i++)
    {
        printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
        product[i].ProductId, product[i].ProductName,
        product[i].Price, product[i].Quantity);
    }
}

void updateQuantity(ProductDetails *product, int count)
{
    int id, found = 0;
    printf("\nEnter Product ID to update quantity: ");
    while(scanf("%d", &id) != 1)
    {
        while(getchar() != '\n');
        printf("Invalid input. Enter a valid integer ID: ");
    }

    for(int i = 0; i < count; i++)
    {
        if(product[i].ProductId == id)
        {
            product[i].Quantity = validateProductQuantity();
            printf("\nQuantity updated successfully!\n");
            found = 1;
            break;
        }
    }
    if(!found) printf("\nProduct not found.\n");
}

int containsSubstring(char *text, char *pattern)
{
    for(int i = 0; text[i] != '\0'; i++)
    {
        int j = 0;
        while(pattern[j] != '\0' && text[i + j] == pattern[j]) j++;
        if(pattern[j] == '\0') return 1;
    }
    return 0;
}

void searchByID(ProductDetails *product, int count)
{
    int id;
    printf("\nEnter Product ID to search: ");
    while(scanf("%d", &id) != 1)
    {
        while(getchar() != '\n');
        printf("Invalid input. Enter a valid ID: ");
    }

    for(int i = 0; i < count; i++)
    {
        if(product[i].ProductId == id)
        {
            printf("\nProduct Found: Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
            product[i].ProductId, product[i].ProductName,
            product[i].Price, product[i].Quantity);
            return;
        }
    }
    printf("\nProduct not found.\n");
}

void searchByName(ProductDetails *product, int count)
{
    char key[51], ch;
    int i = 0, found = 0;

    printf("\nEnter name to search (partial allowed): ");
    while((ch = getchar()) != '\n');
    while(i < 50)
    {
        ch = getchar();
        if(ch == '\n' || ch == EOF) break;
        key[i++] = ch;
    }
    key[i] = '\0';

    for(int a = 0; a < count; a++)
    {
        if(containsSubstring(product[a].ProductName, key))
        {
            if(!found) printf("\nProducts Found:\n");
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
            product[a].ProductId, product[a].ProductName,
            product[a].Price, product[a].Quantity);
            found = 1;
        }
    }
    if(!found) printf("No products found.\n");
}

void searchByPriceRange(ProductDetails *product, int count)
{
    float min, max;
    int found = 0;

    printf("\nEnter minimum price: ");
    while(scanf("%f", &min) != 1)
    {
        while(getchar() != '\n');
        printf("Invalid input. Enter a valid number: ");
    }
    printf("Enter maximum price: ");
    while(scanf("%f", &max) != 1)
    {
        while(getchar() != '\n');
        printf("Invalid input. Enter a valid number: ");
    }

    if(min < 0 || max < 0 || min > max)
    {
        printf("\nInvalid range.\n");
        return;
    }

    for(int i = 0; i < count; i++)
    {
        if(product[i].Price >= min && product[i].Price <= max)
        {
            if(!found) printf("\nProducts in price range:\n");
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
            product[i].ProductId, product[i].ProductName,
            product[i].Price, product[i].Quantity);
            found = 1;
        }
    }
    if(!found) printf("No products found.\n");
}

void deleteProduct(ProductDetails **product, int *count)
{
    int id, found = 0;
    printf("\nEnter Product ID to delete: ");
    while(scanf("%d", &id) != 1)
    {
        while(getchar() != '\n');
        printf("Invalid input. Enter a valid ID: ");
    }

    for(int i = 0; i < *count; i++)
    {
        if((*product)[i].ProductId == id)
        {
            for(int j = i; j < *count - 1; j++)
                (*product)[j] = (*product)[j + 1];

            (*count)--;
            found = 1;

            if(*count == 0)
            {
                free(*product);
                *product = NULL;
            }
            else
            {
                ProductDetails *temp = realloc(*product, (*count) * sizeof(**product));
                if(temp == NULL)
                {
                    printf("\nMemory reallocation failed (data preserved).\n");
                    return;
                }
                *product = temp;
            }

            printf("\nProduct deleted successfully!\n");
            break;
        }
    }
    if(!found) printf("\nProduct not found.\n");
}

int main()
{
    const int maxProducts = 100;
    ProductDetails *product = NULL;
    int count = 0, choice;

    getInitialProducts(&product, &count, maxProducts);
    if(count == 0) return 0;

    do
    {
        showMenu();
        if (scanf("%d", &choice) != 1)
        {
            while(getchar() != '\n');
            printf("\nInvalid choice. Enter a number between 1 and 8.\n");
            continue;
        }

        switch(choice)
        {
            case 1: addProduct(&product, &count, maxProducts); break;
            case 2: viewAllProducts(product, count); break;
            case 3: updateQuantity(product, count); break;
            case 4: searchByID(product, count); break;
            case 5: searchByName(product, count); break;
            case 6: searchByPriceRange(product, count); break;
            case 7: deleteProduct(&product, &count); break;
            case 8:
                free(product);
                printf("\nMemory released successfully. Exiting program...\n");
                return 0;
            default:
                printf("\nInvalid choice. Try again.\n");
        }
    } while(choice != 8);

    return 0;
}