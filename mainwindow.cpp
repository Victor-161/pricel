
#include <QFile>
#include <QSettings>
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
#include <QMessageBox>
#include <QTextStream>

//#define  koef_M     5.0


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isBlocked(false)
{
    ui->setupUi(this);

    // Создание и настройка системы
    setupSystem();

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

    // Подключение кнопок старта и метода остановки таймера
    for (Axle* axle : axles) {
            connect(ui->startButton, &QPushButton::clicked, axle, &Axle::startTimer);
            connect(ui->resetButton, &QPushButton::clicked, axle, &Axle::stopTimer);
    }

    // Таймер для обновления визуализации
    QTimer *visualizationTimer = new QTimer(this);
    connect(visualizationTimer, &QTimer::timeout, this, &MainWindow::updateVisualization);
    visualizationTimer->start(); //(50) 20 FPS

    // Подключение кнопок блокировки и сброса
    connect(ui->blockButton, &QPushButton::clicked, this, &MainWindow::onBlockButtonClicked);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::onResetButtonClicked);
    connect(ui->actionLoadConfig, &QAction::triggered, this, &MainWindow::on_actionLoadConfig);


}

void MainWindow::setupSystem()
{
    // Путь к конфигу в папке с исполняемым файлом
    QSettings settings("config/settings.ini", QSettings::IniFormat);

    float d1 = settings.value("distances/d1", 20).toFloat();
    float d12 = settings.value("distances/d12", 31.50).toFloat();
    float d23 = settings.value("distances/d23", 3.20).toFloat();
    float d34 = settings.value("distances/d34", 0.60).toFloat();
    float d45 = settings.value("distances/d45", 35.10).toFloat();

    if (!settings.contains("distances/d1")) {
        qWarning() << "Датчики установлены по умолчанию";
    }
    else qWarning() << "Датчики установлены в соответствии с файлом конфигурации";

    // Создаем датчики
    for (int i = 1; i <= 5; ++i) {
        sensors.append(new Sensor(i, this));
    }

    // Устанавливаем позиции датчиков
    float x = d1 * koef_M;
    qDebug() << "d1=" << x;
    sensors[0]->setPosition(QPointF(x, 150));
    x += d12 * koef_M;
    qDebug() << "d2="<< x;
    sensors[1]->setPosition(QPointF(x, 150));
    x += d23 * koef_M;
    qDebug() << "d3="<< x;
    sensors[2]->setPosition(QPointF(x, 150));
    x += d34 * koef_M;
    qDebug() << "d4="<< x;
    sensors[3]->setPosition(QPointF(x, 150));
    x += d45 * koef_M;
    qDebug() << "d5="<< x;
    sensors[4]->setPosition(QPointF(x, 150));


    // Устанавливаем начальную скорость м/с
    if (!settings.contains("locomotive/speed")) {
        qWarning() << "Cкорости локомотива установлена по умолчанию";
    }
    else qWarning() << "Скорость локомотива установлена в соответствии с файлом конфигурации";

    float speed = settings.value("locomotive/speed", 1.00f).toFloat();
    qDebug() <<"speed = "<< speed;

    QStringList wheelFormulaStr = settings.value("locomotive/wheel_formula", "2.0,2.0,4.66,2.0,2.0").toString().split(',', Qt::SkipEmptyParts);
    QList<float> wheelFormula;
    for (const QString& val : wheelFormulaStr) {
        wheelFormula.append(val.trimmed().toFloat());
    }


        // Инициализируем колесную формулу ЧМЭ3
    if (!settings.contains("locomotive/wheel_formula")) {
        qWarning() << "Позиции осей локомотива установлены по умолчанию";
    }
    else qWarning() << "Позиции осей локомотива установлены в соответствии с файлом конфигурации";

    float currentPos = 30.0f; // Начальная позиция
    for (int i = 0; i < wheelFormula.size()+1; ++i) {
        Axle *axle = new Axle(i, this);
        axle->setPosition(currentPos);
        axle->setVelocity(speed * koef_M); // Скорость 1 м/с
        axles.append(axle);
        qDebug()<<"Позиция оси №"<< axle->getNumber()<<"=" <<currentPos;
        currentPos += (wheelFormula[i] * koef_M);
    }

    // Регистрируем датчики для всех осей
    for (Axle* axle : axles) {
        for (Sensor* sensor : sensors) {
            axle->registerSensor(sensor);
//            qDebug() <<"axle="<< axle->getNumber()<<"speed_:"<< axle->getVelocity();
            connect(axle, &Axle::passedSensor, sensor, &Sensor::handleAxlePass);
        }
        connect(axle, &Axle::passedSensor, this, &MainWindow::handleSensorData);
    }
}

