#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

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

struct node *nexus;

void fwalk(struct node *cmd, void *excl){
	int i;
	if(cmd==NULL) return;
	if(cmd->ptr!=excl){
		if(cmd->mode!=CMD){
			for(i=0;i<cmd->len;++i){
				fwalk(cmd->ptr[i], excl);
			}
		}
		free(cmd->ptr);
	}
	free(cmd);
}

int walk(struct node *cmd){
	char **argv;
	int i;
	int pid[cmd->len];
	if(cmd->mode==CMD){
		argv=(char **)cmd->ptr;
		fwalk(nexus, argv);
		return -execvp(argv[0], argv);
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
			waitpid(pid[i], NULL, 0);
			fwalk(cmd->ptr[i], NULL);
			cmd->ptr[i]=NULL;
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
			waitpid(pid[i], NULL, 0);
			fwalk(cmd->ptr[i], NULL);
			cmd->ptr[i]=NULL;
		}
		return 0;
	}
	return 1;
}

int main(int argc, char *argv[]){
	int i;
	char *end[]={NULL, "]]]", "]]"};
	void *p;
	nexus=malloc(sizeof(struct node));
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
		if(strcmp("[[", argv[i])==0){
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
		if(strcmp("[[[", argv[i])==0){
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
		if(strcmp(end[cmd->opcode], argv[i])==0){
#ifdef DEBUG
			printf("END\n");
#endif
			if(cmd->mode!=CMD && cmd->len==1){
#ifdef DEBUG
				printf("PROMOTE\n");
#endif
				leave=cmd->ptr[0];
				cmd->mode=leave->mode;
				free(cmd->ptr);
				cmd->ptr=leave->ptr;
				cmd->size=leave->size;
				cmd->len=leave->len;
				free(leave);
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
#ifdef DEBUG
	printf("FINALIZING\n");
#endif
	while(cmd){
		if(cmd->mode!=CMD && cmd->len==1){
#ifdef DEBUG
			printf("PROMOTE\n");
#endif
			leave=cmd->ptr[0];
			cmd->mode=leave->mode;
			free(cmd->ptr);
			cmd->ptr=leave->ptr;
			cmd->size=leave->size;
			cmd->len=leave->len;
			free(leave);
		}
		cmd=cmd->parent;
	}
	i=walk(nexus);
	fwalk(nexus, NULL);
	return i;
}
