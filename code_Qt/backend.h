#ifndef BACKEND_H
#define BACKEND_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
namespace Ui { class BackEnd; }
QT_END_NAMESPACE

class BackEnd : public QMainWindow
{
    Q_OBJECT

public:
    BackEnd(QWidget *parent = nullptr);
    ~BackEnd();

private slots:
    void networkReplyReadyRead();
    void networkReplyReadyReadSensor();
    void sendData();
    void sendAutoData();
    void giveData();
    void giveDataSensor();

    void on_lightBtn_clicked();

    void on_fanBtn_clicked();

    void on_pumpBtn_clicked();

    void on_autoBtn_clicked();

private:
    Ui::BackEnd *ui;
    QTimer *tmr,*tmr1;
    QNetworkAccessManager * m_networkManager;
    QNetworkAccessManager * m_networkManager1;
    QNetworkReply * m_networkReply;
    QNetworkReply * m_networkReply1;
};
#endif // BACKEND_H