void MainWindow::raschetKolBas(){

    QMap<int, QVector<float>> axleDistances;

    for (size_t i = 0; i < sensorId_t.size(); ++i){
        float distance;
        if(timePrev_t[i] < 65535){
            distance = (velocity_t[i] * timePrev_t[i]) /1000;
            axleDistances[axleNum_t[i]].append(distance);}
    }


    // Рассчитываем средние расстояния
    QMap<int, float> avgDistances;
    QList<int> axles = axleDistances.keys();
    std::sort(axles.begin(), axles.end());

    for (int i = 0; i < axles.size(); ++i) {
        float sum = 0;
        int count = 0;

        for (float dist : axleDistances[axles[i]]) {
            sum += dist;
            count++;
        }

        if (count > 0) {
            avgDistances[axles[i]] = sum / count;
//            qDebug()<<"avgDistances = "<< avgDistances[axles[i]];
        }
    }

    // Формируем колесную формулу
    QString wheelFormula = "\"";
    for (int i = 0; i < axles.size(); ++i) {
        wheelFormula += QString::number(avgDistances[axles[i]], 'f', 2);
        if (i < axles.size() - 1) wheelFormula += ", ";
    }
    wheelFormula += '"';
    qDebug() <<"wheel_Formula_ras = "<<wheelFormula;


    // Формируем полный отчет
    QString report = "Результаты расчета колесной формулы\n";
    report += "================================\n";
    report += "Дата расчета: " + QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss") + "\n";

    report += "wheelFormula = " + wheelFormula + "\n";

    // Сохраняем в файл
    QString filePath = QCoreApplication::applicationDirPath() + "/wheel_formula_ras.txt";
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << report;
        file.close();

        // Показываем сообщение пользователю
        QString message = "Результаты сохранены в файл:\n" + filePath + "\n\n";
        message += "Колесная формула: " + wheelFormula;
        QMessageBox::information(this, "Результаты расчета", message);

        statusBar()->showMessage("Файл с результатами сохранен", 3000);
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл с результатами");
    }

}

void MainWindow::initSensorTable()
{
    ui->sensorTable->setColumnCount(5);
    QStringList headers;
    headers << "Датчик" << "Скорость (м/с)" << "Время (мс)" << "Номер оси" << "Расстояние (м)";
    ui->sensorTable->setHorizontalHeaderLabels(headers);
    ui->sensorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//    ui->sensorTable->setSortingEnabled(1);
}

void MainWindow::handleSensorData(Sensor* sensor, float velocity, qint64 timeSinceLast, int axleNum, float distance)
{
    // Только если датчик есть в нашей системе
    if (!sensors.contains(sensor)) return;

    // Добавляем новую строку в таблицу
    int row = ui->sensorTable->rowCount();
    ui->sensorTable->insertRow(row);

    // Заполняем данные

    if (isLoadDataActive==0){
        ui->sensorTable->setItem(row, 0, new QTableWidgetItem(QString::number(sensor->getId())));
        ui->sensorTable->setItem(row, 1, new QTableWidgetItem(QString::number(sensor->getLastVelocity(), 'f', 2)));
        ui->sensorTable->setItem(row, 2, new QTableWidgetItem(QString::number(sensor->getLastTimeSinceLast())));
        ui->sensorTable->setItem(row, 3, new QTableWidgetItem(QString::number(sensor->getLastAxleNum())));
        ui->sensorTable->setItem(row, 4, new QTableWidgetItem(QString::number(sensor->getDistanceToNextAxle(), 'f', 2)));
    }
    else{   // вывод тестовых данных
        for (size_t i = 0; i < sensorId_t.size(); ++i){
            if(sensorId_t[i] == sensor->getId()){
                if(axleNum_t[i] == sensor->getLastAxleNum()){

                    float distance_t=0;
                    if(timePrev_t[i] < 65535){
                        distance_t = (velocity_t[i] * timePrev_t[i]) /1000;}

                    ui->sensorTable->setItem(row, 0, new QTableWidgetItem(QString::number(sensorId_t[i])));
                    ui->sensorTable->setItem(row, 1, new QTableWidgetItem(QString::number(velocity_t[i])));
                    ui->sensorTable->setItem(row, 2, new QTableWidgetItem(QString::number(timePrev_t[i])));
                    ui->sensorTable->setItem(row, 3, new QTableWidgetItem(QString::number(axleNum_t[i])));
                    ui->sensorTable->setItem(row, 4, new QTableWidgetItem(QString::number(distance_t, 'f', 2)));
                }
            }
        }
    }
    // Прокрутка к новой записи
    ui->sensorTable->scrollToBottom();
}

