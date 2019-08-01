#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QtCharts>
#include "qaesencryption.h"

#define IP                      "192.168.50.1"
#define PORT                    12800
#define ID_OBJET                "111111"
#define ID_APP_X                "XX"
#define ID_APP_MEDECIN          "F1"
#define ID_APP_INFIRMIER        "F2"
#define ID_APP_SECRETAIRE       "F3"
#define ID_APP_FAMILLE          "F4"
#define GET_LITS                "00"
#define PUT_DATA_PATIENT        "11"
#define GET_DATA_PATIENT        "12"
#define GET_SUIVI_PATIENT       "13"
#define ARRET_HOSPITALISATION   "18"

#define ATTENTE_MSG_MEDECIN         1
#define ATTENTE_MSG_INFIRMIER       2
#define ATTENTE_MSG_SECRETAIRE      3
#define ATTENTE_MSG_FAMILLE         4
#define ATTENTE_MSG_LITS_DISPO      5
#define ATTENTE_MSG_LITS_PATIENTS   6

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonConnexionTcp_clicked(); // Connexion automatique
    void on_pushButtonDeconnexionTcp_clicked();
    void on_pushButtonConnexionUtilisateur_clicked();
    void on_pushButtonDeconnexionUtilisateur_clicked();
    void on_pushButtonValidationPatient_clicked();
    void on_pushButtonEnregistrer_clicked();
    void on_pushButtonAjoutPatient_clicked();
    void on_pushButtonLitsDispo_clicked();
    void on_pushButtonFinHospitalisation_clicked();
    void on_pushButtonChargementDonnees_clicked();
    void onReadyRead();
    void erreurSocket(QAbstractSocket::SocketError erreur);

private:
    Ui::MainWindow *ui;
    QTcpSocket *_socket;
    QAESEncryption *_encryption;
    QByteArray _key16Medecin, _key16Infirmier, _key16Secretaire, _key16Famille, _outputHex;
    quint8 _flagMsgAttendu;
    quint8 _flagSuiviPatient;
    QLineSeries *_temperatureSeries;
    QLineSeries *_rythmeCardSeries;
    QLineSeries *_freqRespiSeries;
    QLineSeries *_tensionSySeries;
    QLineSeries *_tensionDiSeries;
    void clearChart();
    void suiteConnexionUtilisateur();
    void transmitData(QString id_app, QString commande, QByteArray msg);
    void displayData(QJsonDocument doc);
    void displayLitsDispo(QJsonDocument doc);
    void displayLitsPatient(QJsonDocument doc);
    void displaySuiviPatient(QJsonDocument doc);
    void clearDataAdministrative();
    void onOroffDataAdministrative(bool booleanValue);
};

#endif // MAINWINDOW_H
