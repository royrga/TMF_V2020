#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void data_init();
    void readTemp();
    void clockfunc();
    void releTimefunc();

    void on_pushButton_on_clicked();

    void on_pushButton_vacuum_clicked();

    void on_pushButton_Stop_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QTimer *timerT;
    QTimer *releTime;

    typedef enum directionState{
        change_up,
        change_down,\
        no_chamge
    }directionState;

    // Sampling temperature variables
    directionState dir_change;
    directionState last_dir_change;
    int sample_temp[3];
    float average_sample_temps;
    float last_average_sample;
    float set_point_temp;



    // Rele protection variables
    bool rele_timeout; // FALSE => Rele state can change, TRUE => Rele state can not change

    // Temperature Control Variables
    bool control_state; //TRUE => Control ON, FALSE => Control OFF

    // SPI variable
    typedef union
    {
        struct
        {
            unsigned short int NULB : 2;
            unsigned short int value: 12;
            unsigned short int DONT : 2;
        }data;
        unsigned char buffer[2];
    }Int16;
    char buffer[2];
    Int16 adc_data;
    int fd,fe;
    //handler SPI
    int h[3];

    // Methods to setup
    void cfg_Gpios_Coms();
    void cfg_Timers();
};

#endif // MAINWINDOW_H
