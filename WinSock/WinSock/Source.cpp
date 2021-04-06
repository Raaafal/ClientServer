#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<winsock.h>
#include<cctype>
#include <process.h>
#include<string.h>
#pragma comment(lib, "Ws2_32.lib")

SOCKET *sockety;
HANDLE semSockety;
int rozmiarSocketow = 1;
int liczbaSocketow = 0;
void inicjalizujSockety() {
	sockety = (SOCKET *)malloc(sizeof(SOCKET)*rozmiarSocketow);
	*sockety = 0;
	semSockety = CreateSemaphore(
		NULL,           // default security attributes
		1,  // initial count
		1,  // maximum count
		NULL);          // unnamed semaphore
}
void dodajSocketa(SOCKET s,int ignorujSemafory) {
	if(!ignorujSemafory)
	WaitForSingleObject(
		semSockety,   // handle to semaphore
		INFINITE);           // time-out interval
	for (int i = 0; i < liczbaSocketow; i++) {
if (!*(sockety + i)) {
	*(sockety + i) = s;
	if (!ignorujSemafory)
		ReleaseSemaphore(
			semSockety,  // handle to semaphore
			1,            // increase count by one
			NULL);       // not interested in previous count
	return;
}

	}
	liczbaSocketow++;
	if (liczbaSocketow >= rozmiarSocketow) {
		rozmiarSocketow *= 2;
		SOCKET *temp = sockety;
		sockety = (SOCKET *)malloc(sizeof(SOCKET)*rozmiarSocketow);
		for (int i = 0; i < liczbaSocketow - 1; i++) {
			*(sockety + i) = *(temp + i);
		}
		free(temp);
	}
	*(sockety + liczbaSocketow - 1) = s;
	if (!ignorujSemafory)
		ReleaseSemaphore(
			semSockety,  // handle to semaphore
			1,            // increase count by one
			NULL);       // not interested in previous count
}
void usunSocketa(SOCKET s) {
	WaitForSingleObject(
		semSockety,   // handle to semaphore
		INFINITE);           // time-out interval
	for (int i = 0; i < liczbaSocketow; i++) {
		if ((*(sockety + i)) == s) {
			*(sockety + i) = 0;
			break;
		}

	}
	closesocket(s);
	ReleaseSemaphore(
		semSockety,  // handle to semaphore
		1,            // increase count by one
		NULL);       // not interested in previous count
}
void wyslijDoWszystkich(char *wiadomosc) {
	WaitForSingleObject(
		semSockety,   // handle to semaphore
		INFINITE);           // time-out interval
	for (int i = 0; i < liczbaSocketow; i++) {
		if (*(sockety + i))
			send(*(sockety + i), wiadomosc, strlen(wiadomosc) + 1, 0);
	}
	ReleaseSemaphore(
		semSockety,  // handle to semaphore
		1,            // increase count by one
		NULL);       // not interested in previous count
}




HANDLE semZapisWiadomosci;
char **wiadomosci = 0;
int liczbaWiadomosci = 0;
int rozmiar = 1;
void inicjalizujWiadomosci() {
	wiadomosci = (char**)malloc(sizeof(char **)*rozmiar);
	semZapisWiadomosci = CreateSemaphore(
		NULL,           // default security attributes
		1,  // initial count
		1,  // maximum count
		NULL);          // unnamed semaphore
}
void dodaj(char *wiadomosc) {
	WaitForSingleObject(
		semZapisWiadomosci,   // handle to semaphore
		INFINITE);           // time-out interval
	wyslijDoWszystkich(wiadomosc);
	liczbaWiadomosci++;
	if (liczbaWiadomosci >= rozmiar) {
		rozmiar *= 2;
		char **temp = wiadomosci;
		wiadomosci = (char **)malloc(sizeof(char *)*rozmiar);
		for (int i = 0; i < liczbaWiadomosci - 1; i++) {
			*(wiadomosci + i) = *(temp + i);
		}
		free(temp);
	}
	*(wiadomosci + liczbaWiadomosci - 1) = wiadomosc;
	ReleaseSemaphore(
		semZapisWiadomosci,  // handle to semaphore
		1,            // increase count by one
		NULL);       // not interested in previous count
}
void wyslijWszystkieIDodajSocketa(SOCKET s) {//dodawanie socketa w tej samej funkcji, ¿eby odby³o siê za tym samym opuszczeniem semafora
	WaitForSingleObject(
		semZapisWiadomosci,   // handle to semaphore
		INFINITE);           // time-out interval
	for (int i = 0; i < liczbaWiadomosci; i++) {
		if (send(s, *(wiadomosci + i), strlen(*(wiadomosci + i)) , 0) <= 0){
			ReleaseSemaphore(
				semZapisWiadomosci,  // handle to semaphore
				1,            // increase count by one
				NULL);       // not interested in previous count
			closesocket(s);
			return;
		}
	}

	dodajSocketa(s,1);
	ReleaseSemaphore(
		semZapisWiadomosci,  // handle to semaphore
		1,            // increase count by one
		NULL);       // not interested in previous count
}

