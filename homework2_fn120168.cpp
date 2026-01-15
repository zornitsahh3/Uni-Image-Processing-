#include <QLabel>
#include <QTextStream>
#include <QString>

#include <cmath>

using namespace std;

const int INTENS_MIN = 0;
const int INTENS_MAX = 255;

const double EPS = 1.0e-14;
const double ETA_THRESHOLD = 0.5;  // Threshold for separability measure
const int MIN_IMAGE_SIZE = 32;      // Minimum size to continue recursion
//ETA_THRESHOLD: If ? (eta) ? 0.5, the region is considered "separable enough" to apply Otsu directly
//MIN_IMAGE_SIZE: If a sub-image is smaller than 32?32 pixels, stop recursion and apply Otsu directly

const QString file_name = "gray_lenna.png";

//Computes the normalized histogram (probability distribution) of pixel intensities for a given rectangular region
void calcHisto(QImage &image, int x1, int y1, int x2, int y2, double histo[])
{
    for (int i = 0; i <= INTENS_MAX; i++)
    {
        histo[i] = 0;
    }
    int numb_pix = 0;
    for (int indx_row = y1; indx_row < y2; indx_row++)
    {
        if (indx_row >= 0 && indx_row < image.height())
        {
            quint8* ptr_row = (quint8*)(image.bits() 
                    + indx_row * image.bytesPerLine());
            for (int indx_col = x1; indx_col < x2; indx_col++)
            {
                if (indx_col >= 0 && indx_col < image.width())
                {
                    histo[ptr_row[indx_col]]++;
                    numb_pix++;
                }
            }
        }
    }
    if (numb_pix > 0)
    {
        for (int i = 0; i <= INTENS_MAX; i++)
        {
            histo[i] /= numb_pix;
        }
    }
}//calcHisto

//Computes the total variance of the intensity distribution
double calculateTotalVariance(const double histo[])
{
    // First calculate the global mean
    double m_g = 0.0;
    for (int i = 0; i <= INTENS_MAX; i++)
    {
        m_g += i * histo[i];
    }
    
    // Then calculates total variance which uses the global mean ??_T = ?(i - ?_T)? ? p_i
    double sigma_T = 0.0;
    for (int i = 0; i <= INTENS_MAX; i++)
    {
        double diff = i - m_g;
        sigma_T += diff * diff * histo[i];
    }
    
    return sigma_T;
}//calculateTotalVariance

//Calculate Separability Measure ?, where ? = ??_B / ??_T;
//`??_B` = between-class variance (how well the threshold separates classes)
//`??_T` = total variance (overall spread of intensities)

double calculateEta(const double histo[], int threshold)
{
    // Calculate cumulative sums
    double p_1[INTENS_MAX + 1] = {0};
    p_1[0] = histo[0];
    for (int i = 1; i <= INTENS_MAX; i++)
    {
	//Calculates cumulative probability `p_1[i]` = probability of intensity ? i
        p_1[i] = p_1[i - 1] + histo[i];
    }

    // Cumulative mean
    double m[INTENS_MAX + 1] = {0};
    for (int i = 1; i <= INTENS_MAX; i++)
    {
	//Calculates cumulative mean `m[i]` = mean intensity up to level i
        m[i] = m[i - 1] + i * histo[i];
    }

    // Global mean
    double m_g = m[INTENS_MAX];

    // Between-class variance for given threshold
    double div = (p_1[threshold] * (1 - p_1[threshold]));
    double sigma_B = 0.0;
    if (fabs(div) >= EPS)
    {
	//??_B = (m_g ? p_1[t] - m[t])? / (p_1[t] ? (1 - p_1[t]))
        double diff = m_g * p_1[threshold] - m[threshold];
        sigma_B = (diff * diff) / div;
    }

    // Total variance
    double sigma_T = calculateTotalVariance(histo);

    // Eta = ??_B / ??_T
    if (fabs(sigma_T) < EPS)
    {
        return 0.0;
    }
    
    return sigma_B / sigma_T;
}//calculateEta

int otsu(const double histo[])
{
    // compute cumulative sums
    double p_1[INTENS_MAX + 1] = {0};
    p_1[0] = histo[0];
    for (int i = 1; i <= INTENS_MAX; i++)
    {
        p_1[i] = p_1[i - 1] + histo[i];
    }

    // cumulative mean
    double m[INTENS_MAX + 1] = {0};
    for (int i = 1; i <= INTENS_MAX; i++)
    {
        m[i] = m[i - 1] + i * histo[i];
    }

    // global mean
    double m_g = m[INTENS_MAX];

    // between-class
    double b_c[INTENS_MAX + 1] = {0};
    for (int i = 1; i <= INTENS_MAX; i++)
    {
        double div = (p_1[i] * (1 - p_1[i]));
        b_c[i] = 
            fabs(div < EPS) ? 0 :
            ((m_g * p_1[i] - m[i]) * (m_g * p_1[i] - m[i])) / div;
    }

    // find max
    double max = 0;
    int max_i = 0;
    for (int i = 0; i <= INTENS_MAX; i++)
    {
        if (b_c[i] > max)
        {
            max = b_c[i];
            max_i = i;
        }
    }

    return max_i;
}//otsu

