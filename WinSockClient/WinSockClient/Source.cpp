#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<Winsock2.h>
#include<stdio.h>
#include <windows.h>
#include<process.h>
#include<conio.h>
#include<math.h>
#pragma comment(lib, "Ws2_32.lib")

SOCKET s;
char imie[80];
int wyswietlaImie = 0;
int wlasnaWiadomoscNaKonsoli = 0;
//HANDLE semWyswietlaImie;


HANDLE semTabelaCzytania;
char *tabelaCzytania;
int rozmiarBufora = 1;
int indexCzytania = 0;
void inicjalizacjaCzytania() {
	tabelaCzytania = (char*)malloc(rozmiarBufora);
	*tabelaCzytania = '\0';
	semTabelaCzytania = CreateSemaphore(
		NULL,           // default security attributes
		1,  // initial count
		1,  // maximum count
		NULL);          // unnamed semaphore
}
int lbackspace = 0;
int linnychznakow = 0;
void usunPrzeczytane(int ignorujSem) {
	if(!ignorujSem)
	WaitForSingleObject(
		semTabelaCzytania,   // handle to semaphore
		INFINITE);           // time-out interval
	lbackspace = 0;
	linnychznakow = 0;
	indexCzytania = 0;
	*tabelaCzytania = '\0';
	if (!ignorujSem)
	ReleaseSemaphore(
		semTabelaCzytania,  // handle to semaphore
		1,            // increase count by one
		NULL);       // not interested in previous count
}

int dl = 0;
char *tabelaDoWyslania;
void dodajDoPrzeczytanych(char c) {
	//if(!czySemOpuszczony)
	WaitForSingleObject(
		semTabelaCzytania,   // handle to semaphore
		INFINITE);           // time-out interval
	if (c == '\b'&&lbackspace >= linnychznakow) {
		ReleaseSemaphore(
			semTabelaCzytania,  // handle to semaphore
			1,            // increase count by one
			NULL);       // not interested in previous count
		return;
	}
	if (c == '\b')lbackspace++;
	else linnychznakow++;
	dl = linnychznakow - lbackspace;
	/*
	if (c == '\n') {
		indexCzytania = 0;
		*tabelaCzytania = '\0';
		ReleaseSemaphore(
			semTabelaCzytania,  // handle to semaphore
			1,            // increase count by one
			NULL);       // not interested in previous count
		return;
	}*/
	if (indexCzytania >= rozmiarBufora-2) {
		char *temp = tabelaCzytania;
		int mnoznik = 2;
		tabelaCzytania = (char *)malloc(mnoznik*rozmiarBufora);
		for (int i = 0; i < indexCzytania; i++)*(tabelaCzytania + i) = *(temp + i);
		free(temp);
		rozmiarBufora *= mnoznik;
	}
	if (c == '\b') {
		indexCzytania-=2;
		*(tabelaCzytania + indexCzytania + 1) = '\0';
	}
	else {
		*(tabelaCzytania + indexCzytania) = c;
		*(tabelaCzytania + indexCzytania + 1) = '\0';
	}
	indexCzytania++;
	if (wyswietlaImie) {
		if (c == '\b') printf("\b \b");
		else
		printf("%c", c);//pod semaforem bo nie mozna doposcic do sytuacji, w ktorej najpierw zapiszemy nowy znak, potem inny fragment kodu go wyswietli (przerysuje cala tabeleczytania), a potem ten print go wyswietli drugi raz

	}
	if (c == '\n') {
		tabelaDoWyslania = (char *)malloc(strlen(tabelaCzytania) + 1);
		memcpy(tabelaDoWyslania, tabelaCzytania, strlen(tabelaCzytania) + 1);
		usunPrzeczytane(1);
	}
	ReleaseSemaphore(
		semTabelaCzytania,  // handle to semaphore
		1,            // increase count by one
		NULL);       // not interested in previous count
}
void wyswietlPrzeczytane(int ignorujSem) {
	if(!ignorujSem)
	WaitForSingleObject(
		semTabelaCzytania,   // handle to semaphore
		INFINITE);           // time-out interval
	printf("%s: ", imie);
	wyswietlaImie = 1;
	printf("%s", tabelaCzytania);//semafor jest dlatego, ¿e funkcja dodajdoprzeczytanych mog³a byæ pomiêdzy dopisaniem '\0' na koñcu i dodadkowego znaku,
	//oraz tez aby wyœwietli³a siê aktualna tabela, a nie tabela bez jednego znaku
	if (!ignorujSem)
	ReleaseSemaphore(
		semTabelaCzytania,  // handle to semaphore
		1,            // increase count by one
		NULL);       // not interested in previous count
}
/*void czytanie(void *dane) {//gdy inny uzytkownik czatu wysle wiadomosc, ktora przykryje w konsoli aktualnie pisana, mysimy miec zapamietana wiadomosc
	char c;
	while (1) {
		c = _getch();
		ungetc(c, stdin);
		dodajDoPrzeczytanych(c);
		
	}
}*/


BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	if (CEvent == CTRL_CLOSE_EVENT)
	{
		closesocket(s);
		WSACleanup();
	}
	return TRUE;
}
CONSOLE_SCREEN_BUFFER_INFO csbi;
void odbieranie(void *data) {
	char buf[11];
	buf[10] = '\0';
	for (;;)
	{
		int result;
		if (( result=recv(s, buf, 10,0))<0) {
			printf("Serwer zerwal polaczenie\n");
			break;
		}
		//buf[result] = '\0';
		/*if (buf[1] == ':') {
			int r = 1;
		}*/
		//if(dlug!=80)
		//buf[dlug] = '\0';
		//printf("\33[2K");
		
		/*WaitForSingleObject(
			semPiszeKonsole,   // handle to semaphore
			INFINITE);           // time-out interval*/
		WaitForSingleObject(
			semTabelaCzytania,   // handle to semaphore
			INFINITE);           // time-out interval
		if (wyswietlaImie) {
			//printf("\r");
			//for (int i = 0; i < strlen(imie) + 1;i++)printf(" ");
			//printf("\r");
			printf("\33[2K\r");
			wyswietlaImie = 0;

			
		}
		if (wlasnaWiadomoscNaKonsoli) {
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
			int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			for(int i=0;i<ceil((float)(dl+strlen(imie)+2)/columns);i++)
			printf("\033[A\33[2K");//usuwamy okreslona ilosc linii
			wlasnaWiadomoscNaKonsoli = 0;
			//printf("\033[A");
			//for (int i = 0; i < dl; i++) {
			//	printf("\b");
			//}
		}
		printf("%s", buf);

		int offset = 0;
		while(result > strlen(buf+offset)+1) {
			offset += strlen(buf + offset) + 1;
			printf("%s", buf + offset);
		}
		if ((buf[result - 2] == '\n' &&buf[result-1]=='\0')|| buf[result-1] == '\n'|| result==1) {
			printf("\r");
			wyswietlPrzeczytane(1);
		}
		ReleaseSemaphore(
			semTabelaCzytania,  // handle to semaphore
			1,            // increase count by one
			NULL);       // not interested in previous count
	}
}

