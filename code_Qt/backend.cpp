#include "backend.h"
#include "ui_backend.h"

#include <QTime>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QElapsedTimer>
#include <QSslSocket>

QString timeData[10];

BackEnd::BackEnd(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::BackEnd)
{
    ui->setupUi(this);

    //============================================================================= setup do thi
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(Qt::red));
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(Qt::blue));
    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setPen(QPen(Qt::green));
    //----------------------------------------------------------------------truc y1
    ui->customPlot->yAxis->setLabel("Humidity (%)");
    ui->customPlot->yAxis->setRange(0,100);
    ui->customPlot->yAxis->ticker()->setTickCount(10);
    //----------------------------------------------------------------------truc y2
    ui->customPlot->yAxis2->setVisible(true); // truc y2 mac dinh bi an
    ui->customPlot->yAxis2->setLabel("Temperature(^C)");
    ui->customPlot->yAxis2->setRange(0,50);
    ui->customPlot->yAxis2->ticker()->setTickCount(10);
    //----------------------------------------------------------------------truc x
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    ui->customPlot->xAxis->setLabel("Date");
    ui->customPlot->replot();
    //=============================================================================================

    //---------------------------------khoi chay timer de doc trang thai cho cac Btn
    this->tmr = new QTimer(this);
    tmr->setInterval(2000); //---------> 2s/lan
    connect(this->tmr,SIGNAL(timeout()),this,SLOT(giveData()));
    tmr->start();
    //---------------------------------khoi chay timer de doc trang thai sensor
    this->tmr1 = new QTimer(this);
    tmr1->setInterval(5000); //---------> 5s/lan
    connect(this->tmr1,SIGNAL(timeout()),this,SLOT(giveDataSensor()));
    tmr1->start();
}

BackEnd::~BackEnd()
{
    delete ui;
}

//------------------------------------------------------------------------
void BackEnd::giveData() //--> code lay du lieu tu firebase ve
{
    m_networkManager = new QNetworkAccessManager( this );
    m_networkReply = m_networkManager->get( QNetworkRequest( QUrl( "https://qtfirebaseintegrationexa-942c6-default-rtdb.firebaseio.com/RealTimeData.json" )));
    connect( m_networkReply, &QNetworkReply::readyRead, this, &BackEnd::networkReplyReadyRead );
}

void BackEnd::giveDataSensor() //--> code lay du lieu DHT11 tu firebase ve
{
    m_networkManager1 = new QNetworkAccessManager( this );
    m_networkReply1 = m_networkManager1->get( QNetworkRequest( QUrl( "https://qtfirebaseintegrationexa-942c6-default-rtdb.firebaseio.com/RealTimeData.json" )));
    connect( m_networkReply1, &QNetworkReply::readyRead, this, &BackEnd::networkReplyReadyReadSensor );

    //================================================================================================
    static QElapsedTimer time; // thoi gian da troi qua
    double key = time.elapsed() / 1000; // miligiay
    //-------------------------------------------------------------
    for(int i=0;i<9;i++){
        timeData[i]=timeData[i+1];//tao du lieu cho truc x (lui dan time)
    }
    timeData[9]=QTime::currentTime().toString("hh:mm:ss"); // dong ho gioi gian hien tai
    //-------------------------------------cap nhat cap du lieu (x,y)
    double val = ui->lcdTemp->value();
    double val1 = ui->lcdHumi->value();
    double val2 = ui->lcdWet->value();
    //----------------------------------------------xay dung truc y
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);

    for(int i=0;i<10;i++){
        textTicker->addTick(key-45 + 5*i,timeData[i]); // 5*i - 45
    }
    /*buoc nhay cua key la 2
    gia tri lui tuong ung -----------(key-15)----------(key-10)----------(key-5)-----------(key)
                                     timeData[6]      timeData[7]      timeData[8]       timeData[9]
    giai he pt a*i +b=c     a*9 + b = 0
                            a*8 + b = -5
    */
    ui->customPlot->xAxis->setTicker(textTicker);
    ui->customPlot->graph(0)->addData(key, val);
    ui->customPlot->graph(1)->addData(key, val1);
    ui->customPlot->graph(2)->addData(key, val2);
    ui->customPlot->xAxis->setRange(key, 50, Qt::AlignRight);// du lieu duoc ve tu ben phai key = 0
                                                             // buoc nhay = 5 , co 50/5 vach du lieu
    ui->customPlot->replot(); // reset lai plot
}

