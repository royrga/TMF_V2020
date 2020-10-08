#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SPIconfig.h"
#include <qglobal.h>
#include <pigpio.h>
#include <QTimer>
#include <QDebug>

#define RESISTOR    22
#define BOMB        27
#define VACCUM      10
#define LED_RESISTOR 3
#define LED_VACCUM   2

#define RESISTOR_ON     1
#define BOMB_ON         1
#define VACCUM_ON       1
#define LED_VACCUM_ON   1
#define LED_RESISTOR_ON 1

#define RESISTOR_OFF     0
#define BOMB_OFF         0
#define VACCUM_OFF       0
#define LED_VACCUM_OFF   0
#define LED_RESISTOR_OFF 0


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //TODO Crear funcion para inicializar variables
    data_init();
    cfg_Gpios_Coms();
    cfg_Timers();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::data_init()
{
    average_sample_temps    = 0;
    last_average_sample     = 0;
    set_point_temp          = 0;
    control_state           = FALSE;
}

void MainWindow::readTemp()
{
    for(int i=0;i<3; i++)
    {
        int state = spiRead(h[i],buffer,2);
        if( (state == PI_BAD_HANDLE) || (state == PI_BAD_SPI_COUNT) || (state == PI_SPI_XFER_FAILED) )
        {
            qDebug()<< "Error spiRead sensor => " << i+1 ;
        }
        adc_data.buffer[1] = buffer[0];
        adc_data.buffer[0] = buffer[1];
        sample_temp[i] =(double)(adc_data.data.value*0.1);
    }
    qDebug()<<"T1 = "<< sample_temp[0] << " T2 = " << sample_temp[1] << " T3 = " << sample_temp[2];
    average_sample_temps = (sample_temp[0] + sample_temp[1] + sample_temp[2])/3.0;
    ui->lcd_resistance_temp->display((double)average_sample_temps);

//    if(average_sample_temps > last_average_sample)
//    {
//        dir_change = change_up;
//    }
//    else if(average_sample_temps < last_average_sample)
//    {
//        dir_change = change_down;
//    }
//    else
//    {
//        dir_change = no_chamge;
//    }

//    if((last_dir_change != dir_change)&&(dir_change != no_change))
//    {

//    }

    if(control_state) // ON/OFF Control Logic
    {
        if((average_sample_temps >= set_point_temp) && (!rele_timeout))
        {
            //ui->pushButton_preHeat->setEnabled(false);
            gpioWrite(RESISTOR, RESISTOR_OFF); // Resistencia apagada
            rele_timeout = TRUE;
            releTime->start(2000);
        }
        else if((average_sample_temps < set_point_temp) && (!rele_timeout))  //TODO Buscar manera de usar menos releTime
        {
            gpioWrite(RESISTOR, RESISTOR_ON); // Resistencia encendida
            rele_timeout = TRUE;
            releTime->start(2000);
        }
    last_average_sample = average_sample_temps;
    last_dir_change = dir_change;
    }
    else
    {
        //Control OFF
        gpioWrite(RESISTOR, RESISTOR_OFF); // Resistencia apagada
    }
}

void MainWindow::clockfunc()
{

}

void MainWindow::releTimefunc()
{
    //Freq. operation 30 ops/min, 2 seconds delay between operations
    rele_timeout = FALSE;
    releTime->stop();
}

