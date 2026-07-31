#include <QtWidgets/QApplication>

#include "ui/MainWindow.h"
#include "core/LogLevel.h"
#include "repository/LogWriter.h"

int main(int argc, char *argv[])
{

    LogWriter logger;
    logger.write(LogLevel::Info, "アプリケーションを起動しました。", Q_FUNC_INFO);
    logger.deleteOldLogs();

    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
