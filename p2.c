/******
 ** **Name: Pranay Mhatre
 ** **Prof: John Carroll
 ** **Course: cs570
 ** **Reference: Foster & Foster's C by Discovery.
 ** **p2.c emulates a shell. It is able to handle shell commands, input/output redirection, errors handling like ambiguous redirection input.etc. 
 ** **p2.c spawns a child process that does a execvp to run the input executable commands. except for commands like cd(there is no child created)
 ** **p2.c is able to re execute the last executed command when the input is "!!"
 ** **p2.c ususally waits for the child process to complete its tasks but if the last character is a "&" parent process doesnot wait for the child.
 ** **p2. internally calls parse(char *input) and check_for_builtins().
 * * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "p2.h"
/******
** **parse takes a character pointer pointing to the input stream. It internally calls the getword function to get the length of the input word.
** **It stores the input stream on a big charcter array (inputbuffer) and creates 2 array of character pointers;
** **1> newargsv -> holds only the ececutable command.
** **2> redirection_buffer -> holds data about the type of redirection (inp/oup) and the file names.
** **errors are thrown when there is a ambiguous redirection command.
for p4:
parse does the following additional tasks:
detects a | in the input and used a flag "is_backslash_pipe"(this flag is set in getword if we get \|).
it also does the redirection checks i.e. check if the file is present before appending to it.
it also errors out ambiguous inputs.
** */


