/*
 * Copyright (c) 2010 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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

void fwalk(struct node *cmd){
	int i;
	if(cmd==NULL) return;
	if(cmd->mode!=CMD){
		for(i=0;i<cmd->len;++i){
			fwalk(cmd->ptr[i]);
		}
	}
	free(cmd->ptr);
	free(cmd);
}

int walk(struct node *cmd){
	int i;
	int pid[cmd->len];
	if(cmd->mode==CMD){
		char *argv[cmd->len+1];
		for(i=0;i<=cmd->len;++i){
			argv[i]=cmd->ptr[i];
		}
		fwalk(nexus);
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
			fwalk(cmd->ptr[i]);
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
			fwalk(cmd->ptr[i]);
			cmd->ptr[i]=NULL;
		}
		return 0;
	}
	return 1;
}

int main(int argc, char *argv[]){
	int i;
	char *end[]={NULL, "]]]", "]]"};
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
				cmd->ptr=realloc(cmd->ptr, cmd->size * sizeof(void *));
				//XXX if(cmd->ptr==NULL) ...
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
				cmd->ptr=realloc(cmd->ptr, cmd->size * sizeof(void *));
				//XXX if(cmd->ptr==NULL) ...
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
			cmd->ptr=realloc(cmd->ptr, cmd->size * sizeof(void *));
			//XXX if(cmd->ptr==NULL) ...
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
	fwalk(nexus);
	return i;
}
