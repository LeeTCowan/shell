#include "commandlines.h"

/**
 * This function accepts a pointer to an empty commandLine struct (cLine), reads a line from stdin, analyzes
 * the input, and then sets cLine's variables if the line is a valid command.
 * 0 is returned for success. Negative numbers are returned if something fails. 1 is returned if the command line was empty.
 */
int get_command_line(commandLine *cLine, char* line){
    cLine->userLine = NULL;
    size_t len = 0;
    ssize_t read;

	if(line == NULL){
		read = getline(&(cLine->userLine), &len, stdin);
	}
	else{
		cLine->userLine = (char*)malloc(sizeof(char) * (strlen(line) + 1));
		cLine->userLine[0] = '\0';
		strcat(cLine->userLine, line);
		read == 0;
	}

    if(read == -1){
        perror("Error occurred within the getline() function. Try again.");
        return -1;
    }
    else{
        if(count_words(cLine) < 1){
            return 1;
        }
        else{
            cLine->words = (char**)calloc((cLine->numWords) + 1, sizeof(char*));	//check if failed

            set_words(cLine);
			
            if(count_pipes(cLine) < 0){
                return -2;
            }
			
			if(check_redirection(cLine) < 0){
				return -3;
			}
			
			set_initials(cLine);

            if(set_args_and_special(cLine) != 0){
                return -4;
            }
			
            return 0;
        }
    }

}


/**
 * This function counts the number of "words" the userLine has inside of the commandLine struct passed.
 * A word is any printable character sequence that does not include a space or tab. Quotation marks are special however, in that
 * spaces will not be considered as seperating two words if they all occur within a set of quotes.
 * Multiple internal sets of quotes are not supported.
 * The number of words is returned and cLine->numWords is set.
 * -1 is returned if there is a misuse of quotation marks.
 */
int count_words(commandLine* cLine){
    char* line = cLine->userLine;
    unsigned int count = 0;
    int state = OUT;
    int quoteState = OUT;

    for(int i = 0; line[i] != '\n' && line[i] != EOF  && line[i] != '\0'; i++) {
        if(state == OUT && ((line[i] >= 35 && line[i] <= 126) || line[i] == 33)){
            state = IN;
            count++;
        }
        else if(state == OUT && (line[i] == '"')){
            state = IN;
            quoteState = IN;
            count++;
        }
        else if(state == IN && quoteState == OUT && line[i] == '"'){
            quoteState = IN;
        }
        else if(state == IN && quoteState == IN && line[i] == '"'){
            quoteState = OUT;
        }
        else if(state == IN && quoteState == OUT && (line[i] == ' ' || line[i] == '\n' || line[i] == '\t' ))
            state = OUT;
        else		//including if(state == IN && quoteState == IN && (line[i] == ' ' || line[i] == '\n' || line[i] == '\t')){
            ;
	}
    if(quoteState == IN){
        printf("Misuse of quotation marks. See the readme for more information.\n");
        cLine->numWords = -1;
        return -1;
    }

    cLine->numWords = count;

    return count;
}

/**
 * This function takes in a pointer to a commandLine struct (cLine). The function assumes that the userLine variable contains a string,
 * and also assumes that the words pointer was properly allocated the correct amount of memory for pointers.
 * The function parses the userLine string into "words" as is defined as any sequence of printable characters that does not include a space
 * or tab. Quotation marks are special however, in that spaces will not be considered as seperating two words if they all occur within a set of quotes.
 * Multiple internal sets of quotes are not supported.
 */
void set_words(commandLine* cLine){
    char buffer[200];
    int bufferIndex = 0;
    int bufferWordLength = 0;
    int wordIndex = 0;
    int state = OUT;
    int quoteState = OUT;
    char c;

    c = cLine->userLine[0];
    for(int i = 1; c != '\n' && c != EOF  && c != '\0'; i++) {
        if(state == OUT && ((c >= 35 && c <= 126) || c == 33)){
            state = IN;
            buffer[bufferIndex] = c;
            bufferIndex++;
        }
        else if(state == OUT && (c == '"')){    //Does not put the quote mark inside the word.
            state = IN;
            quoteState = IN;
        }
        else if(state == IN && quoteState == IN && ((c >= 35 && c <= 126) || c == 32 || c == 33)){
            buffer[bufferIndex] = c;
            bufferIndex++;
        }
        else if(state == IN && quoteState == IN && c == '"'){   //Does not put the quote mark inside the word.
            quoteState == OUT;
        }
        else if(state == IN && quoteState == OUT && ((c >= 35 && c <= 126) || c == 33)){
            buffer[bufferIndex] = c;
            bufferIndex++;
        }
        else if(state == IN && quoteState == OUT && c == '"'){
            quoteState = IN;
        }
        else if(state == IN && quoteState == OUT && (c == ' ' || c == '\t')){
            buffer[bufferIndex] = '\0';
            bufferWordLength = strlen(buffer);

            cLine->words[wordIndex] = (char*)calloc(bufferWordLength + 1, sizeof(char));           //check if failed
            strcat(cLine->words[wordIndex], buffer);

            wordIndex++;
            bufferWordLength = 0;
            bufferIndex = 0;
            state = OUT;
        }
        else
            ;   //do nothing

        c = cLine->userLine[i];
    }

    if(state == IN){
        buffer[bufferIndex] = '\0';
        bufferWordLength = strlen(buffer);

        cLine->words[wordIndex] = (char*)calloc(bufferWordLength + 1, sizeof(char));             //check if failed
        strcat(cLine->words[wordIndex], buffer);

        wordIndex++;
    }

    cLine->words[wordIndex] = NULL;

    return;
}


