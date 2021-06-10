#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <iostream>
#include <clocale>
#include <fcntl.h>
#include <time.h>
#include <cmath>

#define TimeOfGame 30
#define MaxX 480
#define MaxY 330
#define Radius 35

using namespace std;

enum states { WaitingOfConnection, WaitingOfCombat, Win, Lose, Combat, DeadHeat };
enum Msg_type { result_of_shot, state_for_client, coor_for_target, score_of_enemy };
enum ResultOfShot { not_hit, hit };

struct ResultOfBung
{
	int16_t type = static_cast<int16_t>(result_of_shot);
	int16_t result;
	int16_t ScoreOfPlayer;
};

struct ScoreOfEnemy
{
	int16_t type = static_cast<int16_t>(score_of_enemy);
	int16_t Score;
};

struct Message
{
	int16_t type = static_cast<int16_t>(coor_for_target);
	int16_t PosX;
	int16_t PosY;
};

struct Shot
{
	int16_t PosX;
	int16_t PosY;
};

struct StateForClient
{
	int16_t type = static_cast<int16_t>(state_for_client);
	int16_t state;
};

long start;
int FirstPlayer, SecondPlayer;
int ScoreOfFP = 0, ScoreOfSP = 0;
states stateOfFirstPlayer = WaitingOfConnection, stateOfSecondPlayer = WaitingOfConnection;

struct Threads
{
	pthread_t* firstThread;
	pthread_t* secondThread;
};

//Поток, в котором происходит обработка игрового времени и результатов игры. Параметр необходим для корректного функционирвоания потока и ничего не содержит.
void* Timer(void* AllThreads)
{
	Threads* GameThreads = static_cast<Threads*>(AllThreads);

	while (time(NULL) <= start + TimeOfGame);

	cout << "Время вышло" << endl;

	pthread_cancel(*GameThreads->firstThread);
	pthread_cancel(*GameThreads->secondThread);

	sleep(3);

	StateForClient stateForFP, stateForSP;

	if (ScoreOfFP > ScoreOfSP)
	{
		stateForFP.state = static_cast<int16_t>(Win);
		stateForSP.state = static_cast<int16_t>(Lose);
	}
	else if (ScoreOfFP < ScoreOfSP)
	{
		stateForFP.state = static_cast<int16_t>(Lose);
		stateForSP.state = static_cast<int16_t>(Win);
	}
	else if (ScoreOfFP == ScoreOfSP)
	{
		stateForFP.state = stateForSP.state = static_cast<int16_t>(DeadHeat);
	}

	send(FirstPlayer, &stateForFP, sizeof(StateForClient), MSG_NOSIGNAL);
	send(SecondPlayer, &stateForSP, sizeof(StateForClient), MSG_NOSIGNAL);

	pthread_exit(0);
}

//Поток, в котором происходит обработка входящих данных от первого клиента. Параметр необходим для корректного функционирования потока и ничего не содержит.
void* DataFromFirstClient(void* NullData)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	StateForClient State;
	State.state = static_cast<int16_t>(Combat);
	send(FirstPlayer, &State, sizeof(State), MSG_NOSIGNAL);

	while (1)
	{
		cout << "ScoreFP = " << ScoreOfFP << endl;
		Message CoorForTarget;

		CoorForTarget.PosX = rand() % MaxX;
		CoorForTarget.PosY = rand() % MaxY;

		cout << "x1 = " << CoorForTarget.PosX << ", y1 = " << CoorForTarget.PosY << endl;

		send(FirstPlayer, &CoorForTarget, sizeof(CoorForTarget), MSG_NOSIGNAL);

		Shot PlayerShot;
		recv(FirstPlayer, &PlayerShot, sizeof(Shot), MSG_NOSIGNAL);

		cout << "X1 = " << PlayerShot.PosX << ", Y1 = " << PlayerShot.PosY << endl;

		int x2 = (PlayerShot.PosX - (CoorForTarget.PosX + Radius)) * (PlayerShot.PosX - (CoorForTarget.PosX + Radius));
		int y2 = (PlayerShot.PosY - (CoorForTarget.PosY + Radius)) * (PlayerShot.PosY - (CoorForTarget.PosY + Radius));

		ResultOfBung Result;

		if (sqrt(x2 + y2) < Radius)
		{
			++ScoreOfFP;
			Result.ScoreOfPlayer = ScoreOfFP;
			Result.result = static_cast<int16_t>(hit);
			send(FirstPlayer, &Result, sizeof(ResultOfBung), MSG_NOSIGNAL);

			ScoreOfEnemy score;
			score.Score = ScoreOfFP;
			send(SecondPlayer, &score, sizeof(ScoreOfEnemy), MSG_NOSIGNAL);

		}
		else
		{
			Result.result = static_cast<int16_t>(not_hit);
			send(FirstPlayer, &Result, sizeof(ResultOfBung), MSG_NOSIGNAL);
		}
	}
}

