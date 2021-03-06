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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFixedSize(size());
    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}

#define NCG_START {
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

        if(!file.isWritable())
            QMessageBox::critical(this, "Write to file failed!", "Please make sure the file isn't being used by another process and make sure you have enough permissons on it.");

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

void MainWindow::on_ncg_height_sb_valueChanged()
{
    if(ui->ncg_is4bpp->isChecked())
        if(ui->ncg_height_sb->value() % 2 != 0)
            ui->ncg_height_sb->setValue(ui->ncg_height_sb->value() + 1);
}

void MainWindow::on_ncg_is4bpp_stateChanged()
{
    if(ui->ncg_height_sb->value() % 2 != 0)
        ui->ncg_height_sb->setValue(ui->ncg_height_sb->value() + 1);

    ui->ncg_height_sb->setSingleStep(ui->ncg_is4bpp->isChecked() + 1);
}
#define NCG_END }

#define NCL_START {
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
        {
            stream << static_cast<quint8>(j);
            stream << static_cast<quint8>(i+j);
        }
    }

    //If NCLR add footer
    if(ui->nclr_cb->isChecked())
    {
        //No PMCP support as MKDS doesn't need it and I couldn't find examples or uses for it
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

        if(!file.isWritable())
            QMessageBox::critical(this, "Write to file failed!", "Please make sure the file isn't being used by another process and make sure you have enough permissons on it.");

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
#define NCL_END }

#define NSC_START {
void MainWindow::on_nsc_saveas_btn_clicked()
{
    //Write file
    QByteArray tempFile;
    QDataStream stream(&tempFile, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    int totalPixels = ui->nsc_width_sb->value() * ui->nsc_height_sb->value();
    int result = totalPixels / 64;
    int dataSize = result * 2;

    //If NSCR add header
    if(ui->nscr_cb->isChecked())
    {
        stream << 0x4E534352; //RCSN
        stream << 0x100FEFF;
        stream << static_cast<quint32>(0x10 + 0x14 + dataSize); //Section Size
        stream << static_cast<quint16>(0x10); //Header Size
        stream << static_cast<quint16>(1); //Number of sub-sections

        stream << 0x5343524E; //NRCS
        stream << static_cast<quint32>(0x14 + dataSize); //Section Size (+Header)
        stream << static_cast<quint16>(ui->nsc_width_sb->value()); //Screen Width
        stream << static_cast<quint16>(ui->nsc_height_sb->value()); //Screen Height
        stream << static_cast<quint32>(0); //Padding
        stream << static_cast<quint32>(dataSize); //Screen Data Size
    }

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
    QString fileName;
    if(!ui->nscr_cb->isChecked())
        fileName = QFileDialog::getSaveFileName(this, "", "", "Nitro Screen (*.nsc);;All Files (*)");
    else
        fileName = QFileDialog::getSaveFileName(this, "", "", "Nitro Screen Resource (*.nscr);;All Files (*)");
    if(fileName == "")
        SkipFileCreation = true;

    if(!SkipFileCreation)
    {
        //Write to actual file
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);

        if(!file.isWritable())
            QMessageBox::critical(this, "Write to file failed!", "Please make sure the file isn't being used by another process and make sure you have enough permissons on it.");

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

void MainWindow::on_nsc_nsmbem_cb_clicked()
{
    int totalPixels = ui->nsc_width_sb->value() * ui->nsc_height_sb->value();
    ui->nsc_width_sb->setEnabled(!ui->nsc_nsmbem_cb->isChecked());

    if(!ui->nsc_nsmbem_cb->isChecked())
    {
        ui->nsc_width_sb->setValue(static_cast<int>(qSqrt(totalPixels)));
        ui->nsc_height_sb->setValue(static_cast<int>(qSqrt(totalPixels)));
    }
    else if(ui->nsc_nsmbem_cb->isChecked())
    {
        ui->nsc_width_sb->setValue(256);
        ui->nsc_height_sb->setValue(totalPixels / 256);
    }
}

void MainWindow::on_nsc_width_sb_valueChanged()
{
    if(ui->nsc_width_sb->value() % 8 != 0)
        ui->nsc_width_sb->setValue(((ui->nsc_width_sb->value() + 8/2) / 8) * 8); //Round to nearest 8

    if(!ui->nsc_nsmbem_cb->isChecked())
        ui->nsc_height_sb->setValue(ui->nsc_width_sb->value());
}

void MainWindow::on_nsc_height_sb_valueChanged()
{
    if(ui->nsc_height_sb->value() % 8 != 0)
        ui->nsc_height_sb->setValue(((ui->nsc_height_sb->value() + 8/2) / 8) * 8); //Round to nearest 8

    if(!ui->nsc_nsmbem_cb->isChecked())
        ui->nsc_width_sb->setValue(ui->nsc_height_sb->value());
}
#define NSC_END }