/**
 * This function returns the number of pipes found in the words array within the commandLine struct (cLine) passed.
 * cLine->numPipes is set.
 * -1 is returned if there is an invalid word involving a pipe.
 */
int count_pipes(commandLine* cLine){
    int count = 0;

    for(int i = 0; cLine->words[i] != NULL ; i++){
        if(cLine->words[i][0] == '|'){
            if(cLine->words[i][1] != '\0'){
                cLine->numPipes = -1;
                return -1;
            }
            else{
                count++;
            }
        }
    }

    cLine->numPipes = count;

    return count;
}

/**
 * This function counts the number of strings there are in a set of strings and returns that value.
 */
int count_args_in_set(char** set){
	int count = 0;
	
	for(int i = 0 ; set[i] != NULL ; i++){
		count++;
	}
	
	return count;
}

/**
 * This function checks the words array inside the commandLine struct (cLine) passed for redirection symbols ('<' or '>' or '>>')
 * and sets the cLine variables "inputRedirExists" and "outputRedirExists" appropriately.
 * -1 is returned if there is an invalid word involving a redirection symbol (there are no other characters next to a symbol).
 * 0 is returned if there were no errors. A negative number is returned if there were.
 */
int check_redirection(commandLine* cLine){
	cLine->inputRedirExists = 0;
	cLine->outputRedirExists = 0;
	
	for(int i = 0; cLine->words[i] != NULL ; i++){
        if(cLine->words[i][0] == '<'){
            if(cLine->words[i][1] != '\0'){
                printf("Bad command: \"%s\". See the readme.\n", cLine->words[i]);	
                return -1;
            }
            else{
                cLine->inputRedirExists = 1;
            }
        }
		else if(cLine->words[i][0] == '>'){
			if(cLine->words[i][1] == '>'){
				if(cLine->words[i][2] != '\0'){
					printf("Bad command: \"%s\". See the readme.\n", cLine->words[i]); 
					return -2;
				}
				else{
					cLine->outputRedirExists = 1;
				}
			}
			else{	
				if(cLine->words[i][1] != '\0'){
					printf("Bad command: \"%s\". See the readme.\n", cLine->words[i]); 
					return -3;
				}
				else{
					cLine->outputRedirExists = 1;
				}
			}
        }
    }
	
}

/**
 * This function sets some initial values in the commandLine struct pointed to by the passed in cLine variable. It is assumed that
 * userLine, numWords, numSets, numPipes, inputRedirExists, and outputRedirExists already have values.
 * programSet, pipeIndices, and argSets are all allocated memory in this function.
 */
void set_initials(commandLine* cLine){
	int maxArgsInASet = cLine->numWords - (2 * cLine->numSets);        //With any command, there are (numPipes + 1) program/file names. The difference between this and numWords is the number of arguments.
	cLine->numSets = cLine->numPipes + 1;
    cLine->inputRedirFile = NULL;
    cLine->outputRedirFile = NULL;
    cLine->inputSymbIndex = -1;
    cLine->outputSingleSymbIndex = -1;
    cLine->outputDoubleSymbIndex = -1;
	cLine->numPrograms = 1 + cLine->numPipes;

	cLine->programSet = (char**)malloc(sizeof(int*) * cLine->numPrograms + 1);
	for(int i = 0 ; i <= cLine->numPrograms; i++){
			cLine->programSet[i] = NULL;
	}
	
    cLine->pipeIndices = (int*)malloc(sizeof(int) * (cLine->numSets));     //The number of pipes is 1 less than the number of sets. This array should be terminated with -1.
    cLine->pipeIndices[0] = -1;
    
	cLine->argSets = (char***)malloc(sizeof(char**) * (cLine->numSets + 1));    //The +1 is for a NULL termination
    for(int j = 0 ; j < cLine->numSets ; j++){
        cLine->argSets[j] = (char**)malloc(sizeof(char*) * (maxArgsInASet  + 1));	//Allocate enough room in each arg set so that any 1 can hold the max number of arguments.
        cLine->argSets[j][0] = NULL;
    }
	cLine->argSets[cLine->numSets] = NULL;
	
	return;
}


