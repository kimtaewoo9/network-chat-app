// Student ID : 20202076
// Name : 김태우

// MAC
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

// 윈도우
// 윈도우는 winsock2.
// #include <unistd.h>
// #include <sys/types.h>
// #include <winsock2.h> 
// #include <sys/time.h>
// #include <sys/stat.h>
// #include <ctype.h>
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <ws2tcpip.h> // socklen_t 사용하려면 이거 추가해야함.
// #include <stdio.h>
// #include <fcntl.h>
// #include <pthread.h>

#define PROMPT() {printf("\n> ");fflush(stdout);}
// stdout 으로 출력한 내용이 있으면 flush를 시켜야 정상적으로 내용이 출력됨.

// 이 서버를 이용해서 사용자가 할 수 있는 3가지 .
// 특정 확장자를 가진 파일 목록 다운로드.
// 주어진 이름의 파일 다운로드
// 파일 업로드 !


// 어떤 요청을 받으면 이 브라우저가 요청한 HTTP request 메시지의 첫번째 라인과
// User-Agent 정보를 출력한다. 또한 헤더 필드의 수를 같이 출력한다.
// 그리고 해당하는 파일을 찾아서 HTTP response message로 만들어서 브라우저로 보낸다.



#define HEADER_FORMAT "HTTP/%s %s\r\n"
#define CONNECTION "Connection: %s\r\n"

#define OK 200
#define BAD_REQUEST 400
#define FORBIDDEN 403
#define NOT_FOUND 404

#define MAX_REQUEST_LINE_LENGTH 1024
#define MAX_USER_AGENT_LENGTH 256

#define BUFFER_SIZE 1024

int count_header_fields(const char *request);
void parse_http_request(const char *request, char *request_line, char *user_agent);
char* removeChar(const char* str, char charToRemove);
void send_response(int client_socket, char *file_path, const char *content_type);


int count_header_fields(const char *request){
   char *header_end = strstr(request, "\r\n\r\n");
    if (header_end == NULL) {
       return 0; // 잘못된 요청 형식
    }

    int num_fields = 0;
   for (const char *p = request; p < header_end; p++) {
       if (*p == '\r' && *(p + 1) == '\n') {
            num_fields++;
        }
    }

    return num_fields;
}

void parse_http_request(const char *request, char *request_line, char *user_agent) {
    char *newline, *user_agent_start;

   // Extract the request line
    newline = strchr(request, '\n'); // 개행문자 위치 반환
    if (newline) {
        size_t request_line_length = newline - request;
        if (request_line_length < MAX_REQUEST_LINE_LENGTH) {
            strncpy(request_line, request, request_line_length);
            request_line[request_line_length] = '\0';
        } else {
            strncpy(request_line, request, MAX_REQUEST_LINE_LENGTH - 1);
            request_line[MAX_REQUEST_LINE_LENGTH - 1] = '\0';
        }
    }
    else{
        strcpy(request_line, "Invalid request");
    }

    // Extract the User-Agent header
    user_agent_start = strstr(request, "User-Agent: "); // User-Agent: 의 위치를 반환함.
    if (user_agent_start) {
        user_agent_start += strlen("User-Agent: ");
        char *user_agent_end = strstr(user_agent_start, " ");
        if (user_agent_end) {
            size_t user_agent_length = user_agent_end - user_agent_start;
            if (user_agent_length < MAX_USER_AGENT_LENGTH) {
                strncpy(user_agent, user_agent_start, user_agent_length);
                user_agent[user_agent_length] = '\0';
            }
            else {
                strncpy(user_agent, user_agent_start, MAX_USER_AGENT_LENGTH - 1);
                user_agent[MAX_USER_AGENT_LENGTH - 1] = '\0';
            }
        } else {
            strcpy(user_agent, "Invalid User-Agent");
        }
    } else {
        strcpy(user_agent, "No User-Agent");
    }
}

// char* removeChar(const char* str, char charToRemove){
//     char* src = str;
//     char* dst = str;
    
//     while (*src) {
//         if (*src != charToRemove) {
//             *dst++ = *src;
//         }
//         src++;
//     }
//     *dst = '\0'; // 결과 문자열 끝에 null 문자 추가
    
//     return str;
// }

