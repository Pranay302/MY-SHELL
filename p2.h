#include <stdio.h>
#include "getword.h"
#define MAXITEM 3000
char *newargsv[MAXITEM];
char *redirection_buffer[MAXITEM];
char *previous_newargsv[MAXITEM];
char *previous_redirection_buffer[MAXITEM];
int nextplace=0;
int startofwordindex=0;
int run_process_in_background=0; 

int pipe_creation_required=0;
int newargsv_index_for_child=0; //hold the index of newargsv for the command used by child

int input_ex_flag_for_comment=0;
int is_backslash_pipe=0; //used in getword to differenciate in | and \|
int prompt_count=1; //prompt_count
struct previous_commands //a struct to store essential flags and pointer arrays for history mechanism to work.
{
    char *history_newargsv[MAXITEM];
	char *history_redirection_buffer[MAXITEM];
	int history_pipe_required;
	int history_newargsv_index_for_child;
};
struct previous_commands commands[9];