int parse(char *input){
	int i=0,j=0;
	int encountered_a_special=0;
	int k=0,existance=0,missing_redirect=0,is_comment=0;
	int greater_than_count=0,greater_than_amp_count=0,less_than_count=0,ambiguity=0,found_amp=0;
	char inputbuffer[3000];
	for(;;) {
		int length=getword(input); //getword returns length of the string and its value is in the character stream (input)
		if((input_ex_flag_for_comment==1 && strcmp(input,"#")==0) || is_comment==1){ //this block handles the comments in a input argument file.
//			printf("\ninput_ex_flag_for_comment is %d input is %s this is inside the block\n",input_ex_flag_for_comment,input);	
			if(length==0){
				is_comment=0;
			}
			else {
				is_comment=1;	
				continue;
			}	
		}
		if(length==0 || ambiguity==1){ 
			if(existance==1){
				existance=0; 
				return 3;
			}
			if(ambiguity==1){
				return 3;   //this status indicates ambiguous input redirection commands
			}
			if(missing_redirect==1){
				newargsv[0]='\0';
                redirection_buffer[0]='\0';
				return 5;
			}		
			if(found_amp==1){ //"&" is the last char.
				run_process_in_background=1;
				newargsv[--i]='\0'; //remove the "&" from the newargsv
				break;
			}
			if(i>0){
				newargsv[i]='\0';  //point the last of newargsv to null.
				redirection_buffer[k]='\0';	
			}
			else if (i==0){
				return 5;  //return 5 basically continues the infinite for loop in main/
			}
			break; 
		}else if (length==-1){
			if(strcmp(input,"done")==0){
				if (newargsv[0]=='\0'){
					printf("\nEncountered a done with no other word in a line.\n");
					return -1;
				}else if(i==0){
					return -1;
				}else{
					length=4;//if 'done' was not the first word then make the length=4 and consider it as a normal word;
				}	
			}
			else if(i>0){//this block handles the EOF adjecent to the last word
				if(found_amp==1){
					run_process_in_background=1;//enters this block only when the last char was a &;
					newargsv[--i]='\0';
					return 4;
				}
		                newargsv[i]='\0';
				return 4;
			}
			if(i==0){	
				return -1;//return -1 from parse brakes the infinite for loop from main.	
			}
		}else if(length==1 && strcmp(input,"&")==0){
			found_amp=1;
		}else {
			found_amp=0;
		}
		for( j=0;j<strlen(input);j++){
			inputbuffer[nextplace]=input[j];//the entire input is stored in this big input buffer;
			nextplace++;
		}
	        inputbuffer[nextplace]='\0';
		nextplace++;
		if(strcmp(input,">")!=0 && strcmp(input,">&")!=0 && strcmp(input,"<")!=0 && encountered_a_special!=1 && strcmp(input,">>")!=0 && strcmp(input,">>&")!=0){ 
			if((strcmp(input,"|")==0) && is_backslash_pipe!=1){ 
				pipe_creation_required=1;
				newargsv[i]='\0';
				i++;
				newargsv_index_for_child=i;// this index of newargsv is used by child to get it's command. 
			}else{
				newargsv[i]=&inputbuffer[startofwordindex];
				i++;
				if(is_backslash_pipe==1){
					is_backslash_pipe=0;
				}
			}
			
		}else{
			
			if(strcmp(input,"&")!=0){
				redirection_buffer[k]=&inputbuffer[startofwordindex];//this pointer array stores (<,>,>&)address and the address of the next word to help in redirection;
				k++;
			}else if(strcmp(input,"&")==0){
		        printf("\nMissing file name for redirection.\n");
				missing_redirect=1;	
		       	startofwordindex+=(length+1);	
				continue; 
			}	
			if(strcmp(input,">")==0 || strcmp(input,">&")==0 || strcmp(input,"<")==0 || strcmp(input,">>")==0 || strcmp(input,">>&")==0){
		        encountered_a_special=1;//this block keeps a check on the number of <,>,>&,>>,>>& to report ambiguity;
				if(strcmp(input,">")==0) greater_than_count++; 
		        else if(strcmp(input,">&")==0) greater_than_amp_count++;  
		        else if(strcmp(input,"<")==0) less_than_count++; 
				else if(strcmp(input,">>")==0) greater_than_count++;
				else if(strcmp(input,">>&")==0) greater_than_count++;
			}else{
				if(greater_than_count>1 || greater_than_amp_count > 1|| (greater_than_count+greater_than_amp_count)>1){	 
					printf("\nAmbiguous output redirect.\n");
					ambiguity=1;
				}else if(less_than_count>1){ 
					printf("\nAmbiguous input redirect.\n");  
					ambiguity=1;
				}
				if(strcmp(redirection_buffer[k-2],">")==0 || strcmp(redirection_buffer[k-2],">&")==0){
					if(strcmp(redirection_buffer[k-1],"!$")!=0){
						if(access(redirection_buffer[k-1],F_OK)!=-1){
							perror("\nFile already exists:");
							newargsv[0]='\0';
							redirection_buffer[0]='\0';
							existance=1;
						}
					}
				}else if(*redirection_buffer[k-2]=='<' || strcmp(redirection_buffer[k-2],">>")==0 || strcmp(redirection_buffer[k-2],">>&")==0){
					if(strcmp(redirection_buffer[k-1],"!$")!=0){
						if(access(redirection_buffer[k-1],F_OK)==-1){
							perror("\nFile doesnot exist:");
							newargsv[0]='\0';
							redirection_buffer[0]='\0';
							existance=1;
						}
					}
				}
				encountered_a_special=0;
			}
		}
		startofwordindex+=(length+1);
	}
	return 0; 
}
/*******
 * * **check_for_builtins is called from the main method after the input stream is parsed and the newargsv and redirection_buffer are filled with their data.
 * * **it backs up data from the input stream so that it can be used later if the next command is a !!.
 * * **change directory functionality is also implemented in this function.
 for p4:
 this function does additional job of storing the historic commands inside the struct.
 it modifies the newargsv and redirection_buffer buffer if we have !3.. or !! as the input.
 it also handles !$ input.
 * */