//Поток, в котором происходит обработка входящих данных от второго клиента. Параметр необходим для корректного функционирования потока и ничего не содержит.
void* DataFromSecondClient(void* NullData)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	StateForClient State;
	State.state = static_cast<int16_t>(Combat);
	send(SecondPlayer, &State, sizeof(State), MSG_NOSIGNAL);

	while (1)
	{
		cout << "ScoreSP = " << ScoreOfSP << endl;
		Message CoorForTarget;

		CoorForTarget.PosX = rand() % MaxX;
		CoorForTarget.PosY = rand() % MaxY;

		send(SecondPlayer, &CoorForTarget, sizeof(CoorForTarget), MSG_NOSIGNAL);

		cout << "x2 = " << CoorForTarget.PosX << ", y2 = " << CoorForTarget.PosY << endl;

		Shot PlayerShot;
		recv(SecondPlayer, &PlayerShot, sizeof(Shot), MSG_NOSIGNAL);

		cout << "X2 = " << PlayerShot.PosX << ", Y2 = " << PlayerShot.PosY << endl;

		int x2 = (PlayerShot.PosX - (CoorForTarget.PosX + Radius)) * (PlayerShot.PosX - (CoorForTarget.PosX + Radius));
		int y2 = (PlayerShot.PosY - (CoorForTarget.PosY + Radius)) * (PlayerShot.PosY - (CoorForTarget.PosY + Radius));

		ResultOfBung Result;

		if (sqrt(x2 + y2) < Radius)
		{
			++ScoreOfSP;
			Result.result = static_cast<int16_t>(hit);
			Result.ScoreOfPlayer = ScoreOfSP;
			send(SecondPlayer, &Result, sizeof(ResultOfBung), MSG_NOSIGNAL);

			ScoreOfEnemy score;
			score.Score = ScoreOfSP;
			send(FirstPlayer, &score, sizeof(ScoreOfEnemy), MSG_NOSIGNAL);
		}
		else
		{
			Result.result = static_cast<int16_t>(not_hit);
			send(SecondPlayer, &Result, sizeof(ResultOfBung), MSG_NOSIGNAL);
		}
	}
}

int main()
{
	setlocale(LC_ALL, "Russian");

	StateForClient stateFP, stateSP;

	//---------------Создание слушающего сокета---------------------
	int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in MasterAddr;
	MasterAddr.sin_family = AF_INET;
	MasterAddr.sin_port = htons(25678);
	MasterAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(MasterSocket, (struct sockaddr*) (&MasterAddr), sizeof(MasterAddr));

	listen(MasterSocket, 1);
	//-------------------------------------------------------------

	//----------------Соединение клиентов---------------------

	FirstPlayer = accept(MasterSocket, 0, 0);

	if (FirstPlayer > 0)
	{
		printf("Первый игрок подключился!\n");

		stateFP.state = static_cast<int16_t>(stateOfFirstPlayer);
		send(FirstPlayer, &stateFP, sizeof(stateFP), MSG_NOSIGNAL);
	}
	else
	{
		printf("Соединение провалено!\n");
		return 0;
	}

	cout << "После подключения первого игрока: " << stateOfFirstPlayer << " " << stateOfSecondPlayer << "\n";

	SecondPlayer = accept(MasterSocket, 0, 0);

	if (SecondPlayer > 0)
	{
		printf("Второй игрок подключился!\n");

		stateOfFirstPlayer = stateOfSecondPlayer = WaitingOfCombat;

		stateFP.state = stateOfFirstPlayer;
		stateSP.state = stateOfSecondPlayer;

		send(FirstPlayer, &stateFP, sizeof(stateFP), MSG_NOSIGNAL);
		send(SecondPlayer, &stateSP, sizeof(stateSP), MSG_NOSIGNAL);
	}
	else
	{
		printf("Соединение провалено!\n");
		return 0;
	}
	//------------------------------------------------------

	cout << "После подключения всех игроков: " << stateOfFirstPlayer << " " << stateOfSecondPlayer << "\n";

	//----------------------Процесс игры----------------------
	void* Null = NULL;

	sleep(3);
	srand(time(NULL));

	//создание переменных для потоков
	pthread_t FirstThread, SecondThread, TimerThread;

	Threads MyThreads;

	MyThreads.firstThread = &FirstThread;
	MyThreads.secondThread = &SecondThread;

	start = time(NULL);

	//создание потоков
	pthread_create(&FirstThread, 0, DataFromFirstClient, Null);
	pthread_create(&SecondThread, 0, DataFromSecondClient, Null);
	pthread_create(&TimerThread, 0, Timer, &MyThreads);

	//Завершение потоков
	pthread_join(TimerThread, 0);
	//--------------------------------------------------------

	shutdown(MasterSocket, SHUT_RDWR);
	close(MasterSocket);
	cout << "Игра завершилась" << endl;

	return 0;
}
