// Student ID : 20202076
// Name : Kim Taewoo


// 맥
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <sys/time.h>
// #include <sys/stat.h>
// #include <netinet/in.h>
// #include <netdb.h>
// #include <arpa/inet.h>
// #include <ctype.h>
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// 윈도우
#include <unistd.h>
#include <sys/types.h>
#include <winsock2.h> //
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROMPT() {printf("\n> ");fflush(stdout);}
#define GETCMD "down"
#define QUITCMD "quit"


int main(int argc, char *argv[]) {
	int socktoserver = -1;
	char buf[BUFSIZ];

	char fname[BUFSIZ];	
	char hostname[256];
	char filename[256];
	char* pnum = "80";

	printf("Student ID : 20202076\n");
	printf("Name : Taewoo Kim\n");

	PROMPT(); 

	for (;;) {
		// 사용자 입력을 받음.
		if (!fgets(buf, BUFSIZ - 1, stdin)) {
			if (ferror(stdin)) {
				perror("stdin");
				exit(1);
			}
			exit(0);
		}

		char *cmd = strtok(buf, " \t\n\r");
		// 입력된 명령어 구분.

		if((cmd == NULL) || (strcmp(cmd, "") == 0)) {
			PROMPT(); 
			continue;
		} else if(strcasecmp(cmd, QUITCMD) == 0) {
				exit(0);
		}

		if((!strcasecmp(cmd, GETCMD)) == 0) {
			printf("Wrong command %s\n", cmd);
			PROMPT(); 
			continue;
		}
		
		// 호스트 이름, 포트번호, 파일 경로..
		char *host_start = buf + 7; // 'http://' 제거
        char *host_end = strchr(host_start, '/');
        int host_len = host_end - host_start;
        strncpy(hostname, host_start, host_len);
        hostname[host_len] = '\0';

        char *file_name = strrchr(buf, '/') + 1;
        strcpy(filename, file_name);


		socktoserver = ConnectToServer(hostname,pnum,filename);
		
		if (socktoserver < 0) {
            printf("Failed to connect to server.\n");
            exit(0);
        }

		char *realname = strrchr(filename, '/');
        if (realname == NULL)
            strcpy(fname, filename);
        else
            strcpy(fname, realname + 1);

        // read() 함수.. 서버로부터 자료를 읽어들인다.
        if (ParseResponse(socktoserver, fname) < 0) {
            printf("Failed to download file from server.\n");
        }
	}
}

// 호스트이름, 포트번호, 파일경로
// http://netapp.cs.kookmin.ac.kr/member/palladio.JPG
// 파일 URL에서 실제 파일 이름을 찾아내서 그 이름으로 저장..
// 서버에 연결이 안되면 에러 메시지 출력
// HTTP Response 메시지의 status 코드가 200이 아닌경우
// 주어진 URL 프로토콜이 http가 아닌 경우에도 에러 메시지 출력

