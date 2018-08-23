#include <stdio.h>
#include "csapp.h"
/*The error-handling functions provide in csapp.c are not appropriate for proxy, some of the functions have been modified*/

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define LRU_MAGIC_NUMBER 9999
#define CACHE_BLOCK_NUMS 10 /*total 10 cache blocks*/

/* You won't lose style points for including this long line in your code */  //4.2
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
/*Always send the following Connection header: Connection: close*/
static const char *conn_hdr = "Connection: close\r\n"; 
/*Always send the following Proxy-Connection header:Proxy-Connection: close*/
static const char *prox_hdr = "Proxy-Connection: close\r\n";
/*The Host header describes the hostname of the end server*/
static const char *host_hdr_format = "Host: %s\r\n";
/*Modern web browsers will generate HTTP/1.1 requests, but*******/
/*proxy should handle them and forward them as HTTP/1.0 requests.*/
static const char *requestlint_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *endof_hdr = "\r\n";

/*handles one HTTP transaction*/
void doit(int fd);
/*parses an HTTP URI*/
int parse_uri(char *uri, char *host, char *path, int *port);
/*4.2 Request headers*/
void build_http_header(char *http_header,char *host,char *path,int port,rio_t *rio);
/*sends error message to client*/
void clienterror(int fd, char *cause, char* errnum, char *shortmsg, char* longmsg);
void *thread(void *vargp);
void cache_init();
int cache_find(char *url);
int cache_eviction();
void cache_LRU(int index);
void cache_uri(char *uri,char *buf);
void readerPre(int i);
void readerAfter(int i);

typedef struct{
	char cache_object[MAX_OBJECT_SIZE];//
	char cache_uri[MAXLINE];
	int isEmpty;
	int LRU;
	int readNum;
	sem_t wMutex;
	sem_t rdnumMutex;
	int writeNum;
	sem_t wtnumMutex;
	sem_t queue;
}Cache_block;

typedef struct{
	Cache_block cache_objects[CACHE_BLOCK_NUMS];
	int cache_num;
}Cache;

Cache cache;


/*P957 TINY web server*/
int main(int argc,char **argv)
{
	int listenfd, *connfdp;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
	char hostname[MAXLINE], port[MAXLINE];
	pthread_t tid;
	
	cache_init();
	if (argc != 2){
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}
	Signal(SIGPIPE,SIG_IGN);
	listenfd = Open_listenfd(argv[1]);
	while(1){
		clientlen = sizeof(clientaddr);
		connfdp = Malloc(sizeof(int));/*avoid race*/
		*connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);//P936
		Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);// P940
		printf("Accepted connection from (%s, %s)\n", hostname, port);
		Pthread_create(&tid, NULL, thread, connfdp);		
	}
    return 0;
}

/* handles HTTP transaction*/
void doit(int fd)
{
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char host[MAXLINE], path[MAXLINE], server_http_header[MAXLINE];
	int port;
	char port_str[100];
	rio_t rio, server_rio;
	int serverfd;
	int client_state = 1;
	int server_state = 1;
	
	
	Rio_readinitb(&rio, fd);//p898
	Rio_readlineb(&rio, buf, MAXLINE, &client_state);//pp898
	//printf("Request headers:\n");
	//printf("%s", buf);
	sscanf(buf, "%s %s %s", method, uri, version);
	char uri_store[100];
	strcpy(uri_store,uri);

	if(strcasecmp(method, "GET")){ //only implement GET method
		clienterror(fd, method, "501","Not implemented","proxy does not implement this method");
		Close(fd);
		return;
	}

	/*uri cached?*/
	int ind;
	if((ind = cache_find(uri_store))!=-1){ 
		readerPre(ind);
        Rio_writen(fd,cache.cache_objects[ind].cache_object,strlen(cache.cache_objects[ind].cache_object));
        readerAfter(ind);
        cache_LRU(ind);
		Close(fd);
        return;
	}

	/*parse uri*/
	if(parse_uri(uri, host, path, &port)<0){
		clienterror(fd, method, "400","Bad request", "uri error");
		Close(fd);
		return;
	}
	
	/*connect to the server*/
	sprintf(port_str, "%d", port);
	if ((serverfd = Open_clientfd(host, port_str))<0){
		clienterror(fd, method, "404","Not found","server not found");
		Close(fd);
		return;
	}
	Rio_readinitb(&server_rio, serverfd);
	
	/*write request to server*/
	build_http_header(server_http_header, host, path, port, &rio); 
	Rio_writen(serverfd, server_http_header, strlen(server_http_header));

	/*receive message from server*/
	size_t i;
	int j;
	int message_size;
	char cacheBuf[MAX_OBJECT_SIZE];
	while((i=Rio_readlineb(&server_rio, buf, MAXLINE, &server_state))!=0 && client_state && server_state){

		message_size += i;
		if(message_size < MAX_OBJECT_SIZE) strcat(cacheBuf,buf);
		//printf("received %d bytes from server\n", i);
		/*send received message to client*/
		j=(Rio_writen(fd, buf, i));
		if(j==2){	//client closed
			client_state = 0;
			break;
		}	
	} 
	Close(fd);
	Close(serverfd);

	if(message_size < MAX_OBJECT_SIZE){
        cache_uri(uri_store,cacheBuf);
	}

}