/**
 * This function sets the variables argSets, the programSet, inputSymbIndex, inputRedirFile, outputSymbIndex, outputRedirFile, and
 * exeInBackGround of the passed in commandLine struct (cLine).
 * The function also does some bad input checking: misuse of redirection symbols, too many redirection symbols, misplacement of redirection symbols,
 * misuse of pipe symbols, and misuse of the background execution symbol.
 */
int set_args_and_special(commandLine* cLine){
    int inputRedirCount = 0;
    int outputSingleRedirCount = 0;
    int outputDoubleRedirCount = 0;
    int argSetCount = 0;
    int argCount = 0;
    int pipeCount = 0;
	int programCount = 0;
    char* currentWord;
    char* nextWord;
	
	cLine->programSet[programCount] = cLine->words[0];
	programCount++;

    for(int i = 1; i < cLine->numWords ; i++){
        currentWord = cLine->words[i];
        nextWord = cLine->words[i+1];

        if(currentWord[0] == '<'){
            if(inputRedirCount > 0){
                printf("Input may only be redirected once. Command rejected. See the readme.\n");
                return 1;
            }
            else if(currentWord[1] != '\0'){
                printf("Bad command: \"%s\". See the readme.\n", currentWord);
                return 2;
            }
            else if(nextWord == NULL || nextWord[0] == '>' || nextWord[0] == '|' || nextWord[0] == '<' || nextWord[0] == '&'){
                printf("Invalid use of redirection: \"%s %s\"\n", currentWord, nextWord);
                return 3;
            }
            else if(pipeCount > 0){
                printf("Invalid use of redirection. Attempted to redirect input after a pipe.\n");
                return 4;
            }
            else{
                cLine->inputSymbIndex = i;
                cLine->inputRedirFile = cLine->words[i + 1];
                inputRedirCount++;
				
				i++;
            }
        }
        else if(currentWord[0] == '>'){
            if(outputSingleRedirCount > 0 || outputDoubleRedirCount > 0){
                printf("Output may only be redirected once. Command rejected. See the readme.\n");
                return 1;
            }
            else if(currentWord[1] != '\0' && currentWord[1] != '>'){
                printf("Bad command: \"%s\". See the readme.\n", currentWord);
                return 2;
            }
            else if(nextWord == NULL || nextWord[0] == '>' || nextWord[0] == '|' || nextWord[0] == '<' || nextWord[0] == '&'){
                printf("Invalid use of redirection symbol: \"%s %s\"\n", currentWord, nextWord);
                return 3;
            }
            else if(currentWord[1] == '>'){
                if(currentWord[2] != '\0'){
                    printf("Bad command: \"%s\". See the readme.\n", currentWord);
                    return 5;
                }
                else{
                    cLine->outputDoubleSymbIndex = i;
                    cLine->outputRedirFile = cLine->words[i + 1];
                    outputDoubleRedirCount++;
					
					i++;
                }
            }
            else{
                cLine->outputSingleSymbIndex = i;
                cLine->outputRedirFile = cLine->words[i + 1];
                outputSingleRedirCount++;
				
				i++;
            }
        }
        else if(currentWord[0] == '|'){
            if(outputSingleRedirCount > 0 || outputDoubleRedirCount > 0){
                printf("Attempted to pipe output to \"%s\" but output was already redirected to \"%s\".\n", nextWord, cLine->outputRedirFile);
                return 6;
            }
            else if(nextWord == NULL || nextWord[0] == '>' || nextWord[0] == '|' || nextWord[0] == '<' || nextWord[0] == '&'){
                printf("Invalid use of redirection symbol: \"%s %s %s\"\n", cLine->words[i - 1], currentWord, nextWord);
                return 7;
            }
            else{
                cLine->pipeIndices[pipeCount] = i;
                pipeCount++;
                cLine->pipeIndices[pipeCount] = -1;

                cLine->argSets[argSetCount][argCount] = NULL;
                argSetCount++;
                argCount = 0;
				
				cLine->programSet[programCount] = cLine->words[i+1];
				programCount++;
				i++;
            }
        }
        else if(currentWord[0] == '&'){
            if(currentWord[1] != '\0'){
                printf("Invalid use of background execution symbol: %s. Please see the readme.", currentWord);
                return 7;
            }
			else if(i != cLine->numWords - 1){
				printf("The \"&\" symbol must be at the end of the command line.\n");
				return 8;
			}
            else{
                cLine->exeInBackground = 1;
            }
        }
        else{   //currentWord is an argument
            cLine->argSets[argSetCount][argCount] = currentWord;
            argCount++;
            cLine->argSets[argSetCount][argCount] = NULL;
        }
    }

    return 0;
}


