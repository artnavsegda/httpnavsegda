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

char * getmime(char * filename)
{
        if (strcmp(strchr(filename,'.'),".html")==0)
                return "\nContent-type: text/html\n\n";
        else if (strcmp(strchr(filename,'.'),".js")==0)
                return "\nContent-type: application/javascript\n\n";
        else if (strcmp(strchr(filename,'.'),".txt")==0)
                return "\nContent-type: text/plain\n\n";
        else if (strcmp(strchr(filename,'.'),".css")==0)
                return "\nContent-type: text/CSS\n\n";
        else
                return "\nContent-type: text/plain\n\n";
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

void drop(int dropstatus, char *dropdesc)
{
        if (dropstatus == -1)
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

void drop2(char *dropstatus, char *dropdesc)
{
        if (dropstatus == NULL)
        {
                perror(dropdesc);
                exit(1);
        }
}

int main(void)
{
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
                int numread = recv(msgsock,buf,10000,0);
                drop(numread,"recv error");
                buf[numread] = '\0';
                char *method = strtok(buf," ");
                char *page = strtok(NULL," ");
                char *buf2 = strtok(NULL,"");
                if (strcmp(page,"/") == 0)
                        page = "/index.html";
                if (strcmp(page,"/special") == 0)
                {
                        code = 200;
                        data = "<!doctype html><html><head><title>Special page</title></head><body><p>you're special</p></body></html>";
                        httpMimeType = getmime(".html");
                }
                else if (strcmp(page,"/getopt") == 0)
                {
                        code = 200;
                        data = strstr(buf2,"\r\n\r\n")+4;
                        httpMimeType = getmime(".txt");
                }
                else if (strcmp(page,"/setopt") == 0)
                {
                        code = 200;
                        data = strstr(buf2,"\r\n\r\n")+4;
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
                                numread = read(webpage,data,filesize(webpage)+1);
                                drop(numread,"read error");
                                data[numread] = '\0';
                                close(webpage);
                                httpMimeType = getmime(page);
                        }
                }
                drop(send(msgsock,response(code),strlen(response(code)),0),"send response error");
                drop(send(msgsock,httpMimeType,strlen(httpMimeType),0),"send mime type error");
                drop(send(msgsock,data,strlen(data),0),"send webpage error");
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
