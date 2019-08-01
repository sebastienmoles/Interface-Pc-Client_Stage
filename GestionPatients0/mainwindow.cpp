#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCharts>
#include <QtDebug>
#include <QHostAddress>
#include "qaesencryption.h"
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _temperatureSeries = new QLineSeries();
    _temperatureSeries->append(0,36);
    _temperatureSeries->append(9,42);

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(_temperatureSeries);
    chart->createDefaultAxes();
    chart->setTitle("Température");

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayoutSanteChart->addWidget(chartView);
    _temperatureSeries->clear();

    _rythmeCardSeries = new QLineSeries();
    _rythmeCardSeries->append(0, 45);
    _rythmeCardSeries->append(9, 150);

    QChart *chart2 = new QChart();
    chart2->legend()->hide();
    chart2->addSeries(_rythmeCardSeries);
    chart2->createDefaultAxes();
    chart2->setTitle("Rythme Cardiaque");

    QChartView *chartView2 = new QChartView(chart2);
    chartView2->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayoutSanteChart->addWidget(chartView2);
    _rythmeCardSeries->clear();

    _freqRespiSeries = new QLineSeries();
    _freqRespiSeries->append(0, 16);
    _freqRespiSeries->append(9, 50);

    QChart *chart3 = new QChart();
    chart3->legend()->hide();
    chart3->addSeries(_freqRespiSeries);
    chart3->createDefaultAxes();
    chart3->setTitle("Fréquence respiratoire");

    QChartView *chartView3 = new QChartView(chart3);
    chartView2->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayoutSanteChart->addWidget(chartView3);
    _freqRespiSeries->clear();

    _tensionSySeries = new QLineSeries();
    _tensionDiSeries = new QLineSeries();
    _tensionSySeries->append(0, 60);
    _tensionSySeries->append(9, 200);

    QChart *chart4 = new QChart();
    chart4->legend()->hide();
    chart4->addSeries(_tensionSySeries);
    chart4->addSeries(_tensionDiSeries);
    chart4->createDefaultAxes();
    chart4->setTitle("Tension");

    QChartView *chartView4 = new QChartView(chart4);
    chartView2->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayoutSanteChart->addWidget(chartView4);
    _tensionSySeries->clear();

    //QLabel *lbl = new QLabel;
    QMovie *movie = new QMovie("ressources/gif_loading4.gif");
    ui->label_gif_chargement->setMovie(movie);
    ui->label_gif_chargement->show();
    ui->label_gif_chargement->setVisible(FALSE);
    movie->start();

    _socket = new QTcpSocket(this);
    _encryption = new QAESEncryption(QAESEncryption::AES_128, QAESEncryption::ECB);

    /* Connexion SIGNAL et SLOT */
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocket(QAbstractSocket::SocketError)));

    char key_16_Medecin[16] =  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    for (int i=0; i<16; i++)
        _key16Medecin.append(key_16_Medecin[i]);

    char key_16_Infirmier[16] =  {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    for (int i=0; i<16; i++)
        _key16Infirmier.append(key_16_Infirmier[i]);

    char key_16_Secretaire[16] =  {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F};
    for (int i=0; i<16; i++)
        _key16Secretaire.append(key_16_Secretaire[i]);

    char key_16_Famille[16] =  {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
    for (int i=0; i<16; i++)
        _key16Famille.append(key_16_Famille[i]);

    _flagMsgAttendu = 0;
    _flagSuiviPatient = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonConnexionTcp_clicked()
{
    _socket->abort();
    _socket->connectToHost(QHostAddress(IP),PORT);
    connect(_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    if(_socket->waitForConnected(3000))
    {
        qDebug() << "connected";
        ui->statusBar->showMessage(" Vous êtes connecté ");
        _socket->write(QByteArray("connected"));
        ui->pushButtonDeconnexionTcp->setEnabled(TRUE);
        ui->pushButtonConnexionTcp->setEnabled(FALSE);
        ui->comboBoxUtilisateur->setEnabled(TRUE);
        ui->lineEditMotdepasse->setEnabled(TRUE);
        ui->pushButtonConnexionUtilisateur->setEnabled(TRUE);
    }else
    {
        qDebug() << _socket->error();
        ui->statusBar->showMessage("! Erreur de connexion ! ");
    }
}

void MainWindow::on_pushButtonDeconnexionTcp_clicked()
{
    _socket->disconnectFromHost();
    qDebug() << "disconnected";
    ui->statusBar->showMessage(" Vous êtes déconncté ");
    ui->pushButtonDeconnexionTcp->setEnabled(FALSE);
    ui->pushButtonConnexionTcp->setEnabled(TRUE);
    on_pushButtonDeconnexionUtilisateur_clicked();
    ui->comboBoxUtilisateur->setEnabled(FALSE);
    ui->lineEditMotdepasse->setEnabled(FALSE);
    ui->pushButtonConnexionUtilisateur->setEnabled(FALSE);
    ui->pushButtonDeconnexionUtilisateur->setEnabled(FALSE);
    clearDataAdministrative();
    clearChart();
}

void MainWindow::onReadyRead()
{
    QJsonDocument doc2;
    QString testdata;
    QByteArray decodedText;
    QByteArray datas = _socket->readAll();
    qDebug() << datas;
    switch (_flagMsgAttendu) {
    case ATTENTE_MSG_MEDECIN:
        if(_flagSuiviPatient)
        {
            doc2 = QJsonDocument::fromJson(datas);
            displaySuiviPatient(doc2);
        }else
        {
            decodedText = _encryption->decode(datas, _key16Medecin);
            qDebug() << "Texte Dechiffre" << decodedText;
            decodedText = _encryption->removePadding(decodedText);
            doc2 = QJsonDocument::fromJson(decodedText);
            displayData(doc2);
        }
        break;
    case ATTENTE_MSG_INFIRMIER:
        if(_flagSuiviPatient)
        {
            doc2 = QJsonDocument::fromJson(datas);
            displaySuiviPatient(doc2);
        }else
        {
            decodedText = _encryption->decode(datas, _key16Infirmier);
            qDebug() << "Texte Dechiffre" << decodedText;
            decodedText = _encryption->removePadding(decodedText);
            doc2 = QJsonDocument::fromJson(decodedText);
            displayData(doc2);
        }
        break;
    case ATTENTE_MSG_SECRETAIRE:
        decodedText = _encryption->decode(datas, _key16Secretaire);
        qDebug() << "Texte Dechiffre" << decodedText;
        decodedText = _encryption->removePadding(decodedText);
        doc2 = QJsonDocument::fromJson(decodedText);
        displayData(doc2);
        break;
    case ATTENTE_MSG_FAMILLE:
        if(_flagSuiviPatient)
        {
            doc2 = QJsonDocument::fromJson(datas);
            displaySuiviPatient(doc2);
        }else
        {
            decodedText = _encryption->decode(datas, _key16Famille);
            qDebug() << "Texte Dechiffre" << decodedText;
            decodedText = _encryption->removePadding(decodedText);
            doc2 = QJsonDocument::fromJson(decodedText);
            displayData(doc2);
        }
        break;
    case ATTENTE_MSG_LITS_DISPO:
        doc2 = QJsonDocument::fromJson(datas);
        displayLitsDispo(doc2);
        break;
    case ATTENTE_MSG_LITS_PATIENTS:
        doc2 = QJsonDocument::fromJson(datas);
        displayLitsPatient(doc2);

        break;
    default:
        break;
    }
}

void MainWindow::erreurSocket(QAbstractSocket::SocketError erreur)
{
    switch(erreur) // On affiche un message différent selon l'erreur qu'on nous indique
    {
    case QAbstractSocket::HostNotFoundError:
        qDebug() << (tr("<em>ERREUR : le serveur n'a pas pu être trouvé. Vérifiez l'IP et le port.</em>"));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << (tr("<em>ERREUR : le serveur a refusé la connexion. Vérifiez si le programme \"serveur\" a bien été lancé. Vérifiez aussi l'IP et le port.</em>"));
        break;
    case QAbstractSocket::RemoteHostClosedError:
        qDebug() << (tr("<em>ERREUR : le serveur a coupé la connexion.</em>"));
        break;
    default:
        qDebug() << (tr("<em>ERREUR : ") + _socket->errorString() + tr("</em>"));
    }

    ui->pushButtonConnexionTcp->setEnabled(TRUE);
    ui->pushButtonDeconnexionTcp->setEnabled(FALSE);
}

void MainWindow::on_pushButtonConnexionUtilisateur_clicked()
{
    if(ui->comboBoxUtilisateur->currentText() == "Médecin")
    {
        if(ui->lineEditMotdepasse->text() == "0000")
        {
            suiteConnexionUtilisateur();
        }else {ui->statusBar->showMessage("Mot de passe incorrect");}
    }else if (ui->comboBoxUtilisateur->currentText() == "Infirmier")
    {
        if(ui->lineEditMotdepasse->text() == "1111")
        {
            suiteConnexionUtilisateur();
        }else {ui->statusBar->showMessage("Mot de passe incorrect");}
    }else if (ui->comboBoxUtilisateur->currentText() == "Secrétaire")
    {
        if(ui->lineEditMotdepasse->text() == "2222")
        {
            suiteConnexionUtilisateur();
        }else {ui->statusBar->showMessage("Mot de passe incorrect");}
    }else if (ui->comboBoxUtilisateur->currentText() == "Famille")
    {
        if(ui->lineEditMotdepasse->text() == "3333")
        {
            suiteConnexionUtilisateur();
        }else {ui->statusBar->showMessage("Mot de passe incorrect");}
    }
}

void MainWindow::on_pushButtonDeconnexionUtilisateur_clicked()
{
    clearDataAdministrative();
    clearChart();
    onOroffDataAdministrative(FALSE);
    ui->pushButtonEnregistrer->setEnabled(FALSE);
    ui->pushButtonDeconnexionUtilisateur->setEnabled(FALSE);
    ui->pushButtonAjoutPatient->setEnabled(FALSE);
    ui->comboBoxUtilisateur->setEnabled(TRUE);
    ui->pushButtonConnexionUtilisateur->setEnabled(TRUE);
    ui->lineEditNumDossier->setEnabled(FALSE);
    ui->lineEditMdpPatient->setEnabled(FALSE);
    ui->pushButtonValidationPatient->setEnabled(FALSE);
    ui->comboBoxLit->setEnabled(FALSE);
    ui->lineEditMotdepasse->clear();
    ui->lineEditNumDossier->clear();
    ui->lineEditMdpPatient->clear();
    ui->pushButtonLitsDispo->setEnabled(FALSE);
    ui->pushButtonFinHospitalisation->setEnabled(FALSE);
    ui->pushButtonChargementDonnees->setEnabled(FALSE);
}

void MainWindow::on_pushButtonValidationPatient_clicked()
{
    _flagMsgAttendu = 0;
    _flagSuiviPatient = 0;
    ui->pushButtonFinHospitalisation->setEnabled(TRUE);
    clearDataAdministrative();
    clearChart();
    onOroffDataAdministrative(FALSE);
    QByteArray byte_numero_dossier;
    if(ui->lineEditNumDossier->text().isEmpty())
    {
        ui->statusBar->showMessage("Veuillez entrer un numéro de dossier");
        qDebug() << "test";
    }else
    {
        byte_numero_dossier = ui->lineEditNumDossier->text().toLocal8Bit();
        switch (ui->comboBoxUtilisateur->currentIndex()) {
        case 0: // Médecin
            transmitData(ID_APP_MEDECIN, GET_DATA_PATIENT, byte_numero_dossier);
            _flagMsgAttendu = 1;
            ui->label_gif_chargement->setVisible(TRUE);
            break;
        case 1: // Infirmier
            transmitData(ID_APP_INFIRMIER, GET_DATA_PATIENT, byte_numero_dossier);
            _flagMsgAttendu = 2;
            ui->label_gif_chargement->setVisible(TRUE);
            break;
        case 2: // Secrétaire
            transmitData(ID_APP_SECRETAIRE, GET_DATA_PATIENT, byte_numero_dossier);
            _flagMsgAttendu = 3;
            ui->label_gif_chargement->setVisible(TRUE);
            break;
        case 3: // Famille
            if(ui->lineEditMdpPatient->text() != "0000")
            {
                ui->statusBar->showMessage("Veuillez entrer le bon mot de passe");
            }else
            {
                transmitData(ID_APP_FAMILLE, GET_DATA_PATIENT, byte_numero_dossier);
                _flagMsgAttendu = 4;
                ui->label_gif_chargement->setVisible(TRUE);
            }
            break;
        }
    }

    if(ui->comboBoxUtilisateur->currentText() != "Secrétaire")
    {
        ui->pushButtonChargementDonnees->setEnabled(TRUE);
    }

    _flagSuiviPatient = 0;
}

void MainWindow::suiteConnexionUtilisateur()
{
    ui->statusBar->showMessage(" Utilisateur connecté ");
    ui->comboBoxUtilisateur->setEnabled(FALSE);
    ui->pushButtonConnexionUtilisateur->setEnabled(FALSE);
    ui->pushButtonDeconnexionUtilisateur->setEnabled(TRUE);
    ui->lineEditNumDossier->setEnabled(TRUE);
    ui->pushButtonAjoutPatient->setEnabled(TRUE);
    ui->pushButtonValidationPatient->setEnabled(TRUE);
    if(ui->comboBoxUtilisateur->currentText() == "Famille")
    {
        ui->lineEditMdpPatient->setEnabled(TRUE);
        ui->pushButtonAjoutPatient->setEnabled(FALSE);
    }
}

void MainWindow::on_pushButtonEnregistrer_clicked()
{
    if(ui->lineEditNom->text().isEmpty() || ui->lineEditPrenom->text().isEmpty() || ui->lineEditAdresse->text().isEmpty()
            || ui->lineEditCodePostal->text().isEmpty() || ui->lineEditVille->text().isEmpty() || ui->spinBoxAge->value() == 0
            || ui->comboBoxSexe->currentText() == "------" || ui->spinBoxPoids->value() == 0
            || ui->comboBoxGroupeSanguin->currentText() == "------" || ui->comboBoxLit->currentText() == "------")
    {
        ui->statusBar->showMessage("Veuillez compléter toutes les parties");
    }else {
        QJsonObject infoAdmin;
        infoAdmin["Nom"] = ui->lineEditNom->text();
        infoAdmin["Prénom"] = ui->lineEditPrenom->text();
        infoAdmin["Adresse"] = ui->lineEditAdresse->text();
        infoAdmin["Code Postal"] = ui->lineEditCodePostal->text();
        infoAdmin["Ville"] = ui->lineEditVille->text();
        infoAdmin["Age"] = ui->spinBoxAge->text();
        infoAdmin["Sexe"] = ui->comboBoxSexe->currentText();
        infoAdmin["Poids"] = ui->spinBoxPoids->text();
        infoAdmin["Groupe Sanguin"] = ui->comboBoxGroupeSanguin->currentText();
        infoAdmin["Date d'entrée"] = ui->dateEditDateEntree->text();
        QJsonDocument doc(infoAdmin);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        qDebug() << strJson;

        QByteArray utf_json = strJson.toUtf8();
        qDebug() << utf_json;

        QByteArray encodedText;

        switch (ui->comboBoxUtilisateur->currentIndex()) {
        case 0: // Médecin
            encodedText = _encryption->encode(utf_json, _key16Medecin);
            encodedText = encodedText.toHex();
            if(ui->lineEditNumeroDossier->text().size() < 6)
            {
                ui->statusBar->showMessage("Mauvais format n° de dossier (il faut 6 chiffres");
                break;
            }
            encodedText.insert(0, ui->lineEditNumeroDossier->text().toUtf8());
            encodedText.insert(0, ui->comboBoxLit->currentText().toLocal8Bit());
            transmitData(ID_APP_MEDECIN, PUT_DATA_PATIENT, encodedText);
            ui->statusBar->showMessage("Sauvegarde patient : OK !");
            qDebug() << encodedText;
            break;
        case 1: // Infirmier
            encodedText = _encryption->encode(utf_json, _key16Infirmier);
            encodedText = encodedText.toHex();
            if(ui->lineEditNumeroDossier->text().size() < 6)
            {
                ui->statusBar->showMessage("Mauvais format n° de dossier (il faut 6 chiffres");
                break;
            }
            encodedText.insert(0, ui->lineEditNumeroDossier->text().toUtf8());
            encodedText.insert(0, ui->comboBoxLit->currentText().toLocal8Bit());
            transmitData(ID_APP_INFIRMIER, PUT_DATA_PATIENT, encodedText);
            qDebug() << encodedText;
            ui->statusBar->showMessage("Sauvegarde patient : OK !");
            break;
        case 2: // Secrétaire
            encodedText = _encryption->encode(utf_json, _key16Secretaire);
            encodedText = encodedText.toHex();
            if(ui->lineEditNumeroDossier->text().size() < 6)
            {
                ui->statusBar->showMessage("Mauvais format n° de dossier (il faut 6 chiffres");
                break;
            }
            encodedText.insert(0, ui->lineEditNumeroDossier->text().toUtf8());
            encodedText.insert(0, ui->comboBoxLit->currentText().toLocal8Bit());
            transmitData(ID_APP_SECRETAIRE, PUT_DATA_PATIENT, encodedText);
            qDebug() << encodedText;
            ui->statusBar->showMessage("Sauvegarde patient : OK !");
            break;
        }

    }

}

void MainWindow::on_pushButtonAjoutPatient_clicked()
{
    ui->pushButtonChargementDonnees->setEnabled(FALSE);
    ui->pushButtonFinHospitalisation->setEnabled(FALSE);
    ui->pushButtonEnregistrer->setEnabled(TRUE);
    ui->lineEditNumeroDossier->setEnabled(TRUE);
    ui->pushButtonLitsDispo->setEnabled(TRUE);
    onOroffDataAdministrative(TRUE);
    clearDataAdministrative();
    clearChart();
}

void MainWindow::transmitData(QString id_app, QString commande, QByteArray msg)
{
    QString id_objet = ID_OBJET;
    QByteArray byte_id_objet, byte_id_app, byte_commande, msg_a_envoyer;
    byte_id_objet = id_objet.toLocal8Bit();
    byte_id_app = id_app.toLocal8Bit();
    byte_commande = commande.toLocal8Bit();
    msg_a_envoyer = byte_id_objet;
    msg_a_envoyer.append(byte_id_app);
    msg_a_envoyer.append(byte_commande);
    msg_a_envoyer.append(msg);
    qDebug() << msg_a_envoyer;
    _socket->write(msg_a_envoyer);
}

void MainWindow::displayData(QJsonDocument doc)
{
    QJsonObject obj;
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            obj = doc.object();
        }
        else
        {
            qDebug() << "Document is not an object" << endl;
        }
    }
    else
    {
        qDebug() << "Invalid JSON...\n" << endl;
    }
    ui->lineEditNom->insert(obj["Nom"].toString());
    ui->lineEditPrenom->insert(obj["Prénom"].toString());
    ui->lineEditAdresse->insert(obj["Adresse"].toString());
    ui->lineEditCodePostal->insert(obj["Code Postal"].toString());
    ui->lineEditVille->insert(obj["Ville"].toString());
    ui->spinBoxAge->setValue(obj["Age"].toString().toInt());
    ui->comboBoxSexe->setCurrentText(obj["Sexe"].toString());
    ui->spinBoxPoids->setValue(obj["Poids"].toString().toInt());
    ui->comboBoxGroupeSanguin->setCurrentText(obj["Groupe Sanguin"].toString());
    ui->dateEditDateEntree->insert(obj["Date d'entrée"].toString());
    ui->lineEditNumeroDossier->insert(ui->lineEditNumDossier->text());

    _flagMsgAttendu = 0;
    QByteArray empty_msg = "";
    transmitData(ID_APP_X, GET_LITS, empty_msg);
    _flagMsgAttendu = ATTENTE_MSG_LITS_PATIENTS;
    ui->comboBoxLit->clear();

    ui->label_gif_chargement->setVisible(FALSE);
}

void MainWindow::on_pushButtonLitsDispo_clicked()
{
    _flagMsgAttendu = 0;
    QByteArray empty_msg = "";
    transmitData(ID_APP_X, GET_LITS, empty_msg);
    _flagMsgAttendu = ATTENTE_MSG_LITS_DISPO;
    ui->comboBoxLit->clear();
}

void MainWindow::displayLitsDispo(QJsonDocument doc)
{
    QJsonObject obj;
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            obj = doc.object();
        }
        else
        {
            qDebug() << "Document is not an object" << endl;
        }
    }
    else
    {
        qDebug() << "Invalid JSON...\n" << endl;
    }

    ui->comboBoxLit->setEnabled(TRUE);

    if(obj["1.a"].toString() == "-")
    {
        ui->comboBoxLit->addItem("1.a");
    }
    if(obj["1.b"].toString() == "-")
    {
        ui->comboBoxLit->addItem("1.b");
    }
    if(obj["2.a"].toString() == "-")
    {
        ui->comboBoxLit->addItem("2.a");
    }
    if(obj["2.b"].toString() == "-")
    {
        ui->comboBoxLit->addItem("2.b");
    }
    if(obj["3.a"].toString() == "-")
    {
        ui->comboBoxLit->addItem("3.a");
    }
    if(obj["3.b"].toString() == "-")
    {
        ui->comboBoxLit->addItem("3.b");
    }
    if(obj["4.a"].toString() == "-")
    {
        ui->comboBoxLit->addItem("4.b");
    }
    if(obj["5.a"].toString() == "-")
    {
        ui->comboBoxLit->addItem("5.a");
    }

}

