#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CMD 0
#define PRL 1
#define SEQ 2

#define INC_SIZE 8

struct node {
	int mode;
	int opcode;
	struct node *parent;
	void **ptr;
	int len;
	int size;
};

int walk(struct node *cmd){
	char **argv;
	int i;
	int pid[cmd->len];
	if(cmd->mode==CMD){
		argv=(char **)cmd->ptr;
		return -execvp(argv[0],argv);
	}
	if(cmd->mode==PRL){
		for(i=0;i<cmd->len;++i){
			pid[i]=fork();			
			if(pid[i]<0)
				return 1;
			if(pid[i]==0){
				return walk(cmd->ptr[i]);
			}
			
		}
		for(i=0;i<cmd->len;++i){
			waitpid(pid[i],NULL,0);
			free(cmd->ptr[i]);
		}
		return 0;
	}
	if(cmd->mode==SEQ){
		for(i=0;i<cmd->len;++i){
			pid[i]=fork();
			if(pid[i]<0)
				return 1;
			if(pid[i]==0){
				return walk(cmd->ptr[i]);
			}
			waitpid(pid[i],NULL,0);
			free(cmd->ptr[i]);
		}
		return 0;
	}
}

int main(int argc, char *argv[]){
	int i;
	char *end[]={NULL,"]]]","]]"};
	void *p;
	struct node *nexus=malloc(sizeof(struct node));
	struct node *cmd=nexus;
	struct node *leave;
	cmd->opcode=cmd->mode=SEQ;
	cmd->parent=NULL;
	cmd->ptr=NULL;
	cmd->size=0;
	cmd->len=0;
	
	for(i=0;i<argc;++i){
#ifdef DEBUG
		printf("[%d] %s: ", i, argv[i]);
#endif
		if(strcmp("[[",argv[i])==0){
#ifdef DEBUG
			printf("SEQ\n");
#endif
			if(cmd->mode==CMD){
				fprintf(stderr, "Unexpected sequentail operator at argv[%d]: %s.\n", i, argv[i]);
				return 1;
			}
			if(cmd->size<=cmd->len){
				cmd->size+=INC_SIZE;
				p=realloc(cmd->ptr, cmd->size * sizeof(void *));
				if(p!=cmd->ptr){
					free(cmd->ptr);
					cmd->ptr=p;
				}
			}
			leave=cmd->ptr[cmd->len]=malloc(sizeof(struct node));
			cmd->len+=1;
			leave->opcode=leave->mode=SEQ;
			leave->parent=cmd;
			leave->ptr=NULL;
			leave->size=0;
			leave->len=0;
			cmd=leave;

			continue;
		}
		if(strcmp("[[[",argv[i])==0){
#ifdef DEBUG
			printf("PRL\n");
#endif
			if(cmd->mode==CMD){
				fprintf(stderr, "Unexpected parallel operator at argv[%d]: %s.\n", i, argv[i]);
				return 1;
			}
			if(cmd->size<=cmd->len){
				cmd->size+=INC_SIZE;
				p=realloc(cmd->ptr, cmd->size * sizeof(void *));
				if(p!=cmd->ptr){
					free(cmd->ptr);
					cmd->ptr=p;
				}
			}
			leave=cmd->ptr[cmd->len]=malloc(sizeof(struct node));
			cmd->len+=1;
			leave->opcode=leave->mode=PRL;
			leave->parent=cmd;
			leave->ptr=NULL;
			leave->size=0;
			leave->len=0;
			cmd=leave;
			continue;
		}
		if(end[cmd->opcode] && strcmp(end[cmd->opcode],argv[i])==0){
#ifdef DEBUG
			printf("END\n");
#endif
			if(cmd->len==1){
				leave=cmd->ptr[1];
				cmd->mode=leave->mode;
				free(cmd->ptr);
				cmd->ptr=leave->ptr;
				cmd->size=leave->size;
				cmd->len=leave->len;
			}
			cmd=cmd->parent;
			continue;
		}
#ifdef DEBUG
		printf("CMD\n");
#endif
		if(cmd->len==0)
			cmd->mode=CMD;
		if(cmd->mode!=CMD){
			fprintf(stderr, "Unexpected command at argv[%d]: %s.\n", i, argv[i]);
			return 1;
		}
		if(cmd->size<=cmd->len+1){
			cmd->size+=INC_SIZE;
			p=realloc(cmd->ptr, cmd->size * sizeof(void *));
			if(p!=cmd->ptr){
				free(cmd->ptr);
				cmd->ptr=p;
			}
		}
		cmd->ptr[cmd->len]=argv[i];
		cmd->len+=1;
		cmd->ptr[cmd->len]=NULL;
	}
	i=walk(nexus);
	free(nexus);
	return i;
}
