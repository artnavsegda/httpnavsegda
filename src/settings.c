#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAXOPT 1000

struct settings {
        char *options[MAXOPT];
        char *values[MAXOPT];
        int optisize;
};

void setopt(struct settings *myset, char *parameter, char *newset)
{
	int found = 0;
	for (int i=0;i<myset->optisize;i++)
	{
		if (strcmp(myset->options[i],parameter)==0)
		{
			if (strlen(newset)>strlen(myset->values[i]))
			{
				found = 1;
				//values[i] = newset; //just change the pointer
                                free(myset->values[i]);
				myset->values[i] = malloc(strlen(newset)+1); //or to allocate new memory ?
				strcpy(myset->values[i],newset);
			}
			else
				strcpy(myset->values[i],newset);
		}
	}
	if (found == 0)
	{
		myset->options[myset->optisize] = malloc(strlen(parameter)+1);
                strcpy(parameter,myset->options[myset->optisize]);
		myset->values[myset->optisize] = malloc(strlen(newset)+1);
                strcpy(newset,myset->values[myset->optisize]);
		myset->optisize++;
	}
}

void drop(int dropstatus, char *dropdesc)
{
        if (dropstatus == -1)
        {
                perror(dropdesc);
                exit(1);
        }
}

void drop2(char *dropstatus, char *dropdesc)
{
        if (dropstatus == NULL)
        {
                perror(dropdesc);
                exit(1);
        }
}

long long filesize(int fd)
{
	struct stat filestat;
	drop(fstat(fd,&filestat),"file status error");
	return filestat.st_size;
}

void feedopt(char *str)
{
        char *options[MAXOPT];
        char *values[MAXOPT];
        int i = 0;
        options[i] = strtok(str," \n");
        while (options[i]!=NULL)
                options[++i] = strtok(NULL," \n");
        int optisize = i;
        for (i=0;i<optisize;i++)
        {
                strtok(options[i],"=");
                values[i] = strtok(NULL,"=");
        }
        for (i=0;i<optisize;i++)
                setopt(NULL,options[i],values[i]);
}

void feedfile(char *filename)
{
        FILE * setfile = fopen(filename,"r");
        drop2(setfile,"file open error");
        char *str = malloc(filesize(setfile)+1);
        int numread = fread(str,1,filesize(setfile),setfile);
        drop(numread,"file read error");
        str[numread] = '\0';
        fclose(setfile);
        feedopt(str);
        free(str);
}

void evalueateopt(char *evaluation)
{
        char *parname;
	char *parvalue;
	parname = strtok(evaluation,"=");
	parvalue = strtok(NULL,"=");
	setopt(NULL,parname,parvalue);
}

void formdecode(char *formdata)
{
        char *dispatch[MAXOPT];
        int i = 0;
        dispatch[i] = strtok(formdata,"&");
        while(dispatch[i] != NULL)
                dispatch[++i] = strtok(NULL, "&");
        int amount = i;
        for (i=0;i<amount;i++)
                evalueateopt(dispatch[i]);
}

