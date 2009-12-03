/*
 * COPYRIGHT (C) 2009 Sean Bruno and Tyler Hayes, Portland State University
 *
 * test.c
 */


#include <stdio.h>
#include <stdlib.h>
#include "test.h"


//function delarations
void checkQuit (char * input);
int checkSyntax(char * input);


/*---------------------------------- MAIN ------------------------------------*/

int main () {
    char buffer[MAX_BUF_LENGTH+1];
    char * fgetsCatcher;
    int light = 0;
    int color = 0;

    while (1) {
        outputPrompt();
        //get user input
        fgetsCatcher = fgets(buffer, MAX_BUF_LENGTH, stdin);
        //check fgets error
        if (!fgetsCatcher) {
            printf("\n***Error in fgets(). Exiting.\n\n");
            exit(-1);
        }
        //check for user quitting (typing 'q')
        checkQuit(buffer);
        //check syntax of input
        if (checkSyntax(buffer) < 0) {
            printf("\n***Syntax error. Try again.***\n");
            continue;
        }
        //get the light number
        light = buffer[0]-'0';
        //get the color
        switch (buffer[2]) {
            case 'r': color = RED;
                      break;
            case 'g': color = GREEN;
                      break;
            case 'b': color = BLUE;
                      break;
        }

        //ioctl

    }

    //this will never execute
    exit(0);
}


/*-------------------------------- functions ---------------------------------*/

/*
 * checkQuit.  Exit program if user enters "q".
 */
void checkQuit (char * input)
{
    if (strcmp(input, "q") == 0) {
        printf("\n\n");;
        exit(0);
    }
}

/*
 * checkSyntax.  Checks the syntax of user's input.  Returns -1 on error.
 */
int checkSyntax (char * input)
{
    //check light is 1 or 2
    if (input[0] != '1' && input[0] != '2')
        return -1;
    //check for '='
    if (input[1] != '=')
        return -1;
    //check color is 'r', 'g', or 'b'
    if (input[2] != 'r' &&
        input[2] != 'g' &&
        input[2] != 'b')
        return -1;

    return 0;
}
