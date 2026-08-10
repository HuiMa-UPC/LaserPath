#include <QApplication>
#include "MainWindow.h"
#include <QFont>
#include <QFontDatabase>
#include <QDebug>

int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	QApplication app(argc, argv);

    int idReg = QFontDatabase::addApplicationFont(":/fonts/MiSans_Regular");
    int idMed = QFontDatabase::addApplicationFont(":/fonts/MiSans_Medium");
    int idBold = QFontDatabase::addApplicationFont(":/fonts/MiSans_Bold");

    QString miSansFamily;
    if (idReg != -1)
    {
        miSansFamily = QFontDatabase::applicationFontFamilies(idReg).first();
        qDebug() << "MiSans familyName:" << miSansFamily;
    }
    else
    {
        qWarning() << "MiSans‑Regular 加载失败，使用系统默认字体";
        miSansFamily = qApp->font().family();
    }
    QFont globalFont(miSansFamily, 11);
    globalFont.setStyleHint(QFont::SansSerif);
    app.setFont(globalFont);

	MainWindow mianWindow;

	mianWindow.show();

	return app.exec();
}
