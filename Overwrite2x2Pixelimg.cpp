                                                                                    outimg.cpp#include <QImage>
#include <QPixmap>
#include <QRgb>
#include <QApplication>
#include <QLabel>

const int IMG_SIZE = 300;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QImage image(2, 2, QImage::Format_RGB32);
    QRgb value;

    value = qRgb(255, 204, 204);    // ffcccc
    image.setPixel(0, 0, value);
    value = qRgb(229, 255, 204);    // e5ffcc
    image.setPixel(0, 1, value);
    value = qRgb(102, 102, 255);    // 6666ff
    image.setPixel(1, 0, value);
    value = qRgb(255, 255, 0);      // ffff00
    image.setPixel(1, 1, value);

    QLabel label;
    QPixmap scaled(
            QPixmap::fromImage(image).scaled(
                IMG_SIZE, IMG_SIZE,
                Qt::IgnoreAspectRatio,
                Qt::FastTransformation
                )
            );
    label.setPixmap(scaled);
    label.show();

    image.save("lenna.png");

    return app.exec();
}//main