int main() {
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	/*semPiszeKonsole = CreateSemaphore(
		NULL,           // default security attributes
		1,  // initial count
		1,  // maximum count
		NULL);          // unnamed semaphore*/
	system(" ");//wlacza kody VT100
	inicjalizacjaCzytania();
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);
	
	struct sockaddr_in sa;
	WSADATA wsas;
	WORD wersja;
	wersja = MAKEWORD(2, 0);
	WSAStartup(wersja, &wsas);
	s = socket(AF_INET, SOCK_STREAM, 0);
	memset((void *)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(8083);
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");

	int result;
	result = connect(s, (struct sockaddr FAR *) & sa, sizeof(sa));
	if (result == SOCKET_ERROR)
	{
		printf("\nBlad polaczenia!\n");
		return 0;
	}
	else {
		int dlug;
		
		printf("Podaj nazwe uzytkownika\n");
		for (;;)
		{
			int maxDlugoscNazwy = 10;
			fgets(imie, 80, stdin);
			dlug = strlen(imie); imie[dlug - 1] = '\0';
			if (dlug == 1) {
				printf("\033[A\33[2K");
				continue;
			}
			else if (dlug > maxDlugoscNazwy + 1) {
				printf("\033[A\33[2K");
				printf("\033[A\33[2K");
				printf("Maksymalna dlugosc nazwy to %d (podana: %d)\n", maxDlugoscNazwy, dlug - 1);
				continue;
			}
			int res = send(s, imie, dlug, 0);
			if (!res) {
				printf("nie udalo sie wyslac, utracono polaczenie\n");
				//break;
				goto koniec;
			}
			else if (res == SOCKET_ERROR) {
				printf("nie udalo sie wyslac, utracono polaczenie: ERROR\n");
				//break;
				goto koniec;
			}

			//if (strcmp(buf, "KONIEC") == 0) break;

			
			if (!recv(s, imie, dlug, 0)) {
				printf("Serwer zerwal polaczenie\n");
				//break;
				goto koniec;
			}
			//printf("%s\n", buf);
			break;
			
			//printf("\033[A\33[2K");

			//break;
		}
		printf("\033[A\33[2K");
		printf("\033[A\33[2K");
		printf("Witaj na czacie %s!!!!\n", imie);
		Sleep(2500);
		printf("\033[A\33[2K");
		_beginthread(odbieranie,0,NULL);
		//_beginthread(czytanie, 0, NULL);

		WaitForSingleObject(semTabelaCzytania,INFINITE);


		printf("%s: ", imie);
		wyswietlaImie = 1;
		ReleaseSemaphore(semTabelaCzytania,1,NULL);
		for (;;)
		{
			char buf[10];
			buf[9] = '\0';
			while (1) {
				char c;
				c = _getch();
				
				//printf("%c",c);
				if (c == '\r') {
					dodajDoPrzeczytanych('\n');
					//printf("\n");
					wlasnaWiadomoscNaKonsoli = 1;
					break;
				}
				else {
					dodajDoPrzeczytanych(c);
					//printf("%c", c);
					
				}
			}
			Sleep(5000);
			
			
			//fgets(buf, 10, stdin);
			
			int dlugosc = strlen(tabelaDoWyslania)+1;
			for (int i = 0; i < dlugosc; i += 9) {
				
				memcpy(buf, tabelaDoWyslania + i, min(9, dlugosc - i));
				//dlug = strlen(buf);
				//if (buf[dlug - 1] == '\n') {
				//buf[dlug - 1] = '\0';
				//}
				//if (dlug == 1) continue;
				int res = send(s, buf, /*dlug+1*/10, 0);
				if (!res) {
					printf("nie udalo sie wyslac, utracono polaczenie\n");
					break;
				}
				else if (res == SOCKET_ERROR) {
					printf("nie udalo sie wyslac, utracono polaczenie: ERROR\n");
					break;
				}

			}
			free(tabelaDoWyslania);
			//usunPrzeczytane();

			/*if (buf[dlug - 1] == '\n') {
				printf("%s: ", imie);
				wyswietlaImie = 1;
			}*/
			/*
			//if (strcmp(buf, "KONIEC") == 0) break;
			
			int dlugosc;
			if (!(dlugosc=recv(s, buf, 80, 0))) {
				printf("Serwer zerwal polaczenie\n");
				break;
			}
			//printf("%d", dlugosc);
			printf("%s\n", buf);*/
			//printf("\033[A\33[2K");
			
			//break;
		}
		koniec:
		closesocket(s);
		WSACleanup();
	}

	return 0;
}
