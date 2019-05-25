#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFile"
#include "QString"
#include "QFileDialog"
#include "QThread"
#include "QDebug"
#include "QMessageBox"
#include "QtMath"
#include "lz77.h"

static int NCG_lastSpinBox_height_value;
static int NSC_lastSpinBox_width_value;
static int NSC_lastSpinBox_height_value;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFixedSize(size());
    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

    NCG_lastSpinBox_height_value = ui->ncg_height_sb->value();
    NSC_lastSpinBox_width_value = ui->nsc_width_sb->value();
    NSC_lastSpinBox_height_value = ui->nsc_height_sb->value();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_ncg_saveas_btn_clicked()
{
    //Write file
    QByteArray tempFile;
    QDataStream stream(&tempFile, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    int num1;
    if(!ui->ncg_is4bpp->isChecked())
        num1 = ui->ncg_width_sb->value() * ui->ncg_height_sb->value();
    else
        num1 = ui->ncg_width_sb->value() * (ui->ncg_height_sb->value() / 2);

    for (int index = 0; index < num1; ++index)
    {
        stream << static_cast<quint8>(0);
    }

    //If LZ77 prompt and process file
    if(ui->ncg_lz77_cb->isChecked())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("I have a question for you!");
        msgBox.setText("Do you want to add a LZ77 header to the file?");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int result = msgBox.exec();

        tempFile = lz77::Compress(tempFile, result == QMessageBox::Yes);
    }

    //Start progress animation
    int progress = 0;
    while(true)
    {
        QThread::msleep(1);
        progress++;
        ui->ncg_progressBar->setValue(progress);
        if(progress == 100)
            break;
    }

    QApplication::beep();

    //Choose save directory
    bool SkipFileCreation = false;
    QString fileName = QFileDialog::getSaveFileName(this, "", "", "Nitro Graphic (*.ncg);;All Files (*)");
    if(fileName == "")
        SkipFileCreation = true;

    if(!SkipFileCreation)
    {
        //Write to actual file
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.write(tempFile);
        file.close();
    }

    //End progress animation
    while(true)
    {
        QThread::msleep(1);
        progress--;
        ui->ncg_progressBar->setValue(progress);
        if(progress == 0)
            break;
    }
}

void MainWindow::on_ncl_saveas_btn_clicked()
{
    //Write file
    QByteArray tempFile;
    QDataStream stream(&tempFile, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    for(int i = 0; i < ui->ncl_pal_n_sb->value(); i++)
    {
        stream << static_cast<quint16>(0xFF7Fu);

        if(!ui->ncl_is_extended_cb->isChecked())
        {
            for(int j = 0; j < 15; j++)
                stream << static_cast<quint16>(0x1F7Cu);
        }
        else
        {
            for(int j = 0; j < 255; j++)
                stream << static_cast<quint16>(0x1F7Cu);
        }
    }

    //If LZ77 prompt and process file
    if(ui->ncl_lz77_cb->isChecked())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("I have a question for you!");
        msgBox.setText("Do you want to add a LZ77 header to the file?");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int result = msgBox.exec();

        tempFile = lz77::Compress(tempFile, result == QMessageBox::Yes);
    }

    //Start progress animation
    int progress = 0;
    while(true)
    {
        QThread::msleep(1);
        progress++;
        ui->ncl_progressBar->setValue(progress);
        if(progress == 100)
            break;
    }

    QApplication::beep();

    //Choose save directory
    bool SkipFileCreation = false;
    QString fileName = QFileDialog::getSaveFileName(this, "", "", "Nitro Palette (*.ncl);;All Files (*)");
    if(fileName == "")
        SkipFileCreation = true;

    if(!SkipFileCreation)
    {
        //Write to actual file
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.write(tempFile);
        file.close();
    }

    //End progress animation
    while(true)
    {
        QThread::msleep(1);
        progress--;
        ui->ncl_progressBar->setValue(progress);
        if(progress == 0)
            break;
    }
}