void BackEnd::networkReplyReadyRead()
{
    QByteArray data_json = m_networkReply->readAll();
    //qDebug()<< data_json;
    QJsonDocument doc = doc.fromJson(data_json);
    QJsonObject object_json = doc.object();
    QJsonValue data_fan = object_json.value(QByteArray("Relay_Fan"));
    QJsonValue data_light = object_json.value(QByteArray("Relay_Light"));
    QJsonValue data_pump = object_json.value(QByteArray("Relay_Pump"));
    QJsonValue data_sensorLight = object_json.value(QByteArray("Value_Light"));
    ui->fanBtn->setText(data_fan.toString());
    ui->lightBtn->setText(data_light.toString());
    ui->pumpBtn->setText(data_pump.toString());
}
void BackEnd::networkReplyReadyReadSensor()
{
    //qDebug() << m_networkReply->readAll();
    QByteArray data_sensor = m_networkReply1->readAll();
    //qDebug() << array1;
    QJsonDocument docSensor = docSensor.fromJson(data_sensor);
    QJsonObject object_sensor = docSensor.object();
    QJsonValue data_Humidity = object_sensor.value(QByteArray("Value_Humidity"));
    QJsonValue data_Temperature = object_sensor.value(QByteArray("Value_Temperature"));
    QJsonValue data_Wet = object_sensor.value(QByteArray("Value_Wet"));
    ui->lcdHumi->display(data_Humidity.toString());
    ui->lcdTemp->display(data_Temperature.toString());
    ui->lcdWet->display(data_Wet.toString());
}
void BackEnd::sendData() //--> code gui du lieu len firebase
{
    m_networkManager = new QNetworkAccessManager( this );
    QVariantMap newRealTimeData;
    newRealTimeData[ "Relay_Fan" ] =            ui->fanBtn->text();
    newRealTimeData[ "Relay_Light" ] =          ui->lightBtn->text();
    newRealTimeData[ "Relay_Pump" ] =           ui->pumpBtn->text();
    newRealTimeData[ "Value_Humidity" ] =       QString::number(ui->lcdHumi->value());
    newRealTimeData[ "Value_Light" ] =          ui->valueLight->text();
    newRealTimeData[ "Value_Temperature" ] =    QString::number(ui->lcdTemp->value());
    newRealTimeData[ "Value_Wet" ] =            QString::number(ui->lcdWet->value());
    newRealTimeData[ "Time" ] =                 QTime::currentTime().toString("hh:mm:ss"); // dong ho gioi gian hien tai
    QJsonDocument jsonDoc = QJsonDocument::fromVariant( newRealTimeData);
    QNetworkRequest newRealTimeDataRequest( QUrl("https://qtfirebaseintegrationexa-942c6-default-rtdb.firebaseio.com/RealTimeData.json"));
    newRealTimeDataRequest.setHeader( QNetworkRequest::ContentTypeHeader, QString( "application/json"));
    m_networkManager->put( newRealTimeDataRequest, jsonDoc.toJson());
}

void BackEnd::sendAutoData() //--> code gui du lieu len firebase
{
    m_networkManager = new QNetworkAccessManager( this );
    QVariantMap newAutoData;
    newAutoData[ "Status_Auto" ] =                 ui->autoBtn->text();
    newAutoData[ "Value_Humidity" ] =       QString::number(ui->autoHum->value());
    newAutoData[ "Value_Light" ] =          QString::number(ui->autoLight->isChecked());
    newAutoData[ "Value_Temperature" ] =    QString::number(ui->autoTem->value());
    newAutoData[ "Value_Wet" ] =            QString::number(ui->autoWet->value());
    QJsonDocument jsonDoc = QJsonDocument::fromVariant( newAutoData);
    QNetworkRequest newAutoDataRequest( QUrl("https://qtfirebaseintegrationexa-942c6-default-rtdb.firebaseio.com/AutoData.json"));
    newAutoDataRequest.setHeader( QNetworkRequest::ContentTypeHeader, QString( "application/json"));
    m_networkManager->put( newAutoDataRequest, jsonDoc.toJson());
}
//============================================================================================================

//------------------------------------------------------------------------code btn ON-OFF
void BackEnd::on_lightBtn_clicked()
{
    QString s = ui->lightBtn->text();
    if(s=="ON"){
       ui->lightBtn->setText("OFF");
    }else{
        ui->lightBtn->setText("ON");
    }
    sendData();
}

void BackEnd::on_fanBtn_clicked()
{
    QString s = ui->fanBtn->text();
    if(s=="ON"){
       ui->fanBtn->setText("OFF");
    }else{
        ui->fanBtn->setText("ON");
    }
    sendData();
}

void BackEnd::on_pumpBtn_clicked()
{
    QString s = ui->pumpBtn->text();
    if(s=="ON"){
       ui->pumpBtn->setText("OFF");
    }else{
        ui->pumpBtn->setText("ON");
    }
    sendData();
}


void BackEnd::on_autoBtn_clicked()
{
    QString s = ui->autoBtn->text();
    if(s=="ON"){
       ui->autoBtn->setText("OFF");
    }else{
        ui->autoBtn->setText("ON");
        sendAutoData();
    }
}

