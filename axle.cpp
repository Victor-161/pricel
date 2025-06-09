#include "axle.h"
#include "sensor.h"
#include <QDateTime>

Axle::Axle(int number, QObject *parent)
    : QObject(parent),
    number(number),
    position(),
    velocity(),
    lastPassTime()
{
    // Настраиваем таймер для обновления позиции
    moveTimer.setInterval(10); // Обновление каждые 100 мс
    connect(&moveTimer, &QTimer::timeout, this, &Axle::updatePosition);
}


void Axle::updatePosition() {

    if (qFuzzyIsNull(velocity)) return;

    float delta = velocity * 0.01f; // 0.1 секунды (интервал таймера)
    move(delta);
}

void Axle::move(float distance) {

    position += distance;

    // Проверяем пересечение с датчиками
    for (Sensor* sensor : sensors) {
        if (!sensor || passedSensors.contains(sensor)) continue;

        if (qAbs(position - sensor->getPosition().x()) < 1.0f) {

            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            lastPassTime = currentTime;
            //qDebug()<<"lastPassTime="<<lastPassTime;
            //qDebug()<<"moveTimer="<<moveTimer.;

            passedSensors.insert(sensor); // Фиксируем проход

        // Генерируем сигнал с полными данными
            emit passedSensor(sensor, velocity, lastPassTime, number, distance  ); // Добавляем номер оси
//          qDebug()<<"Id_1" << sensor->getId() <<"\t vel_1=" << getVelocity()<< "\t t_1="  << lastPassTime << "\t n=" << getNumber()<< distance;
        }
    }
}

void Axle::registerSensor(Sensor* sensor) {
        if (sensor && !sensors.contains(sensor)) {
            sensors.append(sensor);
        }
}

void Axle::setLastIsBlocked(bool blocked) {lastIsBlocked = blocked; }

bool Axle::isBlocked() const {
    return lastIsBlocked;
}

int Axle::getNumber() const {
    return number;
}

float Axle::getPosition() const {
    return position;
}

float Axle::getVelocity() const {
    return velocity;
}

void Axle::setPosition(float position) {
    this->position = position;
}

void Axle::setVelocity(float newVelocity) {
    this->velocity = newVelocity;

    if (!qFuzzyIsNull(newVelocity)) {
        lastPassTime = QDateTime::currentMSecsSinceEpoch();
    }
}

void Axle::resetPosition(float newPosition)
{
    position = newPosition;
    lastPassTime = QDateTime::currentMSecsSinceEpoch();
    passedSensors.clear(); // Сбрасываем флаги прохода
}


void Axle::startTimer(){
    moveTimer.start();
//    qDebug() << "Таймер сработал!";
}

void Axle::stopTimer(){
    moveTimer.stop();
}
