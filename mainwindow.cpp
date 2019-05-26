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

    int totalPixels = ui->ncg_width_sb->value() * ui->ncg_height_sb->value();
    if(ui->ncg_is4bpp->isChecked())
        totalPixels = ui->ncg_width_sb->value() * (ui->ncg_height_sb->value() / 2);

    //If NCGR add header
    if(ui->ncgr_cb->isChecked())
    {
        stream << 0x4E434752; //RGCN
        stream << 0x100FEFF;
        stream << static_cast<quint32>(0x40 + totalPixels); //Section Size
        stream << static_cast<quint16>(0x10); //Header Size
        stream << static_cast<quint16>(2); //Number of sub-sections

        stream << 0x43484152; //RAHC
        stream << static_cast<quint32>(0x20 + totalPixels); //Section Size (+Header)
        stream << static_cast<quint16>(totalPixels / 1024); //Tile Count
        stream << static_cast<quint16>(0x20); //Tile Size
        stream << static_cast<quint32>(4 - ui->ncg_is4bpp->isChecked()); //Tile Bit Depth (is4bpp)
        stream << static_cast<quint64>(0); //Padding?
        stream << static_cast<quint32>(totalPixels); //Tile Data Size
        stream << static_cast<quint32>(0x24); //Tile Data Size
    }

    for (int index = 0; index < totalPixels; ++index)
    {
        stream << static_cast<quint8>(0);
    }

    //If NCGR add footer
    if(ui->ncgr_cb->isChecked())
    {
        stream << 0x43504F53; //SOPC
        stream << 0x10; //Section Size
        stream << 0; //Padding
        stream << static_cast<quint16>(0x20); //Tile Size?
        stream << static_cast<quint16>(totalPixels / 1024); //Tile Count
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
    QString fileName;
    if(!ui->ncgr_cb->isChecked())
        fileName = QFileDialog::getSaveFileName(this, "", "", "Nitro Character Graphic (*.ncg);;All Files (*)");
    else
        fileName = QFileDialog::getSaveFileName(this, "", "", "Nitro Character Graphic Resource (*.ncgr);;All Files (*)");
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

    int colorsPerPalette = 16;
    if(ui->ncl_is_extended_cb->isChecked())
        colorsPerPalette = 256;

    int paletteDataSize = colorsPerPalette * ui->ncl_pal_n_sb->value() * 2;

    //If NCLR add header
    if(ui->nclr_cb->isChecked())
    {
        stream << 0x4E434C52; //RLCN
        stream << 0x100FEFF;
        stream << static_cast<quint32>(0x28 + paletteDataSize); //Section Size
        stream << static_cast<quint16>(0x10); //Header Size
        stream << static_cast<quint16>(1); //Number of sub-sections

        stream << 0x504C5454; //TTLP
        stream << static_cast<quint32>(0x18 + paletteDataSize); //Section Size (+Header)
        stream << static_cast<quint32>(3); //Palette Bit Depth
        stream << static_cast<quint32>(0); //Padding?
        stream << static_cast<quint32>(paletteDataSize); //Palette Data Size
        stream << static_cast<quint32>(colorsPerPalette); //Colors Per Palette
    }

    for(int i = 0; i < ui->ncl_pal_n_sb->value(); i++)
    {
        stream << static_cast<quint16>(0xFF7F);

        for(int j = 1; j < colorsPerPalette; j++)
            stream << static_cast<quint16>(0x1F7C);
    }

    //NO FOOTER FOR NCLR help

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
    QString fileName;
    if(!ui->nclr_cb->isChecked())
        fileName = QFileDialog::getSaveFileName(this, "", "", "Nitro Color (*.ncl);;All Files (*)");
    else
        fileName = QFileDialog::getSaveFileName(this, "", "", "Nitro Color Resource (*.nclr);;All Files (*)");
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