/**
 * This function prints all of the variables inside the passed commandLine struct (cLine) by calling other print functions.
 */
void print_cline_struct(commandLine* cLine){
	printf("************************Printing command line structure************************\n");
    printf("User command line:\n%s\n", cLine->userLine);
    print_words(cLine);
	print_programs(cLine);
    print_args(cLine);
    print_special(cLine);
	printf("*******************************************************************************\n");
	
	return;
}


/**
 * This function prints the words array inside the passed commandLine struct (cLine).
 */
void print_words(commandLine* cLine){
    printf("Number of words: %d\n", cLine->numWords);
	
	for(int i = 0 ; i < cLine->numWords ; i++){
		printf("Word %d: %s\n", i, cLine->words[i]);

	}
	
    printf("\n");
	
	return;
}


/**
 * This function prints the programs array inside the passed commandLine struct (cLine).
 */
void print_programs(commandLine* cLine){
	printf("Number of programs: %d\n", cLine->numPrograms);
	
	for(int i = 0 ; i < cLine->numPrograms ; i++){
		printf("Program %d: %s\n", i, cLine->programSet[i]);

	}
	
	printf("\n");
	
	return;
}


/**
 * This function prints the args array inside the passed commandLine struct (cLine).
 */
void print_args(commandLine* cLine){
	
    for(int i = 0; i < cLine->numSets ; i++){
        printf("Argument set #%d:", i);

        if(cLine->argSets[i][0] == NULL)
            printf("There are no arguments in this set.");
        for(int j = 0 ; cLine->argSets[i][j] != NULL ; j++){
            printf("%s ", cLine->argSets[i][j]);
        }

        printf("\n");
    }
	
	printf("\n");
	
	return;
}


/**
 * This function prints the special variables inside the passed commandLine struct (cLine).
 * Special variables include: inputSymbIndex, inputRedirFile, outputSymbIndex, outputRedirFile,
 * pipeIndices, and exeInBackground.
 */
void print_special(commandLine* cLine){
    if(cLine->inputSymbIndex > 0){
        printf("The input shall be redirected to be from the file: %s\n", cLine->inputRedirFile);
        printf("This file string was found at index(of array cLine->words): %d\n", cLine->inputSymbIndex);
        printf("\n");
    }
    else{
        printf("There is no input redirection.\n");
    }

    if(cLine->outputSingleSymbIndex > 0){
        printf("The output shall be redirected to the file: %s\n", cLine->outputRedirFile);
        printf("This file string was found at index(of array cLine->words): %d\n", cLine->outputSingleSymbIndex);
        printf("\n");
    }
    else if(cLine->outputDoubleSymbIndex > 0){
        printf("The output shall be redirected to the file: %s\n", cLine->outputRedirFile);
        printf("This file string was found at index(of array cLine->words): %d\n", cLine->outputDoubleSymbIndex);
        printf("\n");
    }
    else{
        printf("There is no output redirection.\n");
    }

    if(cLine->numPipes > 0){
        printf("There are %d pipes.\n", cLine->numPipes);
        printf("The pipe indices(of array cLine->words) are: ");
        for(int i = 0; i < cLine->numPipes - 1; i++){
            printf("%d, ", cLine->pipeIndices[i]);
        }
		printf("%d", cLine->pipeIndices[cLine->numPipes - 1]);
        printf("\n\n");
    }
    else{
        printf("There are no pipes.\n");
    }

    if(cLine->exeInBackground){
        printf("This command should be executed in the background.\n");
        printf("\n");
    }
    else{
        printf("This command line will not be executed in the background.\n");
        printf("\n");
    }

	return;
}


/**
 * Frees all of the allocated space within an instance of a commandLine struct.
 *
 */
void free_command_line(commandLine* cLine){
    free(cLine->userLine);

    for(int i = 0; i < cLine->numWords ; i++){
        free(cLine->words[i]);
	}
	
	free(cLine->programSet);
    free(cLine->words);
    free(cLine->pipeIndices);

    for(int j = 0; cLine->argSets[j] != NULL ; j++){
        free(cLine->argSets[j]);
    }
    free(cLine->argSets);

    free(cLine);
	
	return;
}