int parse_uri(char *uri, char *host, char *path, int *port)
/* http://www.abcd.com:8080/1234/index.html  */ 
/*host: www.abcd.com    port: 8080   path: /1234/index.html*/
{
	char *hoststart;
	char *hostend;
	char *pathstart;
	int len;
	
	/*invalid uri*/
	if (strncasecmp(uri, "http://", 7)!=0){
		host[0] = '\0';
		return -1;
	}
	/*get hostname*/
	hoststart = uri + 7; // ignore"http://"
	hostend = strpbrk(hoststart, " :/\r\n\0");
	if (hostend==NULL) hostend = hoststart + strlen(hoststart);
	len = (int)(hostend-hoststart);
	strncpy(host,hoststart,len);
	host[len] = '\0';

	/* path*/
	pathstart = strchr(hoststart, '/');
	if (pathstart == NULL) strcpy(path,"/");//path[0] = '\0';
	else strcpy(path, pathstart);
	
	/*port*/
	*port = 80; //default port
	if(*hostend == ':') *port = atoi(hostend+1);

	return 0;
}
	 
void build_http_header(char *http_header, char *host, char *path, int port, rio_t *rio)
{
	char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];
	int client_state = 1;
	/* build request line*/
	sprintf(request_hdr, requestlint_hdr_format, path);
	while (Rio_readlineb(rio, buf, MAXLINE, &client_state)>0){
		if(strcmp(buf, endof_hdr)==0) break; 
		/*Host header*/
		if(!strncasecmp(buf, "Host", 4)){
			strcpy(host_hdr, buf);
			continue;
		}
		/*if a browser sends any additional request headers as part of an HTTP request,*/ 
		/*proxy should forward them unchanged.*/
		if(!strncasecmp(buf, "Connection", 10) && !strncasecmp(buf,"Proxy-Connection",16)
			&& !strncasecmp(buf, "User-Agent", 10)){
			strcat(other_hdr, buf);
		}
	}
	if(strlen(host_hdr)==0){
		sprintf(host_hdr, host_hdr_format, host);
	}
	/*build http request*/
	sprintf(http_header, "%s%s%s%s%s%s%s",
			request_hdr, host_hdr, conn_hdr, prox_hdr,
			user_agent_hdr, other_hdr, endof_hdr);
	return;
}

