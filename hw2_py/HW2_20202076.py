import sys
import os
import socket

serverPort = int(sys.argv[1])
serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serverSocket.bind(('', serverPort))

serverSocket.listen(1)
print("Student ID :20202076", flush=True)
print("Name: Taewoo Kim", flush=True)

CONTENT_BODY =\
"""HTTP/1.0 200 OK
Connection: close
Content-Length: {length}
Content-Type: {type}

"""
ERROR_BODY =\
"""HTTP/1.0 404 NOT FOUND
Connection: close
Content-Length: 0
Content-Type: text/html

"""

def Converter(type, data):
    print(f"finish {len(data)} {len(data)}", flush=True)
    return CONTENT_BODY.replace("{type}", type).replace("{length}", str(len(data))).encode('ascii')+data

while True:
    clientSocket, addr = serverSocket.accept()

    print(f"Connetion : Host IP {addr[0]}, Port {addr[1]}, Socket {clientSocket.fileno()}", flush=True)

    request = clientSocket.recv(1024).strip()
    request_lines = request.decode().split("\n")
    
    # 요청 메시지의 첫번째 라인과 user-Agent 정보 출력.
    print(request_lines[0], flush=True)
    for line in request_lines:
        if line.startswith('User-Agent:'):
            user_agent = line.split(': ')[1]
            print(f'User-Agent: {user_agent}', flush=True)
            break
    
    # 헤더 필드의 수 출력 ..
    header_fields=[line for line in request_lines[1:] if line.strip()]
    print(f"{len(header_fields)} headers", flush=True)
        # 요청 라인 다음부터 빈 라인까지의 라인 수를 헤더 필드 수로 간주
    # header_fields_count = 0
    # found_blank_line = False
    # for line in request_lines[1:]:
    #     if line == '':
    #         found_blank_line = True
    #         break
    #     header_fields_count += 1
    # print(f"{(header_fields_count)} headers", flush=True)
    
    if request.startswith(b'GET '):
        filename = '.'+request.split()[1].decode()

        if os.path.exists(filename) and os.path.isfile(filename):
            with open(filename, 'rb') as f:
                file_data = f.read()
            if filename.split('.')[2] == "html":
                clientSocket.sendall(Converter('text/html', file_data))
            elif filename.split('.')[2] == "jpg":
                clientSocket.sendall(Converter('image/jpeg', file_data))
            else:
                clientSocket.sendall(Converter('text/plain', file_data))
        else:
            print(f"Server Error : No such file {filename}!", flush=True)
            clientSocket.sendall(ERROR_BODY.encode('ascii'))

    clientSocket.close()