void applyThreshold(QImage &image, int x1, int y1, int x2, int y2, int threshold)
{
    for (int indx_row = y1; indx_row < y2; indx_row++)
    {
        if (indx_row >= 0 && indx_row < image.height())
        {
            quint8* ptr_row = (quint8*)(image.bits()
                    + indx_row * image.bytesPerLine());
            for (int indx_col = x1; indx_col < x2; indx_col++)
            {
                if (indx_col >= 0 && indx_col < image.width())
                {
                    ptr_row[indx_col] = 
                        (ptr_row[indx_col] > threshold) ? INTENS_MAX : INTENS_MIN;
                }
            }
        }
    }
}//applyThreshold

void adaptiveOtsu(QImage &image, int x1, int y1, int x2, int y2, int depth = 0)
{
    int width = x2 - x1;
    int height = y2 - y1;
    
    // Check if sub-image is too small
    if (width < MIN_IMAGE_SIZE || height < MIN_IMAGE_SIZE)
    {
        QTextStream(stdout) << "Sub-image too small (" << width << "x" << height
                           << "), applying Otsu directly" << Qt::endl;
        double histo[INTENS_MAX + 1];
        calcHisto(image, x1, y1, x2, y2, histo);
        int th = otsu(histo);
        applyThreshold(image, x1, y1, x2, y2, th);
        return;
    }

    // Calculate histogram for this sub-image
    double histo[INTENS_MAX + 1];
    calcHisto(image, x1, y1, x2, y2, histo);
    
    // Find optimal threshold
    int optimal_th = otsu(histo);
    
    // Calculate separability measure eta
    double eta = calculateEta(histo, optimal_th);
    
    QTextStream(stdout) << "Depth " << depth << ": Region [" << x1 << "," << y1 
                       << "] to [" << x2 << "," << y2 << "] - Eta: " << eta
                       << ", Threshold: " << optimal_th << Qt::endl;

    // If eta is big enough, apply threshold directly
    if (eta >= ETA_THRESHOLD)
    {
        QTextStream(stdout) << "Eta >= threshold, applying Otsu directly" << Qt::endl;
        applyThreshold(image, x1, y1, x2, y2, optimal_th);
        return;
    }

    // Otherwise, divide image and recurse
    QTextStream(stdout) << "Eta < threshold, dividing image..." << Qt::endl;
    
    // Divide horizontally or vertically (choose larger dimension)
    if (width > height)
    {
        // Divide vertically (split width)
        int mid_x = x1 + width / 2;
        adaptiveOtsu(image, x1, y1, mid_x, y2, depth + 1);
        adaptiveOtsu(image, mid_x, y1, x2, y2, depth + 1);
    }
    else
    {
        // Divide horizontally (split height)
        int mid_y = y1 + height / 2;
        adaptiveOtsu(image, x1, y1, x2, mid_y, depth + 1);
        adaptiveOtsu(image, x1, mid_y, x2, y2, depth + 1);
    }
}//adaptiveOtsu

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QImage image;
    QLabel label;
    if (image.load(file_name))
    {
        QTextStream(stdout) << "Image loaded: " << file_name << Qt::endl;
        QTextStream(stdout) << "Format: " << image.format() << Qt::endl;
        QTextStream(stdout) << "Size: " << image.width() << "x" << image.height() << Qt::endl;

        if (image.format() == QImage::Format_Grayscale8)
        {
            QTextStream(stdout) << "Starting adaptive Otsu binarization..." << Qt::endl;
            QTextStream(stdout) << "Eta threshold: " << ETA_THRESHOLD << Qt::endl;
            QTextStream(stdout) << "Min image size: " << MIN_IMAGE_SIZE << Qt::endl;
            
            adaptiveOtsu(image, 0, 0, image.width(), image.height());
            
            QTextStream(stdout) << "Adaptive Otsu binarization completed!" << Qt::endl;
        }
        else
        {
            QTextStream(stdout) << "Image is not grayscale format!" << Qt::endl;
        }
             
        label.setPixmap(QPixmap::fromImage(image));
    }
    else
    {
        QTextStream(stdout) << "Cannot load image: " << file_name << Qt::endl;
    }

    label.show();

    return app.exec();
}//main

