#include "mainwindow.h"
#include "ui_mainwindow.h"

#define WidthOfFrame 710
#define HeightOfFrame 430

//контруктор класса (описанее интерфейса игры)
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->MainPicture->setFixedSize(QSize(WidthOfFrame, HeightOfFrame));
	setFixedSize(QSize(WidthOfFrame, HeightOfFrame));
	ui->Frame->setFixedSize(QSize(WidthOfFrame, HeightOfFrame));
	ui->Frame->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->Frame->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->ScorePlayers->hide();

	scene = new QGraphicsScene;
	MyTimer = new QTimer;
	Timer = new QTimer;
	MyTimer->setInterval(50);
}

//Спрятать меню
void MainWindow::Main_Menu_off()
{
	ui->MainPicture->hide();
	ui->InputLine->hide();
	ui->Connection->hide();
}

MainWindow::~MainWindow()
{
	delete ui;
}

//Слот, предназначенный для подсоединения клиента к серверу.
void MainWindow::on_Connection_clicked()
{
	QString IPaddress = ui->InputLine->text();

	if (IPaddress == NULL)
	{
		QMessageBox::information(this, "Ошибка", "Адрес сервера не был введён");
	}
	else
	{
		QByteArray tmp = IPaddress.toLatin1();
		const char* IP = tmp.data();

		ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		struct sockaddr_in ServAddr;

		ServAddr.sin_family = AF_INET;
		ServAddr.sin_port = htons(25678);
		inet_pton(AF_INET, IP, &(ServAddr.sin_addr));

		if (::connect(ClientSocket, (struct sockaddr*) &ServAddr, sizeof(ServAddr)) != -1)
		{
			connect(MyTimer, &QTimer::timeout, this, &MainWindow::ReadFromServer);
			fcntl(ClientSocket, F_SETFL, O_NONBLOCK);
			MyTimer->start();
		}
		else
		{
			QMessageBox::information(this, "Ошибка", "Введён неправильный адрес или сервер не запущен");
		}
	}
}

//Слот, предназначенный для отправки координаты выстрела на сервер для его дальнейшей обработки. Первый параметр – позиция выстрела на оси абсцисс, второй параметр – позиция выстрела по оси ординат.
void MainWindow::SendFire(int16_t x, int16_t y)
{
	Shot ShotForServer;

	ShotForServer.PosX = x;
	ShotForServer.PosY = y;

	send(ClientSocket, &ShotForServer, sizeof(Shot), MSG_NOSIGNAL); //отправляем на сервер координаты выстрела
}

//Слот, необходимый для обработки таймера, в течении которого идет игра.
void MainWindow::TimerOfGame()
{
	--myTimer;
	qDebug() << "Осталось " << myTimer;

	if (myTimer >= 0)
	{
		ui->Timer->setText("Время: " + QString::number(myTimer));

		if (myTimer == 0)
		{
			delete Timer;
		}
	}
}

