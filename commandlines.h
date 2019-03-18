#ifndef PROMPT_H_INCLUDED
#define PROMPT_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IN 1
#define OUT 0

typedef struct{
    char* userLine;                 //The original line taken from the command line or batchfile line

    char** words;                   //Pointers to newly allocated strings that hold the parsed words from the original line.
    unsigned int numWords;          //Number of strings. This includes commands, arguments, and special characters such as '<', '>', '|', and ">>".	
				
	unsigned int inputRedirExists;	//Takes on the value 0 if there is no input redirection. Otherwise it takes on  the value 1 if there is.
	unsigned int outputRedirExists;	//Takes on the value 0 if there is no output redirection. Otherwise it takes on  the value 1 if there is.
	
	int inputSymbIndex;             //Index of the words array in which the '<' symbol is found. Index is initialized to -1.
    int outputSingleSymbIndex;      //Index of the words array in which the '>' symbol is found. Index is initialized to -1.
    int outputDoubleSymbIndex;      //Index of the words array in which the '>>' symbol is found. Index is initialized to -1.
    char* inputRedirFile;         	//Pointer to the string (in the words array) that holds the name of the file in which the input is to be redirected to, if it exists.
    char* outputRedirFile;        	//Pointer to the string (in the words array) that holds the name of the file in which the output is to be redirected to, if it exists.

    int numPipes;					//Holds the value of the number of pipes in the command. Initialized to 0.
    int* pipeIndices;				//Keeps a record of the indices of each pipe that are inside the words array.
	
	char*** argSets;                //argv[i] corresponds to a set of arguments. argv[i][j] corresponds to an individual argument.
    unsigned int numSets;           //argSets[set p][word q] points to the q'th word inside set p.
                                    //The number of sets of arguments is 1 greater than the number of pipes.
                                    //Each "set" of arguments corresponds to the arguments between pipes (also the arguments before the first pipe, and after the last pipe).
									
	char** programSet;				//Pointers to the strings in the words array that follow a special character ('<', '>', '|', and ">>") and to the command at the start of a command.
	unsigned int numPrograms;		//numProgram is used to count the number of programs

    unsigned int exeInBackground;

} commandLine;

int get_command_line(commandLine *cLine, char* line);

int count_words(commandLine* cLine);
int count_pipes(commandLine* cLine);
int count_args_in_set(char** set);			//Not used in commandlines.c
int check_redirection(commandLine* cLine);

void set_words(commandLine* cLine);
void set_initials(commandLine* cLine);
int set_args_and_special(commandLine* cLine);

void print_cline_struct(commandLine* cLine);
void print_words(commandLine* cLine);
void print_programs(commandLine* cLine);
void print_args(commandLine* cLine);
void print_special(commandLine* cLine);

void free_command_line(commandLine* cLine);


#endif // PROMPT_H_INCLUDED
