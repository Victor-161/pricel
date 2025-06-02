#include "axle.h"
#include "sensor.h"
#include <QDateTime>

Axle::Axle(int number, QObject *parent)
    : QObject(parent),
    number(number),
    position(),
    velocity(),
    lastPassTime(QDateTime::currentMSecsSinceEpoch())
{
    // Настраиваем таймер для обновления позиции
    moveTimer.setInterval(100); // Обновление каждые 100 мс
    connect(&moveTimer, &QTimer::timeout, this, &Axle::updatePosition);
    moveTimer.start();
}

void Axle::updatePosition() {

    if (qFuzzyIsNull(velocity)) return;

    float delta = velocity * 0.1f; // 0.1 секунды (интервал таймера)
    move(delta);
}

void Axle::move(float distance) {
    position += distance;

    // Проверяем пересечение с датчиками
    for (Sensor* sensor : sensors) {
        if (!sensor || passedSensors.contains(sensor)) continue;

        if (qAbs(position - sensor->getPosition().x()) < 1.0f) {
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            qint64 timeSinceLast = currentTime - lastPassTime;
            lastPassTime = currentTime;
            passedSensors.insert(sensor); // Фиксируем проход
        // Генерируем сигнал с полными данными
            emit passedSensor(sensor, getVelocity(), timeSinceLast, getNumber(), distance); // Добавляем номер оси
        }
    }
}

void Axle::registerSensor(Sensor* sensor) {
        if (sensor && !sensors.contains(sensor)) {
            sensors.append(sensor);
        }
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

float Axle::calculateDistanceTo(const Axle &other) const {
    return qAbs(position - other.position);
}
