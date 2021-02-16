#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32")
#include <WinSock2.h>
#include <stdio.h>


//오류 메시지를 출력하고 프로그램을 종료하는 함수
void err_quit(char *msg)
{
	//에러 코드로부터 오류 메시지를 만든다.
	LPSTR msgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuf, 0, NULL);

	//오류 메시지를 알림창으로 띄운다.
	MessageBox(NULL, msgBuf, msg, MB_ICONERROR);

	//버퍼 메모리 반환
	LocalFree(msgBuf);

	//프로그램 종료
	exit(1);
}

//오류 메시지를 출력하는 함수
void err_display(char *msg)
{
	//에러 코드로부터 오류 메시지를 만든다.
	LPSTR msgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuf, 0, NULL);

	//오류 메시지를 콘솔창에 출력한다.
	printf("[%s] %s\n", msg, msgBuf);

	//버퍼 메모리 반환
	LocalFree(msgBuf);
}

int main()
{
	int retval;

	//윈속을 초기화한다.
	WSADATA wsa;
	retval = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (retval != 0)
	{
		printf("WSAStartup() 오류: %d", retval);
		return 1;
	}

	//서버 대기용 소켓을 생성한다.
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()");

	//지역 IP주소와 지역 포트번호를 설정한다.
	SOCKADDR_IN serveraddr = { 0 };
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(50000);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");

	//서버 대기용 소켓을 LISTENING 상태로 변경한다.
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		err_quit("listen()");

	//통신에 사용할 변수를 선언한다.
	SOCKET client_sock;		//통신용 소켓
	SOCKADDR_IN clientaddr;		//클라이언트의 정보를 담는 소켓 주소 구조체
	int addrlen = sizeof(clientaddr);	//위의 소켓 주소 구조체의 크기를 저장하는 변수
	char buffer[512] = { '\0' };	//데이터 통신에서 사용하는 버퍼

	while (1)
	{
		//클라이언트의 접속을 기다린다.
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			break;
		}

		//클라이언트가 접속하면 정보를 출력한다.
		printf("\n[TCP 서버] 클라이언트 접속: %s:%d \n\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		while (1)
		{
			//데이터를 수신한다.
			retval = recv(client_sock, buffer, sizeof(buffer), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;
			
			//수신한 데이터를 출력한다.
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buffer);

			//수신한 데이터를 그대로 송신한다.
			retval = send(client_sock, buffer, retval, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}
		}

		//통신용 소켓을 닫는다.
		retval = closesocket(client_sock);
		if (retval != 0)
			err_display("closesocket()");

		//클라이언트의 종료를 알린다.
		printf("\n[TCP 서버] 클라이언트 종료: %s:%d \n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	//서버 대기용 소켓을 닫는다.
	retval = closesocket(listen_sock);
	if (retval != 0)
		err_display("closesocket()");

	//윈속을 종료한다.
	retval = WSACleanup();
	if (retval != 0)
		err_display("WSACleanup()");

	return 0;
}