void MainWindow::updateVisualization()
{
    QGraphicsScene *scene = ui->graphicsView->scene();
    scene->clear();

    // Отрисовка пути
    QGraphicsLineItem *track = new QGraphicsLineItem(0, 150, 990, 150);
    track->setPen(QPen(Qt::red, 3));
    scene->addItem(track);

    // Отрисовка датчиков
    for (Sensor* sensor : sensors) {
        QPolygonF triangle;
        triangle << QPointF(0, 0) << QPointF(5, 10) << QPointF(-5, 10);
        QGraphicsPolygonItem *sensorItem = new QGraphicsPolygonItem(triangle);
        sensorItem->setPos(sensor->getPosition().x(), 150);
        sensorItem->setBrush(Qt::yellow);
        scene->addItem(sensorItem);

        // Подпись датчика
        QGraphicsTextItem *label = new QGraphicsTextItem(QString::number(sensor->getId()));
        label->setPos(sensor->getPosition().x()-5, 165);
        scene->addItem(label);
    }

    // Отрисовка осей
    for (Axle* axle : axles) {
        QGraphicsEllipseItem *axleItem = new QGraphicsEllipseItem(-4, -4, 8, 8);
        axleItem->setPos(axle->getPosition(), 140);
        axleItem->setBrush(axle->getNumber() % 2 == 0 ? Qt::red : Qt::green);
        scene->addItem(axleItem);

        // Подпись номера оси
        QGraphicsTextItem *label = new QGraphicsTextItem(QString::number(axle->getNumber()));
        label->setPos(axle->getPosition() - 10, 110);
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
    float speed = settings.value("locomotive/speed", 1.00f).toFloat();
    speed = speed * koef_M;
//    qDebug() <<"speed = "<< speed;

    setAxlesVelocity(isBlocked ? 0.0f : speed);
    for(Axle* axle : axles)       {
        axle->setLastIsBlocked(true);
//      qDebug() <<"lastIsBlocked="<<isBlocked;
    }

}

void MainWindow::onResetButtonClicked()
{
    // Сброс позиций осей
    QSettings settings("config/settings.ini", QSettings::IniFormat);
    QStringList wheelFormulaStr = settings.value("locomotive/wheel_formula", "2.0,2.0,4.66,2.0,2.0").toString().split(',', Qt::SkipEmptyParts);

    QList<float> wheelFormula;
    for (const QString& val : wheelFormulaStr) {
        wheelFormula.append(val.trimmed().toFloat());
    }

    float currentPos = 30;
    for (int i = 0; i < wheelFormula.size()+1; ++i) {

        axles[i]->setPosition(currentPos);
        axles[i]->resetPosition(currentPos);
        currentPos += wheelFormula[i] * koef_M;
        if(isBlocked == 1) onBlockButtonClicked();
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




void MainWindow::on_loadDataButton_clicked()
{
    if(isLoadDataActive) isLoadDataActive=0;
    else isLoadDataActive = 1;

    ui->loadDataButton->setStyleSheet(isLoadDataActive ? "background-color: lightgreen;" : "");



    if(sensorId_t.empty()){
        QSettings settings("config/settings.ini", QSettings::IniFormat);
        QString filePath = settings.value("path/filePath", QCoreApplication::applicationDirPath() + "/test_data.txt").toString();

        if (!settings.contains("path/filePath")) {
            qWarning() << "Значение Path установленo по умолчанию, applicationDir + /test_data.txt";
        }
        else qWarning() << "Значение Path установленo в соответствии с файлом конфигурации";
        qDebug() <<"filePath = " <<filePath;

        QFile file(filePath);

        // Проверка существования файла
        if (!QFile::exists(filePath)) {
            QMessageBox::warning(this, "Ошибка", "Файл test_data.txt не найден.\n"
                                                 "Разместите файл в папке: " + QCoreApplication::applicationDirPath());
            isLoadDataActive = false;
            ui->loadDataButton->setStyleSheet("");                return;
        }


        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("[ID=")) {
                    processSensorData(line);
                    //qDebug() <<"line="<<line;
                }
            }
            file.close();
        }
        raschetKolBas();
    }

    ui->loadDataButton->setText(sensorId_t.empty() ? "Загрузить данные" : "Данные загружены");

    /*  //вывести test_data
    for (size_t i = 0; i < sensorId_t.size(); ++i)
    {    qDebug() << sensorId_t[i] << velocity_t[i] << timePrev_t[i] << axleNum_t[i];    }
    */

}

void MainWindow::processSensorData(const QString &data)
{
    QRegularExpression regex(
        "\\[ID=(\\d+);\\s*vel=([\\d,]+);\\s*time_prev=(\\d+);\\s*axle_num=(\\d+)\\]");
    QRegularExpressionMatch match = regex.match(data);

    if (!match.hasMatch()) {
        qWarning() << "Неверный формат данных:" << data;
        return;
    }

    sensorId_t.push_back(match.captured(1).toInt());
    velocity_t.push_back(match.captured(2).replace(",", ".").toFloat());
    timePrev_t.push_back(match.captured(3).toInt());
    axleNum_t.push_back(match.captured(4).toInt());


}


void MainWindow::on_actionLoadConfig()
{
    on_loadDataButton_clicked();
}