/*P959*/
void clienterror(int fd, char *cause, char* errnum, char *shortmsg, char* longmsg)
{
	char buf[MAXLINE], body[MAXBUF];
	
	/* Build the HTTP response body */
    sprintf(body, "<html><title>Can't find server!</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s<hr><em>Whoops, an error occured!</em>\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
	Rio_writen(fd, body, strlen(body));
}

void *thread(void *vargp)
{
	int connfd = *((int *)vargp);
	/*detached the joinable thread*/	
	Pthread_detach(pthread_self());
	Free(vargp);
	doit(connfd);
	//Close(connfd);
}
/******************************************
typedef struct{
	char cache_object[MAX_OBJECT_SIZE];//
	char cache_uri[MAXLINE];
	int isEmpty;
	int LRU;
	int readNum;
	sem_t wMutex;
	sem_t rdnumMutex;
	int writeNum;
	sem_t wtnumMutex;
	sem_t queue;
}Cache_block;

tyoedef struct{
	Cache_block cache_objects[CACHE_BLOCK_NUMS];
	int cache_num;
}Cache;

********************************************/

/********Cache********/
void cache_init(){
	cache.cache_num = 0;
	int i;
	for(i=0;i<CACHE_BLOCK_NUMS;i++){
		cache.cache_objects[i].LRU = 0;
		cache.cache_objects[i].readNum = 0;
		cache.cache_objects[i].isEmpty = 1;
		cache.cache_objects[i].writeNum = 0;
		Sem_init(&cache.cache_objects[i].wMutex,0,1);
		Sem_init(&cache.cache_objects[i].rdnumMutex,0,1);
		Sem_init(&cache.cache_objects[i].wtnumMutex,0,1);
		Sem_init(&cache.cache_objects[i].queue,0,1);
	}
}

void readerPre(int i){
	P(&cache.cache_objects[i].queue);
    P(&cache.cache_objects[i].rdnumMutex);
    cache.cache_objects[i].readNum++;
    if(cache.cache_objects[i].readNum == 1) P(&cache.cache_objects[i].wMutex);
    V(&cache.cache_objects[i].rdnumMutex);
    V(&cache.cache_objects[i].queue);
}

void readerAfter(int i){
    P(&cache.cache_objects[i].rdnumMutex);
    cache.cache_objects[i].readNum--;
    if(cache.cache_objects[i].readNum==0) V(&cache.cache_objects[i].wMutex);
    V(&cache.cache_objects[i].rdnumMutex);

}

void writePre(int i){
    P(&cache.cache_objects[i].wtnumMutex);
    cache.cache_objects[i].writeNum++;
    if(cache.cache_objects[i].writeNum==1) P(&cache.cache_objects[i].queue);
    V(&cache.cache_objects[i].wtnumMutex);
    P(&cache.cache_objects[i].wMutex);
}

void writeAfter(int i){
    V(&cache.cache_objects[i].wMutex);
    P(&cache.cache_objects[i].wtnumMutex);
    cache.cache_objects[i].writeNum--;
    if(cache.cache_objects[i].writeNum==0) V(&cache.cache_objects[i].queue);
    V(&cache.cache_objects[i].wtnumMutex);
}

int cache_find(char *uri){
    int i;
    for(i=0;i<CACHE_BLOCK_NUMS;i++){
        readerPre(i);
        if((cache.cache_objects[i].isEmpty==0) && (strcmp(uri,cache.cache_objects[i].cache_uri)==0)){ 
			readerAfter(i);	
			break;
		}
        readerAfter(i);
    }
    if(i>=CACHE_BLOCK_NUMS) return -1; /*can not find uri in the cache*/
    return i;
}

/*find the empty cacheObj or which cacheObj should be evictioned*/
int cache_eviction(){
    int min = LRU_MAGIC_NUMBER;
    int minindex = 0;
    int i;
    for(i=0; i<CACHE_BLOCK_NUMS; i++)
    {
        readerPre(i);
        if(cache.cache_objects[i].isEmpty == 1){/*choose if cache block empty */
            minindex = i;
            readerAfter(i);
            break;
        }
        if(cache.cache_objects[i].LRU< min){    /*if not empty choose the min LRU*/
            minindex = i;
            readerAfter(i);
            continue;
        }
        readerAfter(i);
    }

    return minindex;
}
/*update the LRU number except the new cache one*/
void cache_LRU(int index){

    writePre(index);
    cache.cache_objects[index].LRU = LRU_MAGIC_NUMBER;
    writeAfter(index);

    int i;
    for(i=0; i<index; i++)    {
        writePre(i);
        if(cache.cache_objects[i].isEmpty==0 && i!=index){
            cache.cache_objects[i].LRU--;
        }
        writeAfter(i);
    }
    i++;
    for(i; i<CACHE_BLOCK_NUMS; i++)    {
        writePre(i);
        if(cache.cache_objects[i].isEmpty==0 && i!=index){
            cache.cache_objects[i].LRU--;
        }
        writeAfter(i);
    }
}
/*cache the uri and content in cache*/
void cache_uri(char *uri,char *buf){


    int i = cache_eviction();

    writePre(i);/*writer P*/

    strcpy(cache.cache_objects[i].cache_object,buf);
    strcpy(cache.cache_objects[i].cache_uri,uri);
    cache.cache_objects[i].isEmpty = 0;

    writeAfter(i);/*writer V*/

    cache_LRU(i);
}