int check_for_builtins(){
	int i=0,j=0,char_index=0,ex_dollar=0;
	char ex=newargsv[j][char_index];
	int flag=0;
	while(newargsv[j]!='\0'){ //this block identifies the position of !$ in newargsv and replaces the !$ with the last word of previous command.
		if (strcmp(newargsv[j],"!$")==0){
			flag=1;
			int replace_pos=j;
			j=0;
			while(previous_newargsv[j]!='\0'){
				j++;
			}
			newargsv[replace_pos]=previous_newargsv[j-1];
		}
		j++;
	}
	j=0;
	while(redirection_buffer[j]!='\0'){ //this block identifies the position of !$ in redirection_buffer and replaces it with the last word in "newargsv".
		if (strcmp(redirection_buffer[j],"!$")==0){
			int replace_pos=j;
			j=0;
			while(previous_newargsv[j]!='\0'){
				j++;
			}
			redirection_buffer[replace_pos]=previous_newargsv[j-1];
		}
		j++;
	}
	j=0;
	if (ex=='!' && strcmp(newargsv[j],"!!")!=0 && flag!=1){ //enter this block only when the command is !n n-->{1 to 9}
		char_index++;
		char nu=newargsv[j][char_index];
		int number=nu -'0';
		int requested_command= number;
		if((number<10 && number!=0) && number < prompt_count && nu !='!'){
			char_index++;
			if(newargsv[j][char_index]=='\0'){
				//retrive the content of the required command's newargsv and redirection_buffer also retrive flags like "pipe_creation_required".
				while(commands[requested_command-1].history_newargsv[j]!='\0'){    
					newargsv[j]=commands[requested_command-1].history_newargsv[j];
					commands[prompt_count-1].history_newargsv[j]=newargsv[j];
					
					j++;
				}
				newargsv[j]='\0';
				commands[prompt_count-1].history_newargsv[j]='\0';
				j=0;
				while(commands[requested_command-1].history_redirection_buffer[j]!='\0'){
					redirection_buffer[j]=commands[requested_command-1].history_redirection_buffer[j];
					commands[prompt_count-1].history_redirection_buffer[j]=redirection_buffer[j];
					j++;
				}
				redirection_buffer[j]='\0';
				commands[prompt_count-1].history_redirection_buffer[j]='\0';
				pipe_creation_required=commands[requested_command-1].history_pipe_required;
				newargsv_index_for_child=commands[requested_command-1].history_newargsv_index_for_child;
			}
			
		}else if((number > prompt_count)||(number>9 || number<0)){
			perror("\nCannot fetch command number which is not executed.");
		}
	}
	j=0;
	if(strcmp(newargsv[j],"!!")!=0){//whenever the command in not !! it enters this block to keep a backup of the command executed
		while(newargsv[j]!='\0'){
			previous_newargsv[j]=newargsv[j];
			commands[prompt_count-1].history_newargsv[j]=newargsv[j];
			j++;
		}
		previous_newargsv[j]='\0';
		commands[prompt_count-1].history_newargsv[j]='\0';
		j++;
		if(pipe_creation_required==1){
			while(newargsv[j]!='\0'){
				previous_newargsv[j]=newargsv[j];
				commands[prompt_count-1].history_newargsv[j]=newargsv[j];
				j++;
			}
		}
		j=0;
		while(redirection_buffer[j]!='\0'){
			previous_redirection_buffer[j]=redirection_buffer[j];
			commands[prompt_count-1].history_redirection_buffer[j]=redirection_buffer[j];
			j++;
		}
		previous_redirection_buffer[j]='\0';
		commands[prompt_count-1].history_redirection_buffer[j]='\0';
		if(pipe_creation_required==1){
			commands[prompt_count-1].history_pipe_required=pipe_creation_required;
			commands[prompt_count-1].history_newargsv_index_for_child=newargsv_index_for_child;
		}
	}else if(strcmp(newargsv[j],"!!")==0){//if the input is !! enter this block and retrive the backup of previous commands. 
		while(previous_newargsv[j]!='\0'){
			newargsv[j]=previous_newargsv[j];
			commands[prompt_count-1].history_newargsv[j]=newargsv[j];
			j++;
		}
		newargsv[j]='\0';
		commands[prompt_count-1].history_newargsv[j]='\0'; 
		j++;
		while(previous_newargsv[j]!='\0'){
			newargsv[j]=previous_newargsv[j];
			commands[prompt_count-1].history_newargsv[j]=newargsv[j];
			j++;
		}
		j=0;
		while(previous_redirection_buffer[j]!='\0'){
			redirection_buffer[j]=previous_redirection_buffer[j];
			commands[prompt_count-1].history_redirection_buffer[j]=redirection_buffer[j];
			j++;
		}
		redirection_buffer[j]='\0';
		commands[prompt_count-1].history_redirection_buffer[j]='\0';
		pipe_creation_required=commands[prompt_count-2].history_pipe_required;
		newargsv_index_for_child=commands[prompt_count-2].history_newargsv_index_for_child;
		if(pipe_creation_required==1){
			commands[prompt_count-1].history_pipe_required=pipe_creation_required;
			commands[prompt_count-1].history_newargsv_index_for_child=newargsv_index_for_child;
		}
		
	}
	if(strcmp(newargsv[i],"cd")==0 ){ //handle cd
        if(newargsv[i+2]!='\0') perror("\nError: cannot cd to 2 different dir\n"); 
        else if(newargsv[i+1]!='\0'){
			if(chdir(newargsv[i+1]) != 0) perror("\nError : chdir() failed\n");
		}else{
			if(chdir(getenv("HOME")) != 0)  perror("\nError : chdir() failed\n");
		}
		prompt_count++;
		return 0;//return 0 will continue the for loop in main as the command for this execution was 'cd' adn its already handled here so no need to create a child process.
	} 
	return 1;
}
/******
 * * **This function is invoked by the signal command.
 * */