//Метод, предназначенный для считывания данных, отправленных с сервера, с сокета клиента.
void MainWindow::ReadFromServer()
{
	int16_t buffer[3];

	if (recv(ClientSocket, &buffer, sizeof(int16_t), MSG_NOSIGNAL) > 0)
	{
		Msg_type type = static_cast<Msg_type>(buffer[0]);

		qDebug() << "type = " << type;

		if (type == state_for_client)
		{
			recv(ClientSocket, &buffer[1], sizeof(int16_t), MSG_NOSIGNAL);

			States state = static_cast<States>(buffer[1]);

			qDebug() << "state = " << state;
			if (state == WaitingOfConnection)
			{
				Main_Menu_off();

				QPixmap frame;

				frame.load(":/img/WaitingOfConnection.jpg");
				frame = frame.scaled(WidthOfFrame, HeightOfFrame);
				scene->addPixmap(frame);

				ui->Frame->setScene(scene);
				ui->Frame->show();
			}
			else if (state == WaitingOfCombat)
			{
				Main_Menu_off();

				QPixmap frame;

				frame.load(":/img/WaitingOfCombat.jpg");
				frame = frame.scaled(WidthOfFrame, HeightOfFrame);
				scene->addPixmap(frame);

				ui->Frame->setScene(scene);
				ui->Frame->show();
			}
			else if (state == Combat)
			{
				qDebug() << "It's a BATTLE, baby!\n";
				QPixmap frame;

				frame.load(":/img/BattleArea.jpg");
				frame = frame.scaled(580, HeightOfFrame);
				scene->addPixmap(frame);

				AreaForShot* area = new AreaForShot;
				area->setPos(0, 0);
				connect(area, &AreaForShot::fire, this, &MainWindow::SendFire);
				scene->addItem(area);

				ui->Frame->setFixedSize(QSize(580, HeightOfFrame));
				ui->Frame->setScene(scene);
				ui->Frame->show();

				connect(Timer, &QTimer::timeout, this, &MainWindow::TimerOfGame);
				Timer->setInterval(1000);
				Timer->start();

				ui->Timer->setText("Время: 30");
				ui->ScorePlayers->show();
				ui->Player1->setText("Мои: 0");
				ui->Player2->setText("Противника: 0");
			}
			else if (state == Win)
			{
				QPixmap win;
				win.load(":/img/Win.jpg");
				win = win.scaled(WidthOfFrame, HeightOfFrame);

				scene->clear();
				scene->addPixmap(win);
				ui->Frame->setScene(scene);
				ui->Frame->setFixedSize(WidthOfFrame, HeightOfFrame);
				ui->Frame->show();

				ui->Timer->hide();
				ui->ScorePlayers->hide();
				ui->Player1->hide();
				ui->Player2->hide();

				delete MyTimer;

				shutdown(ClientSocket, SHUT_RDWR);
				::close(ClientSocket);

				qDebug() << "Я выиграл";
			}
			else if (state == Lose)
			{
				QPixmap lose;
				lose.load(":/img/Lose.jpg");
				lose = lose.scaled(WidthOfFrame, HeightOfFrame);

				scene->clear();
				scene->addPixmap(lose);
				ui->Frame->setScene(scene);
				ui->Frame->setFixedSize(WidthOfFrame, HeightOfFrame);
				ui->Frame->show();

				ui->Timer->hide();
				ui->ScorePlayers->hide();
				ui->Player1->hide();
				ui->Player2->hide();

				delete MyTimer;

				shutdown(ClientSocket, SHUT_RDWR);
				::close(ClientSocket);

				qDebug() << "Блiн, проiграл";
			}
			else if (state == DeadHeat)
			{
				QPixmap Draw;
				Draw.load(":/img/DeadHeat.jpg");
				Draw = Draw.scaled(WidthOfFrame, HeightOfFrame);

				scene->clear();
				scene->addPixmap(Draw);
				ui->Frame->setScene(scene);
				ui->Frame->setFixedSize(WidthOfFrame, HeightOfFrame);
				ui->Frame->show();

				ui->Timer->hide();
				ui->ScorePlayers->hide();
				ui->Player1->hide();
				ui->Player2->hide();

				delete MyTimer;

				shutdown(ClientSocket, SHUT_RDWR);
				::close(ClientSocket);

				qDebug() << "Ничья";
			}
		}
		else if (type == result_of_shot)
		{
			recv(ClientSocket, &buffer[1], 2 * sizeof(int16_t), MSG_NOSIGNAL);

			ResultOfShot result = static_cast<ResultOfShot>(buffer[1]);

			if (result == hit)
			{
				qDebug() << "Я попал!";

				ui->Player1->setText("Мои: " + QString::number(buffer[2]));
			}
			else if (result == not_hit)
			{
				qDebug() << "Молоко!";
			}

			delete Target;
		}
		else if (type == coor_of_taget)
		{
			recv(ClientSocket, &buffer[1], 2 * sizeof(int16_t), MSG_NOSIGNAL);

			qDebug() << "PosX = " << buffer[1];
			qDebug() << "PosY = " << buffer[2];

			QPixmap target;

			target.load(":/img/Target.jpg");
			target = target.scaled(70, 70);

			Target = scene->addPixmap(target);

			Target->setPos(buffer[1], buffer[2]);
		}
		else if (type == score_of_enemy)
		{
			recv(ClientSocket, &buffer[1], sizeof(int16_t), MSG_NOSIGNAL);
			ui->Player2->setText("Противника: " + QString::number(buffer[1]));
		}
	}
}
