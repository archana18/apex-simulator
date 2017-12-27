#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>

char local_predict(int N, int local_history[N], char program_address, char actual_direction);
char global_predict(int N, int global_pattern_history[N], char actual_direction);
char selector_predict(int N, int selector_history[N], char program_address, char global_prediction, char local_prediction, char actual_direction);

/* CHANGE TO STANDARD IN, STANDARD OUT! */

FILE *input_file;
char *input;

FILE *output_file;
char *output; 

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

int ADDRESS_RANGE = 10;

int GLOBAL_HISTORY_MAX = 64;
int global_history = 0;

int main(int argc, char *argv[]){
     if(argc != 1){
        //check for correct program usage
        printf("usage: p1 < input_filename > output_filename >> statistics\n");
        exit(1);
    }

    input_file = stdin;
    output_file = stdout;  

    off_t input_filesize;

    if(fseeko(input_file, 0L, SEEK_END) == -1){
        perror("<fseek() failed unexpectedly");
        exit(1);
    }

    input_filesize = ftello(input_file);
    if(input_filesize == -1){
        perror("<ftello() failed unexpectedly>");
        exit(1);
    }

    if(fseeko(input_file, 0L, SEEK_SET) == -1){
        perror("<fseek() failed unexpectedly>");
        exit(1);
    };
   
    //allocate space for input
    input = (char*)malloc(input_filesize*sizeof(char)+1);
    if(input == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }

    off_t OUTPUT_LINESIZE = 6;

    //allocate space for output
    output = (char*)malloc(OUTPUT_LINESIZE*sizeof(char)+1);
    if(output == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }
    //read-in input
    int nread;

    nread = fread(input, sizeof(char), input_filesize, input_file);
    if(nread != input_filesize){
        perror("<fread() failed unexpectedly>");
        exit(1);
    }       
    input[input_filesize]='\0';

    int index;

    int local_history[ADDRESS_RANGE]; 
    for(index = 0; index < ADDRESS_RANGE; index++){
        local_history[index] = 0;
    } 

    int global_pattern_history[GLOBAL_HISTORY_MAX];
    for(index = 0; index < GLOBAL_HISTORY_MAX; index++){
        global_pattern_history[index] = 0;
    } 

    int selector_history[ADDRESS_RANGE];    
    for(index = 0; index < ADDRESS_RANGE; index++){
        selector_history[index] = 0;
    }      

    char program_address;
 
    char local_prediction;
    char global_prediction;
    char selector_prediction;

    char final_prediction;
    char actual_direction;
 
    char * parse;
    char * token; 

    token = strtok_r(input, "\n", &parse);
    program_address = token[0] - '0';
    actual_direction = token[1];

    local_prediction = local_predict(ADDRESS_RANGE, local_history, program_address, actual_direction);
    global_prediction = global_predict(GLOBAL_HISTORY_MAX, global_pattern_history, actual_direction);
    selector_prediction = selector_predict(ADDRESS_RANGE, selector_history, program_address, global_prediction, local_prediction, actual_direction);

    if(selector_prediction == 'l'){
        final_prediction = local_prediction;
    }
    else if(selector_prediction == 'g'){
        final_prediction = global_prediction; 
    }

    output[0] = program_address + '0';
    output[1] = local_prediction;
    output[2] = global_prediction;
    output[3] = selector_prediction;
    output[4] = final_prediction;
    output[5] = actual_direction;
    output[6] = '\0';
    fprintf(output_file, "%s\n", output);
            
    while(token = strtok_r(NULL, "\n", &parse)){
        program_address = token[0] - '0';
        actual_direction = token[1];
      
        local_prediction = local_predict(ADDRESS_RANGE, local_history, program_address, actual_direction);

        global_prediction = global_predict(GLOBAL_HISTORY_MAX, global_pattern_history, actual_direction);
            
        selector_prediction = selector_predict(ADDRESS_RANGE, selector_history, program_address, global_prediction, local_prediction, actual_direction);
              
        if(selector_prediction == 'l'){
            final_prediction = local_prediction;
        }
        else if(selector_prediction == 'g'){
            final_prediction = global_prediction; 
        }
   
        output[0] = program_address + '0';
        output[1] = local_prediction;
        output[2] = global_prediction;
        output[3] = selector_prediction;
        output[4] = final_prediction;
        output[5] = actual_direction;
        output[6] = '\0';
        fprintf(output_file, "%s\n", output);
    }      
  
    //cleanup  
    free(input);
    free(output);
    fclose(output_file);
    fclose(input_file);
    return 0;
}

char local_predict(int N, int local_history[N], char program_address, char actual_direction){
    char local_prediction;

    int counter = local_history[program_address];
    if(counter <= 1){
        local_prediction = 'n';
    }
    else if(counter >= 2){
        local_prediction = 't';
    }

    if((actual_direction == 'n') && (counter != 0)){
        local_history[program_address] = counter-1;
    }
    else if((actual_direction == 't')  && (counter != 3)){
        local_history[program_address] = counter+1;
    }
    return local_prediction;
}

char global_predict(int N, int global_pattern_history[N], char actual_direction){
    char global_prediction;

    int counter = global_pattern_history[global_history];
    if(counter <= 1){
        global_prediction = 'n';
    }
    else if(counter >= 2){
        global_prediction = 't';
    }

    if((actual_direction == 'n') && (counter != 0)){
        global_pattern_history[global_history] = counter-1;
    }
    else if((actual_direction == 't')  && (counter != 3)){
        global_pattern_history[global_history] = counter+1;
    }
    if(actual_direction == 'n'){
        global_history = ((global_history*2)%GLOBAL_HISTORY_MAX) +  0;
    }
    else if(actual_direction == 't'){
        global_history = ((global_history*2)%GLOBAL_HISTORY_MAX) +  1;
    }
    return global_prediction;
}

char selector_predict(int N, int selector_history[N], char program_address, char global_prediction, char local_prediction, char actual_direction){
    char selector_prediction;

    int counter = selector_history[program_address];
    if(counter <= 1){
        selector_prediction = 'l';
    }
    else if(counter >= 2){
        selector_prediction = 'g';
    }

    if((local_prediction == actual_direction) && (global_prediction != actual_direction)){
        selector_history[program_address] = counter-1;
    }
    if((local_prediction != actual_direction) && (global_prediction == actual_direction)){
        selector_history[program_address] = counter+1;
    }
    return selector_prediction;
}