void MainWindow::displayLitsPatient(QJsonDocument doc)
{
    QString num_dossier;
    num_dossier = ui->lineEditNumDossier->text();
    QJsonObject obj;
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            obj = doc.object();
            qDebug() << num_dossier;
            QStringList ma_liste_de_cle;
            ma_liste_de_cle = obj.keys();
            for(uint8_t i = 0 ; i < ma_liste_de_cle.size() ; i++)
            {
                if(obj[ma_liste_de_cle.at(i)].toString() == ui->lineEditNumDossier->text())
                {
                    ui->comboBoxLit->clear();
                    ui->comboBoxLit->addItem(ma_liste_de_cle[i]);
                    _flagMsgAttendu = 0;
                }

            }
        }
        else
        {
            qDebug() << "Document is not an object" << endl;
        }
    }
    else
    {
        qDebug() << "Invalid JSON...\n" << endl;
    }

}

void MainWindow::on_pushButtonFinHospitalisation_clicked()
{
    qDebug() << ui->comboBoxLit->currentText().toUtf8();
    transmitData(ID_APP_X, ARRET_HOSPITALISATION, ui->comboBoxLit->currentText().toUtf8());
    ui->statusBar->showMessage("Fin d'hospitalisation enregistrée");
    ui->comboBoxLit->clear();
}