void obsluga(void *data) {
	SOCKET s = (SOCKET)data;
	char imie[80];
	int odp = recv(s, imie, 80, 0);
	if (!odp) {
		printf("\nKlient zerwal polaczenie\n");
		closesocket(s);
		return;
	}
	else if (odp == SOCKET_ERROR) {
		printf("\nSocket Error\n");
		closesocket(s);
		return;
	}
	imie[0] = toupper(imie[0]);
	int result = send(s, imie, strlen(imie), 0);
	if (result <= 0) {
		printf("Nowy uzytkownik %s utracil polaczenie\n",imie);
		closesocket(s);
		return;
	}
	printf("Polaczyl sie nowy uzytkownik: %s\n", imie);

	//dodajSocketa(s,0);
	wyslijWszystkieIDodajSocketa(s);

	char buf[10];
	int dlugoscImienia = strlen(imie);
	int rozmiarWiadomosci = 3 + dlugoscImienia;
	char *wiadomosc=(char *)malloc(rozmiarWiadomosci);
	//*wiadomosc = '\0';
	strcpy(wiadomosc,imie);
	*(wiadomosc + dlugoscImienia) = ':';
	*(wiadomosc + dlugoscImienia + 1) = ' ';
	*(wiadomosc + dlugoscImienia + 2) = '\0';
	
	
	while (1)
	{
		int odp = recv(s, buf, 10, 0);
		if (!odp) {
			printf("\nKlient zerwal polaczenie\n");
			break;
		}
		else if (odp == SOCKET_ERROR) {
			printf("\nSocket Error\n");
			break;
		}
		//printf("\n%s", buf);
		
		int dlug = strlen(buf);
		//for (int i = 0; i < dlug; i++)buf[i] = toupper(buf[i]);
		
		char *nowaWiadomosc = (char *)malloc(dlug + rozmiarWiadomosci);
		rozmiarWiadomosci += dlug;
		strcpy(nowaWiadomosc, wiadomosc);
		strcat(nowaWiadomosc, buf);
		free(wiadomosc);
		wiadomosc = nowaWiadomosc;
		if (buf[dlug - 1] == '\n') {
			printf("%s", wiadomosc);

			dodaj(wiadomosc);
			/*
			int result = send(s, wiadomosc, rozmiarWiadomosci, 0);
			if (result <= 0) {
				printf("Utracono polaczenie\n");
				break;
			}*/
			rozmiarWiadomosci = 3 + dlugoscImienia;
			wiadomosc = (char *)malloc(rozmiarWiadomosci);
			//*wiadomosc = '\0';
			strcpy(wiadomosc, imie);
			*(wiadomosc + dlugoscImienia) = ':';
			*(wiadomosc + dlugoscImienia + 1) = ' ';
			*(wiadomosc + dlugoscImienia + 2) = '\0';
			/*
			while(dlug + 1 < odp) {
				int dlug2 = strlen(buf+dlug+1);
				char *nowaWiadomosc = (char *)malloc(dlug2 + rozmiarWiadomosci);
				rozmiarWiadomosci += dlug2;
				strcpy(nowaWiadomosc, wiadomosc);
				strcat(nowaWiadomosc, buf+dlug+1);
				free(wiadomosc);
				wiadomosc = nowaWiadomosc;
				dlug += dlug2 + 1;
			}*/
		}
		
	};
	usunSocketa(s);
	//closesocket(s);
	//WSACleanup();
}


int main() {
	inicjalizujWiadomosci();
	inicjalizujSockety();
	WSADATA wsas;
	int result;
	WORD wersja;
	wersja = MAKEWORD(1, 1);
	result = WSAStartup(wersja, &wsas);
	//Nastepnie tworzone jest gniazdko za pomoca funkcji socket
	SOCKET s;
	s = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in sa;
	memset((void *)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(8083);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	result = bind(s, (struct sockaddr FAR *) & sa, sizeof(sa));

	result = listen(s, 5);

	SOCKET si;
	struct sockaddr_in sc;
	int lenc;
	for (;;)
	{
		lenc = sizeof(sc);
		si = accept(s, (struct sockaddr FAR *) & sc, &lenc);
		_beginthread(obsluga,0,(void *)si);
		//dodajSocketa(si);
		
		/*
		char buf[80];
		while (1)
		{
			int odp = recv(si, buf, 80, 0);
			if(!odp) {
				printf("\nKlient zerwal polaczenie\n");
				break;
			}
			else if (odp==SOCKET_ERROR) {
				printf("\nSocket Error\n");
				break;
			}
			
			if (strcmp(buf, "KONIEC") == 0)
			{
				printf("\nOtrzymano sygnal \'KONIEC\'\n");
				closesocket(si);
				break;
				//return 0;
			}
			printf("\n%s", buf);

			int dlug = strlen(buf);
			for (int i = 0; i < dlug; i++)buf[i] = toupper(buf[i]);
			send(si, buf, dlug, 0);
		};*/

	}

	if (wiadomosci != NULL)free(wiadomosci);
	if (sockety != NULL)free(sockety);

	return 0;
}