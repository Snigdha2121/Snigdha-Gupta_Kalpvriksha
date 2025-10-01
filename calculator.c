#include <stdio.h>
#include <stdlib.h>
#define max 100

// Below is  function for creating a stack and all its operations like initializing a stack, push(), pop()
typedef struct{
    int top;
    int elements[max];
} stack;

void init_stack(stack *s)
{
    s->top = -1;
}

void push(stack *s, int value)
{
    if(s->top == max-1)
    {
        printf("Overflow");
        exit(1);
    }

    else
    s->elements[++(s->top)] = value;
}

int pop(stack *s)
{
    if(s->top == -1)
    {
        printf("Underflow Condition");
        exit(1);
    }

    else 
    return (s->elements[(s->top)--]);
}


// below is a function for creating a calculator for evaluating expression
int calculator(char exp[], stack *s){
    int i = 0;
    char op = '+';
    while(exp[i] != '\0')
    {
        char c = exp[i];
        if(c == ' ' || c == '\n')
        {
            i++;
            continue;
        }

        else if(c >= '0' && c<= '9')
        {
            int num = 0;
            while(exp[i] >= '0' && exp[i]<= '9')
            {
                num = num * 10 + (exp[i] - '0');
                i++;
            }
            i--;

            if(op == '+') 
            push(s, num);

            else if(op == '-') 
            push(s, -num);

            else if(op == '*') 
            {
                int prev = pop(s);
                push(s, prev*num);
            }
            else if(op == '/') 
            {
                if(num == 0)
                {
                    printf("Error: Division by Zero\n");
                    exit(1);
                }
                int prev = pop(s);
                push(s, prev/num);
            }
        }
        else if(c == '+' || c == '-' || c == '*' || c == '/')
        {
            op = c;
        }

        else
        {
            printf("Error: Invalid Expression\n");
            exit(1);
        }
    i++;
    }
    int result = 0; 
    while(s->top != -1)
    {
        result += pop(s);
    }

    return result;
}


int main()
{
    stack s1;
    init_stack(&s1);

    char expression[max];

    printf("Enter  string you want to evaluate: ");
    fgets(expression, sizeof(expression), stdin);

    printf("%d\n", calculator(expression, &s1));
    return 0;
}