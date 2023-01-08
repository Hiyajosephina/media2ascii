#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgproc.hpp>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#else
#include <sys/ioctl.h>
#endif

using namespace std;
using namespace cv;

const float nums[] = {
    0,      0.0751, 0.0829, 0.0848, 0.1227, 0.1403, 0.1559, 0.185,  0.2183,
    0.2417, 0.2571, 0.2852, 0.2902, 0.2919, 0.3099, 0.3192, 0.3232, 0.3294,
    0.3384, 0.3609, 0.3619, 0.3667, 0.3737, 0.3747, 0.3838, 0.3921, 0.396,
    0.3984, 0.3993, 0.4075, 0.4091, 0.4101, 0.42,   0.423,  0.4247, 0.4274,
    0.4293, 0.4328, 0.4382, 0.4385, 0.442,  0.4473, 0.4477, 0.4503, 0.4562,
    0.458,  0.461,  0.4638, 0.4667, 0.4686, 0.4693, 0.4703, 0.4833, 0.4881,
    0.4944, 0.4953, 0.4992, 0.5509, 0.5567, 0.5569, 0.5591, 0.5602, 0.5602,
    0.565,  0.5776, 0.5777, 0.5818, 0.587,  0.5972, 0.5999, 0.6043, 0.6049,
    0.6093, 0.6099, 0.6465, 0.6561, 0.6595, 0.6631, 0.6714, 0.6759, 0.6809,
    0.6816, 0.6925, 0.7039, 0.7086, 0.7235, 0.7302, 0.7332, 0.7602, 0.7834,
    0.8037, 0.9999};
const int numElements = sizeof(nums) / sizeof(nums[0]);

const string density = " `.-':_,^=;><+!rc*/"
                       "z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]"
                       "2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";
void get_terminal_size(int &width, int &height) {
#ifdef WIN32 // get terminal width/height on Windows
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    height = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
#else // get terminal width/height on Unix
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    width = (int)(w.ws_col);
    height = (int)(w.ws_row);
#endif
}

void Clear() {
#ifdef WIN32
    system("cls");
#else
    system("clear");
#endif
}

int get_ascii_char_index(float brightness) {
    int l = 0, r = numElements;
    while (l < r) {
        int m = (l + r) / 2;
        if (nums[m] < brightness)
            l = m + 1;
        else
            r = m;
    }
    if (nums[l] == brightness)
        return l;
    else
        return l - 1;
}

int main(int argc, char *argv[]) {
    VideoCapture cap(0);
    int width, height;
    while (true) {
        Mat caprtured_frame, scaled_frame, flipped_frame, final_frame;
        get_terminal_size(width, height);
        cap.read(caprtured_frame);
        resize(caprtured_frame, scaled_frame, Size(width, height),
               INTER_LINEAR);
        flip(scaled_frame, flipped_frame, 1);
        cvtColor(flipped_frame, final_frame, COLOR_BGR2GRAY);
        double min, max;
        minMaxIdx(final_frame, &min, &max);
        float difference = max - min;
        char ascii_matrix[height][width];

        for (size_t i = 0; i < height; i++) {
            for (size_t j = 0; j < width; j++) {
                int density_index = get_ascii_char_index(
                    (final_frame.at<uint8_t>(i, j) - min) / difference);
                ascii_matrix[i][j] = density[density_index];
            }
        }
        Clear();
        for (auto &row : ascii_matrix) {
            for (auto &character : row) {
                cout << character;
            }
            cout << endl;
        }
    }
    return 0;
}