void MainWindow::on_nsc_saveas_btn_clicked()
{
    //Write file
    QByteArray tempFile;
    QDataStream stream(&tempFile, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    int totalPixels = ui->nsc_width_sb->value() * ui->nsc_height_sb->value();
    int result = totalPixels / 64;

    for (int index = 0; index < result; ++index)
    {
      stream << static_cast<quint16>(index);
    }

    //If LZ77 prompt and process file
    if(ui->nsc_lz77_cb->isChecked())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("I have a question for you!");
        msgBox.setText("Do you want to add a LZ77 header to the file?");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int result = msgBox.exec();

        tempFile = lz77::Compress(tempFile, result == QMessageBox::Yes);
    }

    //Start progress animation
    int progress = 0;
    while(true)
    {
        QThread::msleep(1);
        progress++;
        ui->nsc_progressBar->setValue(progress);
        if(progress == 100)
            break;
    }

    QApplication::beep();

    //Choose save directory
    bool SkipFileCreation = false;
    QString fileName = QFileDialog::getSaveFileName(this, "", "", "Nitro Tilemap (*.nsc);;All Files (*)");
    if(fileName == "")
        SkipFileCreation = true;

    if(!SkipFileCreation)
    {
        //Write to actual file
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.write(tempFile);
        file.close();
    }

    //End progress animation
    while(true)
    {
        QThread::msleep(1);
        progress--;
        ui->nsc_progressBar->setValue(progress);
        if(progress == 0)
            break;
    }
}

void MainWindow::calculate_ncg_height()
{
    if(ui->ncg_is4bpp->isChecked())
    {
        if (ui->ncg_height_sb->value() % 2 != 0)
        {
            while(ui->ncg_height_sb->value() % 2 != 0)
            {
                if(NCG_lastSpinBox_height_value < ui->ncg_height_sb->value())
                    ui->ncg_height_sb->setValue(ui->ncg_height_sb->value() + 1);
                else
                    ui->ncg_height_sb->setValue(ui->ncg_height_sb->value() - 1);
            }
        }
    }

    NCG_lastSpinBox_height_value = ui->ncg_height_sb->value();
}

void MainWindow::on_ncg_height_sb_valueChanged()
{
    MainWindow::calculate_ncg_height();
}

void MainWindow::on_ncg_is4bpp_stateChanged()
{
    MainWindow::calculate_ncg_height();
}

void MainWindow::on_nsc_nsmbem_cb_clicked()
{
    if(!ui->nsc_nsmbem_cb->isChecked())
    {
        int totalPixels = ui->nsc_width_sb->value() * ui->nsc_height_sb->value();
        ui->nsc_width_sb->setValue(static_cast<int>(qSqrt(totalPixels)));
        ui->nsc_height_sb->setValue(static_cast<int>(qSqrt(totalPixels)));

        ui->nsc_width_sb->setEnabled(true);
    }
    else if(ui->nsc_nsmbem_cb->isChecked())
    {
        int totalPixels = ui->nsc_width_sb->value() * ui->nsc_height_sb->value();
        ui->nsc_height_sb->setValue(totalPixels / 256);

        ui->nsc_width_sb->setEnabled(false);
        ui->nsc_width_sb->setValue(256);
    }
}

void MainWindow::on_nsc_width_sb_valueChanged()
{
    while(ui->nsc_width_sb->value() % 8 != 0)
    {
        if(NSC_lastSpinBox_width_value < ui->nsc_width_sb->value())
            ui->nsc_width_sb->setValue(ui->nsc_width_sb->value() + 1);
        else
            ui->nsc_width_sb->setValue(ui->nsc_width_sb->value() - 1);
    }

    if(!ui->nsc_nsmbem_cb->isChecked())
    {
        ui->nsc_height_sb->setValue(ui->nsc_width_sb->value());
    }

    NSC_lastSpinBox_width_value = ui->nsc_width_sb->value();
}

void MainWindow::on_nsc_height_sb_valueChanged()
{
    while(ui->nsc_height_sb->value() % 8 != 0)
    {
        if(NSC_lastSpinBox_height_value < ui->nsc_height_sb->value())
            ui->nsc_height_sb->setValue(ui->nsc_height_sb->value() + 1);
        else
            ui->nsc_height_sb->setValue(ui->nsc_height_sb->value() - 1);
    }

    if(!ui->nsc_nsmbem_cb->isChecked())
    {
        ui->nsc_width_sb->setValue(ui->nsc_height_sb->value());
    }

    NSC_lastSpinBox_height_value = ui->nsc_height_sb->value();
}
