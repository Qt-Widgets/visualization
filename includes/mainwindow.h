#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMenu>
#include <QTableWidget>
#include <QMainWindow>
#include "visusignal.h"
#include "visuconfiguration.h"
#include "wysiwyg/editsignal.h"
class Stage;

namespace Ui {
class MainWindow;
}

enum DRAG_ORIGIN
{
    TOOLBAR = 0,
    STAGE
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString xmlPath, QWidget *parent = 0);
    void setupToolbarWidgets(QWidget* toolbar);
    VisuSignal* getSignal();
    void setActiveWidget(VisuWidget* widget);
    bool dragOriginIsToolbar(QString originObjectName);
    VisuWidget* getActiveWidget()
    {
        return mActiveWidget;
    }
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

    ~MainWindow();

public slots:
    void cellUpdated(int row, int col);

private:
    Ui::MainWindow *ui;
    VisuSignal* mDefaultSignal;

    QWidget* mToolbar;
    Stage* mStage;
    QTableWidget* mPropertiesTable;
    VisuWidget* mActiveWidget;
    VisuConfiguration* configuration;
    EditSignal* editSignalWindow;
    QMenu* mSignalsMenu;

    void setupMenu();
    void updateMenuSignalList();
    void mapToString(QMap<QString, QString> properties);

private slots:
    void openConfiguration();
    void saveConfiguration();
    void openSignalsEditor();
};

#endif // MAINWINDOW_H
