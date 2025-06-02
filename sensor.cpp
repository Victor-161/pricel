#include "sensor.h"
#include <QDateTime>
#include <QDebug>

Sensor::Sensor(int id, QObject *parent)
    : QObject(parent),
    id(id)

{
}

int Sensor::getId() const               { return id; }
QPointF Sensor::getPosition() const     { return position; }
float Sensor::getLastVelocity() const   { return lastVelocity; }
qint64 Sensor::getLastTimeSinceLast() const { return lastTimeSinceLast; }
int Sensor::getLastAxleNum() const          { return lastAxleNum; }
void Sensor::setPosition(const QPointF &pos) { position = pos; }



void Sensor::registerAxlePass(Axle* axle, float velocity, qint64 timePrev) {
    AxleDetection detection;
    detection.axle = axle;
    detection.velocity = velocity;
    detection.timePrev = timePrev;
    detection.detectionTime = QDateTime::currentMSecsSinceEpoch();
    detections.append(detection);
}

void Sensor::handleAxlePass(Sensor* sensor, float velocity, qint64 timeSinceLast, int axleNum, float distance)
{
//    qDebug() <<"Sensor_handleAxlePass";
    // Проверяем, что данные пришли для этого датчика
    if (sensor != this) return;

    // Сохраняем текущие данные
    axlePassTimes[axleNum] = QDateTime::currentMSecsSinceEpoch();
//    axlePassTimes[axleNum] = timeSinceLast;
    axlePositions[axleNum] = position.x(); // Позиция датчика

    // Рассчитываем расстояние до предыдущей оси
//    distance = calculateAxleDistance(axleNum, velocity);
//    float distance =3.50;
//    if (axlePassTimes.size() > 1) distance = (velocity * timeSinceLast)/1000;
    //qDebug()<<"distance="<<distance;

    // Сохраняем последние данные
    lastVelocity = velocity;
    lastTimeSinceLast = timeSinceLast;
    lastAxleNum = axleNum;
 //   id = sensor->getId();

    // Генерируем сигнал с данными
    emit newData(id, velocity, timeSinceLast, axleNum, distance);
//    qDebug()<<"Id" << id <<"\t vel=" << velocity<< "\t t="  << timeSinceLast << "\t n=" << axleNum;

}

bool Sensor::isAxlePassing(float axlePos) const {
    static const float SENSOR_WIDTH = 0.2f; // Ширина зоны датчика
    return qAbs(axlePos - position.x()) < SENSOR_WIDTH;
}