void send_response(int client_socket, char *file_path, const char *content_type){
    char response_buffer[BUFFER_SIZE];
    char content_length_buffer[20];
    struct stat file_stat;
    int file_fd;
    ssize_t bytes_read, bytes_sent;
    off_t total_bytes_sent = 0;

    // 파일 열기    
    // 파일 경로에서 맨 앞의 '/'를 제거
    char *path_without_leading_slash = file_path;
    if (file_path[0] == '/') {
        file_path = file_path + 1;
    }
    
    // 파일 열기
    file_fd = open(file_path, O_RDONLY);

    // 파일이 없는 경우
	if (file_fd == -1) {
		fprintf(stderr, "Server Error: No such file %s!\n", file_path);
        fprintf(stderr, "close error: Bad file descriptor\n");
        fprintf(stderr, "No Data: close connetcion\n");
		sprintf(response_buffer, "HTTP/1.0 404 NOT FOUND\r\nConnection: close\r\nContent-Length: 0\r\nContent-Type: text/html\r\n\r\n");
        
		// 클라이언트에게 응답 전송하고 바로 return ..
		bytes_sent = send(client_socket, response_buffer, strlen(response_buffer), 0);
        if (bytes_sent == -1) {
            perror("send");
        }
		return;
    }

    // 파일 크기 가져오기
    if (stat(file_path, &file_stat) == -1) {
        perror("stat");
        close(file_fd);
        return;
    }

    // 응답 헤더 생성
    sprintf(response_buffer, "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Length: %lld\r\nContent-Type: %s\r\n\r\n",
            file_stat.st_size, content_type);
    sprintf(content_length_buffer, "%lld", file_stat.st_size);

    // 응답 헤더 전송
    bytes_sent = send(client_socket, response_buffer, strlen(response_buffer), 0);
    if (bytes_sent == -1) {
        perror("send");
        close(file_fd);
        return;
    }

    total_bytes_sent += bytes_sent;
    // 파일 내용 전송
    while ((bytes_read = read(file_fd, response_buffer, BUFFER_SIZE)) > 0) {
        bytes_sent = send(client_socket, response_buffer, bytes_read, 0);
        if (bytes_sent == -1) {
            perror("send");
            close(file_fd);
            return;
        }
        total_bytes_sent += bytes_sent;
    }

    // 전송 결과 출력
    // 실제 전송한 파일의 바이트 수와 파일의 바이트 수 출력
    printf("finish %lld %s\n", total_bytes_sent, content_length_buffer);

    close(file_fd);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in server, remote; // 서버, 클라이언트 빈 소켓 만들기.
	int request_sock, new_sock; // 소켓 디스크립터 저장할 변수.
	int bytesread;
	socklen_t addrlen; // 소켓 주소 구조체의 길이를 addrlen에 저장.
	char buf[BUFSIZ];

    // 매개 변수 없으면 프로그램 종료
	if (argc != 2) {
		(void) fprintf(stderr,"usage: %s portnum \n",argv[0]);
		exit(1);
	}

    int portnum = atoi(argv[1]);

	if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}
	
	printf("Student ID : 20202076\n");
	printf("Name : Taewoo Kim\n");

    PROMPT(); // flush

	// Create a Server Socket
	memset((void *) &server, 0, sizeof (server));
	// server 구조체의 모든 필드를 0으로 초기화함.
	server.sin_family = AF_INET; // 주소 체계 저장 .. IPv4..
	server.sin_addr.s_addr = INADDR_ANY; // 32비트 IPv4 주소 저장
	// INADDR_ANY -> 현재 이 PC가 사용하고 있는 IP 주소를 자동으로 가져옴
	// htonl -> 호스트 오더에서 네트워크 오더로 바꿔라 라는 함수 근데 l이니까 long 32비트
	server.sin_port = htons((u_short)atoi(argv[1])); // argv[1] -> 파라미터로 준 포트번호 10000
	// 네트워크 바이트 순서로 포트번호가 저장됨 .. 80이면 0x0050이 저장됨.(네자리 16진수)
	// htons로 16비트 값을 네트워크 바이트 순서(big endian)로 변환한다.(host to network)
	// 이 코드들로 server 구조체에는 서버 소켓의 주소정보가 설정됨.
	// argv[1]-> 프로그램 실행시 전달된 첫번째 인수.. 즉 port number.
	// 문자열을 정수로 변환한 후

	// 생성된 소켓에 IP와 포트번호 바인딩 .. 빈 소켓을 채워 넣어준다고 생각하면 됨.
	// request_sock -> 아까 만든 서버 소켓.
	// (struct sockaddr*)&server ->할당할 IP주소와 포트번호가 포함된.. sockaddr 구조체의 주소를 전달함.
	// server 구조체의 size도 전달해야함.3
	if (bind(request_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("bind");
		exit(1);
	}
	// 서버 소켓을 수신 대기모드로 전환함. 클라이언트의 요청을 받기 위해서.
	// SOMAXCONN -> 대기할 수 있는 최대 연결 요청의 수!(버퍼의 크기)
	if (listen(request_sock, SOMAXCONN) < 0) {
		perror("listen");
		exit(1);
	}

	// 클라이언트의 요청을 반복문으로 받아들임.
	for (;;) {
		addrlen = sizeof(remote);
		// addrlen -> 클라이언트의 address size
		// remote -> 서버에 연결된 클라이언트의 정보 저장..
		// 근데 이제 IP 랑 portnumber 겠지 .
		new_sock = accept(request_sock,(struct sockaddr *)&remote, &addrlen);
		// accept함수가 성공적으로 실행되면 remote 구조체에
		// 클라이언트의 IPv4 주소와 포트번호가 저장됨 .. 
		// new_sock에는 클라이언트와 통신할 새로운 소켓 디스크립터가 저장됨.
		// new_sock.. 클라이언트 소켓의 소켓 디스크립터.
		// 소켓 디스크립터 -> 운영체제에서 사용하는 소켓을 식별하기 위한 숫자.!
		// 서버에서는 2종류의 소켓을 씀 .. 서버 소켓 ,클라이언트 소켓 !

		if (new_sock < 0) {
			perror("accept");
			exit(1);
		}
		// 클라이언트의 IP주소, port number, 소켓 디스크립터 출력.
		printf("Connection : Host IP %s, Port %d, Socket %d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), new_sock);
        PROMPT(); 
		// http request 메시지의 첫번째 라인과 user agent 출력 ..
		// user agent 헤더필드 -> 클라이언트가 자신의 정보를 알려주기 위해 사용 .
		// 제품이름/버전

		for (;;) {
			bytesread = read(new_sock, buf, sizeof(buf) - 1);
			// new_sock -> 읽을 파일, 혹은 소켓의 디스크립터.
			// read() 는 성공적으로 읽은 바이트수를 반환함 ..
			if (bytesread<=0) {
				if (close(new_sock)) 
					perror("close");
				break;
			}
			
			// 문자열 파싱 .
            // HTTP request 메시지의 첫번째 라인과 User-Agent 정보를 출력한다.
            // 그리고 헤더 필드 수를 출력함

			buf[bytesread] = '\0';  // 문자열 끝 표시

            char request_line[MAX_REQUEST_LINE_LENGTH];
            char user_agent[MAX_USER_AGENT_LENGTH];

			// request 파싱하기..
            parse_http_request(buf, request_line, user_agent);

			// request 메시지의 첫번째 라인 + user_agent 정보 출력.
			// + 헤더 필드의 수 출력.
            printf("%s\n", request_line);
            printf("User-Agent: %s\n", user_agent);
            int num_header_fields = count_header_fields(buf);
            printf("%d headers\n", num_header_fields);
            PROMPT();
			// 요청 파일 경로 파싱
            char *file_path = strchr(request_line, ' ') + 1;
            char *end_ptr = strchr(file_path, ' ');
            if (end_ptr) {
                *end_ptr = '\0';
            }

            // 요청 파일 확장자에 따른 Content-Type 설정
			// 파일 확장자는 html, jpeg 두가지..
            const char *Content_Type;
            char *extension = strrchr(file_path, '.');
            if (extension) {
                if (strcmp(extension, ".html") == 0 || strcmp(extension, ".htm") == 0) {
                    Content_Type = "text/html";
                }
				else if (strcmp(extension, ".jpg") == 0 || strcmp(extension, ".jpeg") == 0) {
                    Content_Type = "image/jpeg";
                }
            }

            send_response(new_sock, file_path, Content_Type);
		}
		close(new_sock);
	}
	close(new_sock);
	return 0;
} /* main - hw2.c */