int ConnectToServer(char* hostname, char* pnum, char* filename){
	int socktoserver;
	struct sockaddr_in server; // 바인드를 해줄 구조체 변수를 선언해준다.

	// 소켓 만들기.
	// AF_INET -> TCP IP 버전 4..
	// #define AF_INET	2
	// SOCK_STREAM -> TCP 통신을 하겠다.
	// SOCK_DGRAM -> UDP 통신을 하겠다.
	// TPPROTO_TCP -> 3번째인자는 프로토콜..을 의미함.
	// 그다음 바인드 해야함. 아이피랑 포트넘버랑 프로토콜을 지정해주는일 !

	// 일단 빈소켓을 만든다.
	if((socktoserver = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0){
		perror("socket");
		exit(1);
	}
	
	struct hostent *hostp;
	// 서버의 IP 주소 저장할 변수.
	// hostp에 서버의 IP주소를 저장한다.
	
	// 서버이름으로 서버의 IP 주소를 얻는다 !
	if((hostp = gethostbyname(hostname)) == 0){
		fprintf(stderr,"Error : unknown host\n");
		exit(1);
	}

	// 클라이언트는 특정 포트에 바인드 하는게 필요하지 않음.
	// 서버의 IP주소와 서버의 port번호를 소켓에 집어넣음 .!
	memset((void*)&server,0,sizeof server);
	server.sin_family = AF_INET; // 바인드 할때 쓰는 정보. IPv4를 쓰겠다.
	memcpy((void *) &server.sin_addr, hostp->h_addr, hostp->h_length);
	// server.sin_addr -> 서버의 IP주소 ..
	// hostp에 저장된 IP주소 정보를 ! server.sin_addr로 복사한다.
	// 호스트 IP 주소를 카피함. hostp->h_length.. 아이피 주소의 길이.
	server.sin_port = htons((u_short)atoi(pnum)); // 소켓 바인드 할때 어떤 주소로 바인드 할거냐.
	// pnum.. 포트넘버 설정.
	// hton -> 호스트의 바이트 순서를 네트워크 바이트 순서로 변경한다.
	// 네트워크에서는 오더를 통일함 빅엔디안으로 !!
	// 운영체제에 따라서 빅엔디안과 리틀엔디안이 다르기때문에 ! hton 함수를 써줘야함.
	// 바이트 오더를 맞춰줌.

	if (connect(socktoserver, (struct sockaddr *)&server, sizeof server) < 0) {
		(void) close(socktoserver);
		fprintf(stderr, "connect");
		exit(1);
	}

	char msg[BUFSIZ];
	sprintf(msg, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-agent: webcli/1.0\r\nConnection: close\r\n\r\n", filename, hostname);
	if(write(socktoserver, msg, strlen(msg)) < 0) {
		perror("write");
		exit(1);
	}

	return socktoserver;
}

int ParseResponse(int sd, char *fname) { 

	char buf[BUFSIZ];
	FILE *fpSock = fdopen(sd, "r");

	unsigned int numread = 0;
	unsigned int numtoread = 0;

	while(1) {
		// 소켓에서 데이터를 읽어들입니다.
		if (fgets(buf, BUFSIZ - 1, fpSock) != NULL) {
			if(strcmp(buf, "\r\n") == 0) {
				// 서버의 응답이 끝나면 루프를 빠져나갑니다.
				break;
			} else if(strncmp(buf, "HTTP/", 5) == 0) {
				// 서버의 응답 헤더라인을 공백 문자로 구분하여 상태코드 파싱..
				char *field = strtok(buf, " ");
				char *value = strtok(NULL, " ");
				char *rest = strtok(NULL, "");
				int val = atoi(value);
				if(val != 200) {
					printf("%s %s\n", value, rest);
					fclose(fpSock);
					return -1;
				}
			} else {
				char *field = strtok(buf, ":");
				char *value = strtok(NULL, " \t\n\r");
				if(strcmp(field, "Content-Length") == 0) {
					numtoread = atoi(value); 
					printf("Total Size %d bytes\n", numtoread);
					// 파일의 크기를 찾아내서 출력함.
				} 
				printf("%s %s\n", field, value);
			} 
		}
		else {
			printf("input error");
			return 0;
		}
	}  

	// now, open the file write
	FILE *fp = fopen(fname, "w+");
	if(fp == NULL) {
		printf("File Open Fail %s", fname);
	}

	unsigned int step = 1;
	while(1) {
		int nread = fread(buf, sizeof(char), BUFSIZ - 1, fpSock);
		if( nread <= 0)
			break;
		numread += (unsigned int)nread;
		fwrite(buf, sizeof(char), nread, fp);
		unsigned int fill = (unsigned int)numread * 10 / numtoread;
		if(fill >= step) {
			printf("Current Downloading %d/%d (bytes) %.0f%%\n", 
				numread, numtoread, (float)numread / numtoread * 100);
			step = fill + 1;
		}
	}		
	fclose(fpSock);
	fclose(fp);

	printf("Download Complete: %s, %d/%d\n",
		fname, numread, numtoread);
	return 1;
}