void MainWindow::clearDataAdministrative()
{
    ui->lineEditNumeroDossier->clear();
    ui->lineEditNom->clear();
    ui->lineEditPrenom->clear();
    ui->lineEditAdresse->clear();
    ui->lineEditCodePostal->clear();
    ui->lineEditVille->clear();
    ui->spinBoxAge->clear();
    ui->comboBoxSexe->setCurrentText("------");
    ui->spinBoxPoids->clear();
    ui->comboBoxGroupeSanguin->setCurrentText("------");
    ui->dateEditDateEntree->clear();
    ui->comboBoxLit->clear();
}

void MainWindow::onOroffDataAdministrative(bool booleanValue)
{
    ui->lineEditNumeroDossier->setEnabled(booleanValue);
    ui->lineEditNom->setEnabled(booleanValue);
    ui->lineEditPrenom->setEnabled(booleanValue);
    ui->lineEditAdresse->setEnabled(booleanValue);
    ui->lineEditCodePostal->setEnabled(booleanValue);
    ui->lineEditVille->setEnabled(booleanValue);
    ui->spinBoxAge->setEnabled(booleanValue);
    ui->comboBoxSexe->setEnabled(booleanValue);
    ui->spinBoxPoids->setEnabled(booleanValue);
    ui->comboBoxGroupeSanguin->setEnabled(booleanValue);
    ui->dateEditDateEntree->setEnabled(booleanValue);
}

