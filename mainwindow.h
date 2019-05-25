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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_ncl_saveas_btn_clicked();

    void on_ncg_saveas_btn_clicked();

    void on_ncg_height_sb_valueChanged();

    void on_ncg_is4bpp_stateChanged();

    void calculate_ncg_height();

    void on_nsc_saveas_btn_clicked();

    void on_nsc_nsmbem_cb_clicked();

    void on_nsc_width_sb_valueChanged();

    void on_nsc_height_sb_valueChanged();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
