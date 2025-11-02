#include <QtGui>
#include <QApplication>
#include <QLabel>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLabel label;
    QImage image;
    if (image.load("lenna.png"))
    {
        for (int indx_row = 0; indx_row < image.height(); indx_row++)
        {
            QRgb* pnt_row = (QRgb*)image.scanLine(indx_row);
            for (int indx_col = 0; indx_col < image.width(); indx_col++)
            {
                QRgb old_pix = pnt_row[indx_col];
                int red = qRed(old_pix);
                int grn = qGreen(old_pix);
                int blu = qBlue(old_pix);
                int grey = 0.2989 * red + 0.5870 * grn + 0.1140 * blu;
                pnt_row[indx_col] = qRgb(grey, grey, grey);

            }
            label.setPixmap(QPixmap::fromImage(image));
        }
    }
    else
    {
        QTextStream(stdout) << "Cannot load image." << Qt::endl;
    }

    label.show();

    return app.exec();
}//main