void MainWindow::cfg_Gpios_Coms()
{
    /// GPIO /// Spi Config /// Serial open ///
    if (gpioInitialise() < 0)
    {
        gpioTerminate();
        qDebug("Error GPIO not initialize");
    }
    // OUTPUTS
    gpioSetMode(2,PI_OUTPUT); // Indicador listo para vaciado
    gpioSetMode(3,PI_OUTPUT); // Indicador resistencia encendida
    gpioSetMode(27,PI_OUTPUT); // Encendido bomba
    gpioSetMode(22,PI_OUTPUT); // Encendido resistencia
    gpioSetMode(10,PI_OUTPUT); // Valvula vaciado
    // INPUTS
    gpioSetMode(9,PI_INPUT); // paro emergencia

    gpioWrite(LED_RESISTOR, LED_RESISTOR_OFF);  // Indicador resistencia
    gpioWrite(LED_VACCUM,   LED_VACCUM_OFF);  // Indicador de vacio apagado
    gpioWrite(RESISTOR,     RESISTOR_OFF); // Resistencia apagada
    gpioWrite(BOMB,         BOMB_OFF); // Bomba apagada
    gpioWrite(VACCUM,       VACCUM_OFF); // Valvula de vaciado

    for(int i=0;i<3; i++)
    {
        if (( h[i]=spiOpen(i,SPI_BAUDRATE,SPI_CONFIG) ) < 0)
        {
            gpioTerminate();
            qDebug()<< "ERROR SPI CH" << i+1 <<" no  open";
            if( i==0 ){ ui->radio_termopar1->setChecked(FALSE); }
            if( i==1 ){ ui->radio_termopar_2->setChecked(FALSE); }
            if( i==2 ){ ui->radio_termopar_3->setChecked(FALSE); }
        }
        else
        {
            qDebug()<< "SPI CH" << i+1 << "opened";
            if( i==0 ){ ui->radio_termopar1->setChecked(TRUE); }
            if( i==1 ){ ui->radio_termopar_2->setChecked(TRUE); }
            if( i==2 ){ ui->radio_termopar_3->setChecked(TRUE); }
        }

    }

    if ((fe=serOpen((char*)"/dev/ttyAMA0",9600,(unsigned)0)) < 0)
    {
        gpioTerminate();
        qDebug("ERROR Serial not open");
    }
    else
    {
        qDebug("Serial Opened");
    }
}

void MainWindow::cfg_Timers()
{
    /// Tiempo de muestreo temperatura
    timerT = new QTimer(this);
    connect(timerT,SIGNAL(timeout()),this,SLOT(readTemp()));

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(clockfunc()));

    //// Frequency operation protection timer
    releTime = new QTimer(this);
    connect(timerT,SIGNAL(timeout()),this,SLOT(releTimefunc()));
}

void MainWindow::on_pushButton_on_clicked()
{

    gpioWrite(RESISTOR, RESISTOR_ON); // Resistencia encendida
    rele_timeout = FALSE;
    control_state = TRUE;
    // Get set point of temperature in Automatic or Manual Mode
//    if(ui->tabWidget->currentIndex() == 0) // Manual Mode
//    {
        set_point_temp = ui->spinBox_manual_temp->value();

//    }
//    else if(ui->tabWidget->currentIndex() == 1) // Automatic mode
//    {
//        switch (ui->comboBox_auto_material->currentIndex()) {
//        case 0:
//            set_point_temp = 100;  // Acrilico
//            break;
//        case 1:
//            set_point_temp = 150;  // PET
//            break;
//        case 2:
//            set_point_temp = 200;  // Termoplastico
//            break;
//        default:
//            break;
//        }

//    }
    ui->lcdnumber_SetPoint_temp->setDecMode();
    ui->lcdnumber_SetPoint_temp->setDigitCount(4);
    ui->lcdnumber_SetPoint_temp->display(set_point_temp);
    timerT->start(300); // Start loop readTemp()
    //ui->tabWidget->setEnabled(false);
    ui->pushButton_on->setDisabled(true);
    ui->pushButton_preHeat->setEnabled(false);   // Only for Test


}

void MainWindow::on_pushButton_vacuum_clicked()
{
    gpioWrite(VACCUM,VACCUM_ON); // Valvula de vaciado
    //TODO Implementar timer para apagar en determinado tiempo
//    gpioWrite(LED_RESISTOR, LED_RESISTOR_ON);  // Indicador resistencia
//    gpioWrite(LED_VACCUM,   LED_VACCUM_ON);  // Indicador de vacio apagado
//    gpioWrite(RESISTOR,     RESISTOR_ON); // Resistencia apagada
//    gpioWrite(BOMB,         BOMB_ON); // Bomba apagada
}

void MainWindow::on_pushButton_Stop_clicked()
{
    timerT->stop();
    //ui->tabWidget->setEnabled(true);
    ui->pushButton_on->setEnabled(true);
    ui->pushButton_preHeat->setEnabled(true);
//    gpioWrite(LED_RESISTOR, LED_RESISTOR_OFF);  // Indicador resistencia
//    gpioWrite(LED_VACCUM,   LED_VACCUM_OFF);  // Indicador de vacio apagado
//    gpioWrite(RESISTOR,     RESISTOR_OFF); // Resistencia apagada
//    gpioWrite(BOMB,         BOMB_OFF); // Bomba apagada
//    gpioWrite(VACCUM,       VACCUM_OFF); // Valvula de vaciado
}
