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
float Sensor::getDistanceToNextAxle() const { return distanceToNextAxle; }
void Sensor::setPosition(const QPointF &pos) { position = pos; }


void Sensor::handleAxlePass(Sensor* sensor, float velocity, qint64 timeSinceLast, int axleNum, float distance)
{
    // Проверяем, что данные пришли для этого датчика
    if (sensor != this) return;
    velocity =  velocity/ koef_M;

    // Сохраняем текущие данные
    axlePassTimes[axleNum] = timeSinceLast;
    axleSpeed[sensor->getId()] = velocity; //
    lastVelocity = velocity;

    //первая ось фиксируемая датчиком имеет время 0
    if (axlePassTimes[axleNum]!=0) lastTimeSinceLast = (axlePassTimes[axleNum]-axlePassTimes[axleNum+1]) ;
    else lastTimeSinceLast = 0;
//    qDebug()<<"lastTimeSinceLast="<<lastTimeSinceLast;

    // предыдущая ось ехала больше минуты?
    if (lastTimeSinceLast > 65535) {
        lastTimeSinceLast = 65535;
        distanceToNextAxle = 0;
    }
    // Рассчитываем расстояние до предыдущей оси
    else distanceToNextAxle = (lastTimeSinceLast * velocity)/1000;

    lastAxleNum = axleNum;

    // Генерируем сигнал с данными
    emit newData(id, lastVelocity, lastTimeSinceLast, lastAxleNum, distanceToNextAxle);
//    qDebug()<<"Id" << id <<"\t vel=" << velocity<< "\t t="  << getLastTimeSinceLast() << "\t n=" << axleNum<< getDistanceToNextAxle()/1000;

}


