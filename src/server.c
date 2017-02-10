#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "filesize.h"
#include "drop.h"

#define WWWROOT "/Users/artnavsegda/GitHub/jsnavsegda"

char * getmime(char * filename)
{
        if (strcmp(strchr(filename,'.'),".html")==0)
                return "\r\nContent-type: text/html\r\n\r\n";
        else if (strcmp(strchr(filename,'.'),".js")==0)
                return "\r\nContent-type: application/javascript\r\n\r\n";
        else if (strcmp(strchr(filename,'.'),".txt")==0)
                return "\r\nContent-type: text/plain\r\n\r\n";
        else if (strcmp(strchr(filename,'.'),".css")==0)
                return "\r\nContent-type: text/CSS\r\n\r\n";
        else
                return "\r\nContent-type: text/plain\r\n\r\n";
}

char * response(int code)
{
        switch (code)
        {
                case 200:
                        return "HTTP/1.1 200 OK";
                break;
                case 404:
                        return "HTTP/1.1 404 Not Found";
                break;
                default:
                        return "HTTP/1.1 200 OK";
                break;
        }
        return "HTTP/1.1 200 OK";
}

int main(void)
{
        chdir(WWWROOT);
        int nchar = 0;
	char *httpMimeType, *data;
	int code, webpage = -1;
        char buf[10000];
        int sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        drop(sock,"socket error");
        int reuseaddr = 1;
        setsockopt(sock,SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
        struct sockaddr_in server = {
                .sin_family = AF_INET,
                .sin_addr.s_addr = INADDR_ANY,
                .sin_port = htons(1100)
        };
        drop(bind(sock,(struct sockaddr *)&server,sizeof(server)),"bind error");
        drop(listen(sock,10),"listen error");
        while(1)
        {
                int msgsock = accept(sock,NULL,NULL);
                drop(msgsock,"accept error");
                while (nchar == 0)
                        ioctl(msgsock,FIONREAD,&nchar);
		printf(">>>>>> %d\n", nchar);
                //char *buf = malloc(nchar+1);
                int numread = recv(msgsock,buf,10000,0);
                //int numread = recv(msgsock,buf,nchar,0);
                drop(numread,"recv error");
                buf[numread] = '\0';

                puts(buf);

                char *method = strtok(buf," ");
                char *page = strtok(NULL," ?");
                char *buf2 = strtok(NULL,"");
                if (strcmp(page,"/") == 0)
                        page = "/index.html";
                if (strcmp(page,"/special") == 0)
                {
                        code = 200;
                        data = "<!doctype html><html><head><title>Special page</title></head><body><p>you're special</p></body></html>";
                        httpMimeType = getmime(".html");
                }
                else if (strcmp(page,"/getdata") == 0)
                {
                        code = 200;
                        //data = strstr(buf2,"\r\n\r\n")+4;
                        data = "something";
                        httpMimeType = getmime(".txt");
                }
                else
                {
                        webpage = open(&page[1],O_RDONLY);
                        if (webpage == -1)
                        {
                                code = 404;
                                data = "<!doctype html><html><head><title>404 Not Found</title></head><body><p>page not found</p></body></html>";
                                httpMimeType = getmime(".html");
                        }
                        else
                        {
                                code = 200;
                                data = malloc(filesize(webpage)+1);
                                drop2(data,"out of memory");
                                numread = read(webpage,data,filesize(webpage));
                                drop(numread,"read error");
                                data[numread] = '\0';
                                close(webpage);
                                httpMimeType = getmime(page);
                        }
                }
                drop(send(msgsock,response(code),strlen(response(code)),0),"send response error");
                drop(send(msgsock,httpMimeType,strlen(httpMimeType),0),"send mime type error");
                drop(send(msgsock,data,strlen(data),0),"send webpage error");
                //free(buf);
		if (webpage != -1)
                {
                	free(data);
                        data = NULL;
                        webpage = -1;
                }
                shutdown(msgsock,2);
                close(msgsock);
        }
        close(sock);
        return 0;
}
