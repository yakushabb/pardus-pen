#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QMainWindow>
#include <QColorDialog>

#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include "DrawingWidget.h"
#include "FloatingWidget.h"
#include "FloatingSettings.h"
#include "WhiteBoard.h"
#include "Button.h"

#define _(String) gettext(String)

extern "C" {
#include "settings.h"
}


DrawingWidget *window;
FloatingWidget *floatingWidget;
FloatingSettings *floatingSettings;
WhiteBoard *board;
QMainWindow* mainWindow;

#ifdef screenshot
#include "ScreenShot.h"
QPushButton *ssButton;
#endif


extern void setupWidgets();

extern int screenWidth;
extern int screenHeight;

int pagestatus;

int eraser_status;

int main(int argc, char *argv[]) {

    // Force use X11 or Xwayland
    setenv("QT_QPA_PLATFORM", "xcb",1);

    settings_init();

    // translation part
    const char *systemLanguage = std::getenv("LANG");
    if (systemLanguage != nullptr) {
        bindtextdomain("pardus-pen", "/usr/share/locale");
        setlocale(LC_ALL, systemLanguage);
        textdomain("pardus-pen");
    } else {
        std::cerr << "LANG environment variable not set." << std::endl;
        return 1;
    }

    QApplication app(argc, argv);

    mainWindow = new QMainWindow();
    window = new DrawingWidget();
    board = new WhiteBoard(mainWindow);
    board->setType(get_int((char*)"page"));
    board->setOverlayType(get_int((char*)"page-overlay"));

    window->penSize[PEN] = get_int((char*)"pen-size");
    window->penSize[ERASER] = get_int((char*)"eraser-size");
    window->penSize[MARKER] = get_int((char*)"marker-size");
    window->penType=PEN;
    window->penStyle=SPLINE;
    window->penColor = QColor(get_string((char*)"color"));

    mainWindow->setCentralWidget(window);
    mainWindow->setWindowIcon(QIcon(":tr.org.pardus.pen.svg"));
    mainWindow->setWindowTitle(QString("Pardus Pen"));

    floatingSettings = new FloatingSettings(mainWindow);
    floatingSettings->hide();

    floatingWidget = new FloatingWidget(mainWindow);
    floatingWidget->setSettings(floatingSettings);

    setupWidgets();

    floatingWidget->show();

    mainWindow->setAttribute(Qt::WA_StaticContents);
    mainWindow->setAttribute(Qt::WA_TranslucentBackground, true);
    mainWindow->setAttribute(Qt::WA_NoSystemBackground);
    mainWindow->setAttribute(Qt::WA_AcceptTouchEvents, true);
    mainWindow->setStyleSheet(
        "background: none;"
        "font-size: "+QString::number(screenHeight / 62)+"px;"
    );
    mainWindow->showFullScreen();

    // Create a lambda function to handle geometry changes
    auto handleGeometryChange = [](const QRect &newGeometry){
        (void)newGeometry;
        screenWidth  = newGeometry.width();
        screenHeight = newGeometry.height();
        mainWindow->setFixedSize(screenWidth, screenHeight);
        window->setFixedSize(screenWidth, screenHeight);
        board->setFixedSize(screenWidth, screenHeight);
        floatingWidget->moveAction();
        puts("Screen geometry changed");
    };

    QObject::connect(QGuiApplication::primaryScreen(), &QScreen::geometryChanged,
                     handleGeometryChange);

    window->loadArchive("/home/a/test/ff.tar.gz");
    return app.exec();
}
