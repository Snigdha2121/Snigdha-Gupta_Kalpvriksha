#include <stdio.h>
#define MAX 100

// defining a stack structure with top and array of elements with maximum capacity 100
typedef struct
{
    int top;
    int elements[MAX];
} stack;

// initialize a stack with top as -1    
void init_stack(stack *s)
{
    s->top = -1;
}

// if stack is full, throw error "overflow condition", else increment top and insert value at new position
int push(stack *s, int value, int *status)
{
    if(s->top == MAX-1)
    {
        printf("Error: Stack Overflow\n");
        *status = -1;
        return 0;
    }
    s->elements[++(s->top)] = value;
    return 0;
}

// if stack is empty show error message, else return the top value and decrement the top by 1
int pop(stack *s, int *status)
{
    if(s->top == -1)
    {
        printf("Error: Stack Underflow\n");
        *status = -1;
        return 0;
    }
    return s->elements[(s->top)--];
}

int stack_expression(stack *s, int num, char op, int *status)
{
    // push digit to stack as is if + op, -digit if - op, if multply or divide then pop prev digit perform operation and push back to the stack
    int prev;
    if(op == '+') push(s, num, status);
    else if(op == '-') push(s, -num, status);
    else if(op == '*') {
        prev = pop(s, status);
        if(*status == -1) return 0;
        push(s, prev*num, status);
    }
    else if(op == '/') 
    {
        if(num == 0) {
            printf("Error: Division by Zero\n");
            *status = -1;
            return 0;
        }
        prev = pop(s, status);
        if(*status == -1) return 0;
        push(s, prev/num, status);
    }
    return 0;
}

// this calculates the sum of all the digits which are left in the stack and return the result
int calc_stack(stack *s, int *status)
{
    int result = 0;
    int val;
    while(s->top != -1)
    {
        val = pop(s, status);
        if(*status == -1) return 0;
        result += val;
    }
    return result;
}

// parsing expression character by character, identify each character, update status = -1 if invalid charcater is found
int parse_expression(char exp[], stack *s, int *status){
    int i = 0;
    char op = '+';

    while(exp[i] != '\0') 
    {
        // if character is blank space or new line it is valid char, increment to next character and continue iterating
        if(exp[i] == ' ' || exp[i] == '\n') 
        {
            i++;
            continue;
        }

        // unary minus handling
        int sign = 1;
        if(exp[i] == '-' && (i == 0 || exp[i-1]=='+' || exp[i-1]=='-' || exp[i-1]=='*' || exp[i-1]=='/')) {
            sign = -1;
            i++;
        }

        // if char is between 0 and 9 it is digit, iterate till all subsequent characters are digits, multiply number  with sign
        if(exp[i] >= '0' && exp[i] <= '9')
        {
            int num = 0;
            while(exp[i] >= '0' && exp[i] <= '9')
            {
                num = num * 10 + (exp[i] - '0');
                i++;
            }
            num *= sign;
            i--;
            stack_expression(s, num, op, status);
            if(*status == -1) return 0;
        }

        // check if the character is a valid operator + - * /
        else if(exp[i] == '+' || exp[i] == '-' || exp[i] == '*' || exp[i] == '/')
        {
            op = exp[i];
        }

        // if the character is anything other than the digit, operator or blank space it is invalid thus throw error message
        else 
        {
            printf("Error: Invalid Expression\n");
            *status = -1;
            return 0;
        }
        i++;
    }
    return calc_stack(s, status);
}

int main()
{
    stack s1;
    char choice;
    char expression[MAX];

    do {
        init_stack(&s1);
        int status = 0;

        printf("Enter string you want to evaluate: ");
        fgets(expression, sizeof(expression), stdin);

        int result = parse_expression(expression, &s1, &status);

        if(status == 0)
            printf("Result = %d\n", result);

        printf("Do you want to calculate again? (y/n): ");
        scanf(" %c", &choice);
        while(getchar() != '\n');
    } while(choice != 'n');

    return 0;
}