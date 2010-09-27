#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define SEQ 0
#define PRL 1

int main(int argc, char *argv[]){
	char *end[]={"]]","]]]"};
	char *cmds[4096][4096]={0};
	int pid[4096];
	int mode,submode,i,sp=0,cmd=0,stk=0;

	if(strcmp("[[",argv[0])==0){
		submode=mode=SEQ;
	}else if(strcmp("[[[",argv[0])==0){
		submode=mode=PRL;
	}else{
		fprintf(stderr, "Unknown running mode.\n");
		return 1;
	}

	for(i=1;i<argc;++i){
		if(strcmp("[[",argv[i])==0){
			if(stk==0){
				submode=SEQ;
				++stk;
			}else if(submode==SEQ){
				++stk;
			}
			goto push;
		}
		if(strcmp("[[[",argv[i])==0){
			if(stk==0){
				submode=PRL;
				++stk;
			}else if(submode==PRL){
				++stk;
			}
			goto push;
		}
		if(strcmp(end[submode],argv[i])==0){
			--stk;
			switch(stk){
				case -1:
					goto run;
				case 0:
					submode=mode;
					cmds[cmd][sp]=argv[i];
					++cmd;
					sp=0;
					continue;
			}
		}
		push:
			cmds[cmd][sp]=argv[i];
			++sp;
			cmds[cmd][sp]=NULL;
	}
	run:
		if(cmds[cmd][0]) ++cmd;
		switch(mode){
			case SEQ:
				for(i=0;i<cmd;++i){
					pid[i]=fork();
					if(pid[i]<0){
						return 1;
					}
					if(pid[i]==0){
							return -execvp(cmds[i][0],cmds[i]);
					}
					waitpid(pid[i],NULL,0);
				}
				break;
			case PRL:
				for(i=0;i<cmd;++i){
					pid[i]=fork();
					if(pid[i]<0){
						return 1;
					}
					if(pid[i]==0){
							return -execvp(cmds[i][0],cmds[i]);
					}
				}
				for(i=0;i<cmd;++i){
					waitpid(pid[i],NULL,0);
				}
				break;
		}
	return 0;
}
