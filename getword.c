/******

 * **Name: Pranay Mhatre
 * **Prof: John Carroll
 * **Course: CS570 
 * **Reference:  Foster & Foster's C by Discovery.
 * **This Program Simulates a basic lexical analyser it just recognizes words on the basis of spaces/ newline characters/ end of file signal. 
 * **It produces tokens as output, this series of tokens has data about the string encountered and the length of the string.
 * **This program is also able to recognize special characters and seperate out words on the basis of special characters
 * **Apart from specila characters this program handles a backslash in a special way. Any special character Appearing after a backslash is considered as a normal charac * **ter.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
extern int is_backslash_pipe;
/*The method isASpecialChar returns 1 if the input character 
 * it receives as a parameter is either one of '<' , '>' , '|' , '#' , '&' */
int isASpecialChar(char x){              //this method is called at several places in getword to find if the present char is a special character
    char listOfSpecialChar[5]={ '<' , '>' , '|' , '&' }; //remove '#' from this list for p4
    int i=0;
    while(listOfSpecialChar[i]){
        if (listOfSpecialChar[i]==x) return 1;
        i++;
    }
    return 0;

}
int getword(char *w){
    char collectedWord[255];
    int i=0;
    int prevCharWasABackslash=0;
    int gotDoubleGreaterThanAlready=0;
    int prevSpecialChar=0;
	int gotANormalWord=0;
    while ((*w=getchar())!=EOF){
		if((*w!=' ' && *w!='\n' && *w!='\\' && isASpecialChar(*w)!=1 ) || prevCharWasABackslash==1){
			if(i==254){
				ungetc(*w,stdin);   //this ungetc shoves back the 255th character into the input stream as it was read already and we need it for the next iteration
				*w='\0'; 
				return i>0?(int)strlen(collectedWord):0; 
			}
			if(*w!='\n'){
				if(prevSpecialChar==1){
					ungetc(*w,stdin);   //this ungetc will put back the newline char. as we used it up to recognize the end of word but we need it in the next iteration to get n=0 s=[]
					break;
				}
				if(*w=='|' && prevCharWasABackslash==1){
					is_backslash_pipe=1; ///use this global variable in p2 to differenciate in | and \|
				}else if(*w=='|' && prevCharWasABackslash==0){
					is_backslash_pipe=0;
				}
				
				collectedWord[i]=*w;
				i++;
				gotANormalWord=1; 
			}else{      // this block is excuted when we encounter a new line character right after a backslash character
				ungetc('\n',stdin);
				prevCharWasABackslash=0;
				break;
			}
			if(prevCharWasABackslash==1){
				prevCharWasABackslash=0;
			} 
		
		}else if(*w==' ' && i==0){ 
			continue;      // this block skips any unnecessary spaces.             
		}else {
			if(*w=='\n' && i!=0){ 
				ungetc('\n',stdin);     
			}else if(*w=='\\'){
				prevCharWasABackslash=1;  //this token 'prevCharWasABackslash' is used while reading the next character as it needs to be handled specially
				continue;
			}else if(isASpecialChar(*w)&& gotANormalWord!=0 ){
				ungetc(*w,stdin);
			}else if(isASpecialChar(*w) && *w!='>' && gotDoubleGreaterThanAlready!=1){
				collectedWord[i]=*w; //this block gets executed for any special character except the greater than symbol
				
				i++;
				w++;
				break;
			}else if(isASpecialChar(*w)){ 
					collectedWord[i]=*w;       //this block recognizes the special strings like >>& and >> 
					i++;
					collectedWord[i]='\0';
					if(strcmp(collectedWord,">>")==0){
						gotDoubleGreaterThanAlready=1;
					}else if(gotDoubleGreaterThanAlready){ //the token gotDoubleGreaterThanAlready becomes 1 when we have already encountered greater than symbol twice
						if(*w =='&'){  //checks for >>& 
							w++;
							break;
						}else{
							collectedWord[i-1]='\0';
							ungetc(*w,stdin);
							break;
						}
					}else if(strcmp(collectedWord,">&")==0){
						w++;
						break;
					}else {
						prevSpecialChar=1; 
					}
					w++;
					continue;
				}                       
			*w='\0';    
			collectedWord[i]='\0';
			if(strcmp(collectedWord,"done")==0) return -1; 
			return i>0?(int)strlen(collectedWord):0; //
		}
        w++;
    }
    *w='\0';            
    collectedWord[i]='\0';
    if(strcmp(collectedWord,"done")==0) return -1;
    return i==0?-1:(int)strlen(collectedWord); 
}

