
#include <QFile>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sensor.h"
#include "axle.h"
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QDateTime>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isBlocked(false)
{
    ui->setupUi(this);

    // Инициализация графической сцены
    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);


    QAction* exitAction = new QAction("Выход", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    ui->menuFile->addAction(exitAction);


    // Настройка таблицы
    initSensorTable();

    // Создание и настройка системы
    setupSystem();

    // Таймер для обновления визуализации
    QTimer *visualizationTimer = new QTimer(this);
    connect(visualizationTimer, &QTimer::timeout, this, &MainWindow::updateVisualization);
    visualizationTimer->start(); //(50) 20 FPS

    // Подключение кнопок
    connect(ui->blockButton, &QPushButton::clicked, this, &MainWindow::onBlockButtonClicked);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::onResetButtonClicked);
    static float speed;
}

void MainWindow::setupSystem()
{
    // Создаем 5 датчиков с указанными позициями

    QSettings *settings;

    settings = new QSettings("config/settings.ini", QSettings::IniFormat);
    settings->beginGroup("distances");
    float d12 = settings->value("d12", 31.50).toFloat();
    float d23 = settings->value("d23", 3.20).toFloat();
    float d34 = settings->value("d34", 0.60).toFloat();
    float d45 = settings->value("d45", 35.10).toFloat();

    // Создаем датчики
    for (int i = 1; i <= 5; ++i) {
        sensors.append(new Sensor(i, this));
    }

    // Устанавливаем позиции датчиков
    float x = 50.0;
    qDebug() << "d1=" << x;
    sensors[0]->setPosition(QPointF(x, 150));
    x += d12;
    qDebug() << "d12="<< x;
    sensors[1]->setPosition(QPointF(x, 150));
    x += d23;
    qDebug() << "d23="<< x;
    sensors[2]->setPosition(QPointF(x, 150));
    x += d34;
    qDebug() << "d34="<< x;
    sensors[3]->setPosition(QPointF(x, 150));
    x += d45;
    qDebug() << "d45="<< x;
    sensors[4]->setPosition(QPointF(x, 150));

    // Инициализируем колесную формулу ЧМЭ3
    //QList<float> wheelFormula = {2.0f, 2.0f, 4.66f, 2.0f, 2.0f}; // В метрах
    float currentPos = 30.0f; // Начальная позиция

    settings->beginGroup("locomotive");
    QStringList wheelFormulaStr = settings->value("wheel_formula", "2.0,2.0,4.66,2.0,2.0").toString().split(',', Qt::SkipEmptyParts);

    QList<float> wheelFormula;
    for (const QString& val : wheelFormulaStr) {
        wheelFormula.append(val.trimmed().toFloat());
    }

    // Устанавливаем начальную скорость м/с
    float speed = settings->value("speed", 1.0f).toFloat();   //, 1.0f
    qDebug() <<"speed = "<< speed;

    for (int i = 0; i < wheelFormula.size(); ++i) {
        Axle *axle = new Axle(i, this);
        axle->setPosition(currentPos);
        axle->setVelocity(speed); // Скорость 1 м/с
        axles.append(axle);
        currentPos += wheelFormula[i];
        qDebug()<< axle->getNumber() <<"currentPos="<<currentPos;
        qDebug()<< axle->getVelocity();
    }

    // Регистрируем датчики для всех осей
    for (Axle* axle : axles) {
        for (Sensor* sensor : sensors) {
            axle->registerSensor(sensor);
        //    qDebug() <<"sensor="<< sensor->getId();
            qDebug() <<"axle="<< axle->getNumber()<<"speed_:"<< axle->getVelocity();
            connect(axle, &Axle::passedSensor, sensor, &Sensor::handleAxlePass);
        }
        connect(axle, &Axle::passedSensor, this, &MainWindow::handleSensorData);
    }
}

void MainWindow::initSensorTable()
{
    ui->sensorTable->setColumnCount(5);
    QStringList headers;
    headers << "Датчик" << "Скорость (м/с)" << "Время (мс)" << "Номер оси" << "Расстояние (м)";
    ui->sensorTable->setHorizontalHeaderLabels(headers);
    ui->sensorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->sensorTable->setSortingEnabled(1);
}