void MainWindow::on_pushButtonChargementDonnees_clicked()
{
    _flagSuiviPatient = 1;
    qDebug() << "Test1";

    QByteArray msg = ui->lineEditNumDossier->text().toUtf8();
    if(ui->comboBoxUtilisateur->currentText() == "Médecin")
    {
        _flagMsgAttendu = 1;
        transmitData(ID_APP_MEDECIN, GET_SUIVI_PATIENT, msg);
        ui->label_gif_chargement->setVisible(TRUE);
    }else if(ui->comboBoxUtilisateur->currentText() == "Infirmier")
    {
        _flagMsgAttendu = 2;
        transmitData(ID_APP_INFIRMIER, GET_SUIVI_PATIENT, msg);
        ui->label_gif_chargement->setVisible(TRUE);
    }else if(ui->comboBoxUtilisateur->currentText() == "Secrétaire")
    {
        ui->statusBar->showMessage("Vous n'avez pas les droits");
    }else if(ui->comboBoxUtilisateur->currentText() == "Famille")
    {
        transmitData(ID_APP_FAMILLE, GET_SUIVI_PATIENT, msg);
        ui->label_gif_chargement->setVisible(TRUE);
        _flagMsgAttendu = 4;
    }
}

void MainWindow::displaySuiviPatient(QJsonDocument doc)
{
    QStringList points_mesures = { "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
                                   "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
                                   "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
                                   "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
                                   "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",};
    int nombre_de_points = 10;
    int compteur = 0;
    QByteArray data;
    QString value;
    QStringList testlist;
    QString DataAsString;
    int hex;
    char valueChar;

    QJsonObject obj;
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            obj = doc.object();
            if(obj["Nombre de points insuffisant"].toString() == "1")
            {
                _flagSuiviPatient = 0;
                ui->statusBar->showMessage("Nombre de points insuffisant");
                ui->label_gif_chargement->setVisible(FALSE);
            }
            for(int i =0 ; i < points_mesures.size() ; ++i)
            {
                qDebug() << obj.value(points_mesures.at(i)).toString();
                value = obj.value(points_mesures.at(i)).toString();
                testlist = value.split("0x");
                qDebug() << testlist;
                for(int i = 1 ; i < (testlist.size()) ; ++i)
                {
                    bool ok;
                    hex = testlist.at(i).toInt(&ok,16);
                    valueChar = (char)hex;
                    data.append(valueChar);
                }

                qDebug() << data;
                QByteArray decodedText;
                if(ui->comboBoxUtilisateur->currentText() == "Médecin")
                    decodedText = _encryption->decode(data, _key16Medecin);
                else if(ui->comboBoxUtilisateur->currentText() == "Infirmier")
                    decodedText = _encryption->decode(data, _key16Infirmier);
                else if(ui->comboBoxUtilisateur->currentText() == "Famille")
                    decodedText = _encryption->decode(data, _key16Famille);
                qDebug() << decodedText;
                DataAsString = QString(decodedText);
                qDebug() << DataAsString.toDouble();
                data.clear();
                if(compteur < 10 )
                {
                    _temperatureSeries->append(compteur, DataAsString.toDouble());
                    compteur = compteur + 1;
                }else if(compteur >= 10 && compteur < 20)
                {
                    _rythmeCardSeries->append((compteur-10), DataAsString.toDouble());
                    compteur = compteur + 1;
                }else if(compteur >= 20 && compteur <30)
                {
                    _freqRespiSeries->append((compteur-20), DataAsString.toDouble());
                    compteur = compteur + 1;
                }else if(compteur >=30 && compteur <40)
                {
                    _tensionSySeries->append((compteur-30), DataAsString.toDouble());
                    compteur = compteur + 1;
                }else if(compteur >=40 && compteur <50)
                {
                    _tensionDiSeries->append((compteur-40), DataAsString.toDouble());
                    compteur = compteur + 1;
                }

                ui->label_gif_chargement->setVisible(FALSE);
            }
        }
        else
        {
            qDebug() << "Document is not an object" << endl;
            ui->label_gif_chargement->setVisible(FALSE);
            ui->statusBar->showMessage("Erreur chargement");
        }
    }
    else
    {
        qDebug() << "Invalid JSON...\n" << endl;
        ui->label_gif_chargement->setVisible(FALSE);
    }

}

void MainWindow::clearChart()
{
    _temperatureSeries->clear();
    _rythmeCardSeries->clear();
    _freqRespiSeries->clear();
    _rythmeCardSeries->clear();
    _tensionDiSeries->clear();
    _tensionSySeries->clear();
}