void signal_handler(int num){
	
}
/******
 * * **The main function can take in the arguments passed directly to p2 and redirect input to be read from the file specified. 
 * * **It keeps on running an infinite for loop which internally does the following:
 * * **Parse the input ->call the check_for_builtins method -> do the necessery setup required for redirection->
 * * **fork a child process->inside child do required redirection to stdin/stdout and execute the command stored in the newargsv pointer array.
 * * **outside the child, the parent waits for the child to complete(unless the command ends with an &) and finally it goes ahed to kill the processgroup.
 for p4:
 this method is now capable of handling pipe.
 it creates a child which internally creates a grandchild.
 the grandchild reads from the input and writes its output on to the write end of the pipe.
 the child reads data from the read end of the pipe and writes the final output to the desired output file descriptor.
 * */
int main(int argc, char *argv[])
{
	int parse_status=0;
	int statusfromfork=0;
	int do_we_redirect_to_out=0,redirect_to_out=0,append_with_err=0,append=0;
	int do_we_redirect_to_in=0,redirect_to_in=0;
	int file_dis_out ,file_dis_in,redirect_error,handled_builtins,ret,fd;
	int file_discriptor_cmd,adjecent_EOF=0;
	char outfile[255];
	//int prompt_count=1; //prompt_count
	//char PROMPT[]="%1% ";
	char s[255];
	char infile[255];
	char outfile_name[255];
	char infile_name[255];	
	char input_executeble[255]; 
	pid_t pid;	
	setpgid(0,0); 
	signal(SIGTERM,signal_handler);
	if(argc>2){
		printf("\nError");
	}else if(argc==2){  //this block is to handle arguments that are passed to p2.
//		printf("\ninside the arcg==2 block\n");	
		input_ex_flag_for_comment=1;
		strcpy(input_executeble,argv[1]);  //input_executeble is the char array of size 255 which holds the name of input executable file.
		file_discriptor_cmd=open(input_executeble,O_RDONLY); 
        if (file_discriptor_cmd==-1) perror("\nError");
        ret=dup2(file_discriptor_cmd, STDIN_FILENO); //p2 will redirect the input from the file_discriptor STDIN_FILENO 
	    if(ret < 0) perror("\nError : ");
		close(file_discriptor_cmd);
	}
	for(;;) {
        int l=0; 
		int j=0;
		if(argc != 2){
			printf("%%%d%% ",prompt_count); //prompt_count
		}
        
        parse_status=parse(s); //internally calls the getword function; creates an array of pointers :newargsv and redirection_buffer
		if(parse_status==-1 || adjecent_EOF==1){	//**parse_status value : meaning
			adjecent_EOF=0;				//**3:ambiguous redirect command:
			break;					//**4:encountered a EOF right after the last word.
        }else if(parse_status==3 || parse_status==5){  	//**5:encountered anewline char with no other word in the input.
			if(parse_status==3){
				prompt_count++;
				continue;
			}else{
				continue;	
			}
						//**-1:Got a EOF and its time to break the for loop:
	    }else if(parse_status==4){
			adjecent_EOF=1;
		}
		handled_builtins=check_for_builtins();
	    if(handled_builtins==0){ 
			run_process_in_background=0;	
			continue;
        }
	    while(redirection_buffer[j]!='\0'){//this block will recognize the type of redirection and set te appropriate input/output file names. 
			if(*redirection_buffer[j]=='>' || strcmp(redirection_buffer[j],">&")==0 ||do_we_redirect_to_out==1 || strcmp(redirection_buffer[j],">>&")==0 || strcmp(redirection_buffer[j],">>")==0){
					if(do_we_redirect_to_out==1){
						strcpy(outfile , redirection_buffer[j]);
						outfile[strlen(redirection_buffer[j])+1]='\0';	
						l=0;
						while(outfile[l]!='\0'){
							outfile_name[l]=outfile[l];
							l++;
						}
						outfile_name[l]='\0';
						do_we_redirect_to_out=0;
					}else if(*redirection_buffer[j]=='>' || strcmp(redirection_buffer[j],">&")==0 || strcmp(redirection_buffer[j],">>&")==0 || strcmp(redirection_buffer[j],">>")==0){
						do_we_redirect_to_out=1;
						redirect_to_out=1;
						redirect_error=strcmp(redirection_buffer[j],">&")==0?1:0;
						append=strcmp(redirection_buffer[j],">>")==0?1:0;
						append_with_err=strcmp(redirection_buffer[j],">>&")==0?1:0;
					}
				}else if(*redirection_buffer[j]=='<' || do_we_redirect_to_in==1){
					if(do_we_redirect_to_in==1){
						strcpy(infile , redirection_buffer[j]);
						infile[strlen(redirection_buffer[j])+1]='\0';	
						l=0;
						while(infile[l]!='\0'){
							infile_name[l]=infile[l];
							l++;
						}	
						infile_name[l]='\0'; 
						do_we_redirect_to_in=0;
					}else if(*redirection_buffer[j]=='<'){
						do_we_redirect_to_in=1;
						redirect_to_in=1;
					}
				}
			j++;
		}
		fflush(stdout);	
		statusfromfork=fork(); 
	
		if (statusfromfork == 0){ //child will execute this block.
		//use pipe_creation_required to detect if there is a need to create pipe.
			if(pipe_creation_required==0){
				if(redirect_to_in==0 && run_process_in_background==1){ //if there is no reading from input redirect the input FD to /dev/null
							fd = open("/dev/null", O_RDONLY);
							ret=dup2(fd,STDIN_FILENO);
							if(ret < 0) perror("\nError in dup2");
							
				}
				if(redirect_to_in==1){ //input redirection block
							file_dis_in=open(infile_name,O_RDONLY);   
							if(file_dis_in==-1) perror("\nError in open ");
							ret=dup2(file_dis_in, STDIN_FILENO);
							if(ret < 0)perror("\nError in dup2");
							close(file_dis_in);
				}
				if(redirect_to_out==1){ // output redirection block
					
					if (append==1 || append_with_err==1){
						file_dis_out=open(outfile_name,O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR); 
					}else{
						file_dis_out=open(outfile_name,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
					}
					if(file_dis_out==-1) perror("\nError in open");
					ret=dup2(file_dis_out, STDOUT_FILENO);
					if(ret < 0) perror("\nError in dup2 for normal output redirection");
					if(redirect_error==1 || append_with_err==1){
						ret=dup2(STDOUT_FILENO,STDERR_FILENO);//this will redirect the stderr to the stdout;
						if(ret < 0) perror("\nError in dup2 for error redirection");
					}
					close(file_dis_out);
				}
				execvp(newargsv[0],newargsv);
				perror("\nError in execvp");
				exit(0);
			}else{
				int filedes[2];
				pid_t grandchild_pid;
				pipe(filedes);
				grandchild_pid=fork();
				if(grandchild_pid==0){ //grandchild's code
					ret=dup2(filedes[1], STDOUT_FILENO); //grand child says i will write to the write end of the pipe.
					if(ret < 0)perror("\nError in dup2");
					close(filedes[0]);
					close(filedes[1]);
					if(redirect_to_in==1){
						file_dis_in=open(infile_name,O_RDONLY);   
						if(file_dis_in==-1) perror("\nError in open ");
						ret=dup2(file_dis_in, STDIN_FILENO);
						if(ret < 0)perror("\nError in dup2");
						close(file_dis_in);
					}
					execvp(newargsv[0],newargsv);
					perror("\nError in execvp -- grandchild");
					exit(0);
				}else if(grandchild_pid>0){//child's code
					ret=dup2(filedes[0], STDIN_FILENO); //child says i will be taking input from the read end of the pipe.
					if(ret < 0)perror("\nError in dup2");
					close(filedes[0]);
					close(filedes[1]);
					if(redirect_to_out==1){ // output redirection block
						if (append==1 || append_with_err==1){
								file_dis_out=open(outfile_name,O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
						}else{
								file_dis_out=open(outfile_name,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
						}
						if(file_dis_out==-1) perror("\nError in open");
						ret=dup2(file_dis_out, STDOUT_FILENO);
						if(ret < 0) perror("\nError in dup2");
						if(redirect_error==1 || append_with_err==1){
							ret=dup2(STDOUT_FILENO,STDERR_FILENO);//this will redirect the stderr to the stdout;
							if(ret < 0) perror("\nError in dup2");
						}
						close(file_dis_out);
					}
					execvp(newargsv[newargsv_index_for_child],newargsv+newargsv_index_for_child); //this is not '0' but the index of the newargsv array for child's use. as the newargsv has data for both child and grandchild.
					perror("\nError in execvp");
					exit(0);
				}
				
			}
		}else if(statusfromfork > 0){ //parent will execute this block.
			if(run_process_in_background==1){
                		printf("\n%s [%d]\n",newargsv[0],statusfromfork);
            		}else{
                		//while((pid=wait(NULL))>0);//wait for the child to complete its task.
						for(;;){
							pid_t pid;
							pid=wait(NULL);
							if(pid == statusfromfork){
								break;
							}
						}
            		}
            		redirection_buffer[0]='\0';
			redirect_to_in=0;
			redirect_to_out=0; 
			run_process_in_background=0;
			pipe_creation_required=0;
			append=0;
			append_with_err=0;
			
		}else if(statusfromfork==-1){ //error with fork.
			printf("\n there was some problem in fork");
        }
			prompt_count++;   
    	}
	killpg(getpgrp(),SIGTERM);//sends a signal to the signal handler to kill the process group;
	if(argc==1){
		printf("p2 terminated.\n");
	}
	
	exit(0);
}