void MainWindow::handleSensorData(Sensor* sensor, float velocity, qint64 timeSinceLast, int axleNum, float distance)
{
    // Только если датчик есть в нашей системе
    if (!sensors.contains(sensor)) return;

    // Добавляем новую строку в таблицу
    int row = ui->sensorTable->rowCount();
    ui->sensorTable->insertRow(row);
    float distance_1 = (distance * timeSinceLast)/100;

    // Заполняем данные
    ui->sensorTable->setItem(row, 0, new QTableWidgetItem(QString::number(sensor->getId())));
    ui->sensorTable->setItem(row, 1, new QTableWidgetItem(QString::number(velocity, 'f', 2)));
    ui->sensorTable->setItem(row, 2, new QTableWidgetItem(QString::number(timeSinceLast)));
    ui->sensorTable->setItem(row, 3, new QTableWidgetItem(QString::number(axleNum)));
    ui->sensorTable->setItem(row, 4, new QTableWidgetItem(QString::number(distance_1, 'f', 2)));

    // Прокрутка к новой записи
    ui->sensorTable->scrollToBottom();
}

void MainWindow::updateVisualization()
{
    QGraphicsScene *scene = ui->graphicsView->scene();
    scene->clear();

    // Отрисовка пути
    QGraphicsLineItem *track = new QGraphicsLineItem(0, 150, 1000, 150);
    track->setPen(QPen(Qt::red, 3));
    scene->addItem(track);

    // Отрисовка датчиков
    for (Sensor* sensor : sensors) {
        QPolygonF triangle;
        triangle << QPointF(0, 0) << QPointF(15, 30) << QPointF(-15, 30);
        QGraphicsPolygonItem *sensorItem = new QGraphicsPolygonItem(triangle);
        sensorItem->setPos(sensor->getPosition().x(), 150);
        sensorItem->setBrush(Qt::yellow);
        scene->addItem(sensorItem);

        // Подпись датчика
        QGraphicsTextItem *label = new QGraphicsTextItem(QString::number(sensor->getId()));
        label->setPos(sensor->getPosition().x() - 5, 120);
        scene->addItem(label);
    }

    // Отрисовка осей
    for (Axle* axle : axles) {
        QGraphicsEllipseItem *axleItem = new QGraphicsEllipseItem(-8, -8, 16, 16);
        axleItem->setPos(axle->getPosition(), 150);
        axleItem->setBrush(axle->getNumber() % 2 == 0 ? Qt::red : Qt::green);
        scene->addItem(axleItem);

        // Подпись номера оси
        QGraphicsTextItem *label = new QGraphicsTextItem(QString::number(axle->getNumber()));
        label->setPos(axle->getPosition() - 5, 170);
        scene->addItem(label);
    }
}

void MainWindow::setAxlesVelocity(float velocity)
{
    for (Axle* axle : axles) {
        axle->setVelocity(isBlocked ? 0.0f : velocity);
    }
}

void MainWindow::onBlockButtonClicked()
{
    isBlocked = !isBlocked;
    ui->blockButton->setText(isBlocked ? "Разблокировать" : "Блокировка");
    QSettings settings("config/settings.ini", QSettings::IniFormat);
    settings.beginGroup("locomotive");
    float speed = settings.value("locomotive/speed", 1.0f).toFloat();
    setAxlesVelocity(isBlocked ? 0.0f : speed);
}

void MainWindow::onResetButtonClicked()
{
    // Сброс позиций осей
    QSettings settings("config/settings.ini", QSettings::IniFormat);
    QStringList wheelFormulaStr = settings.value("locomotive/wheel_formula", "2.0,2.0,4.66,2.0,2.0")
                                      .toString().split(',', Qt::SkipEmptyParts);

    QList<float> wheelFormula;
    for (const QString& val : wheelFormulaStr) {
        wheelFormula.append(val.trimmed().toFloat());
    }

    float currentPos = 30;
    for (int i = 0; i < axles.size(); ++i) {
        currentPos += wheelFormula[i];
        axles[i]->setPosition(currentPos);
        axles[i]->resetPosition(currentPos);
    }


    // Очистка таблицы
    ui->sensorTable->setRowCount(0);
}

MainWindow::~MainWindow()
{
    qDeleteAll(sensors);
    qDeleteAll(axles);
    delete ui;
}

void MainWindow::on_action_changed()
{
    connect(ui->action, &QAction::triggered, this, &QApplication::quit);
}

