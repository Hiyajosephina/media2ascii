#include <cmath>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <string>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#else
#include <sys/ioctl.h>
#endif

using std::cout;
using std::endl;
using std::string;
using namespace cv;

const float NUMS[] = {
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
const int NUM_ELEMENTS = sizeof(NUMS) / sizeof(NUMS[0]);

const string DENSITY = " `.-':_,^=;><+!rc*/"
                       "z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]"
                       "2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";

const int INITIAL_TOLERANCE = 15;
const int INITIAL_MODE = 0;
const bool INITAL_HAS_COLOR = true;

void get_terminal_size(int &, int &);
void Clear();
int get_ascii_char_index(float);
void convert_to_ascii(Mat);

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
    int l = 0, r = NUM_ELEMENTS;
    while (l < r) {
        int m = (l + r) >> 1;
        if (NUMS[m] < brightness)
            l = m + 1;
        else
            r = m;
    }
    if (NUMS[l] == brightness)
        return l;
    else
        return l - 1;
}

bool is_number(string number) {
    for (int i = 0; number[i] != 0; i++) {
        if (!isdigit(number[i]))
            return false;
    }
    return true;
}

void convert_to_ascii(Mat frame0, uint8_t brightness_tolerance,
                      bool has_color) {
    int width, height;
    Mat frame1;
    get_terminal_size(width, height);
    resize(frame0, frame1, Size(width, height), INTER_LINEAR);
    flip(frame1, frame0, 1);
    cvtColor(frame0, frame1, COLOR_BGR2GRAY);
    double min, max;
    minMaxIdx(frame1, &min, &max);
    float difference = max - min;
    // keep track of last brightness value and its associated char to reduce
    // get_ascii_char_index() function calls for adjacent, similar(with
    // regards to a tolerance) pixels and also keep track of last ansi color
    uint8_t last_brigthness = 0;
    char last_char = ' ';
    int r = 0, g = 0, b = 0;
    string line = "";
    Clear();
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            uint8_t curr_brightness = frame1.at<uint8_t>(i, j);
            if (abs(curr_brightness - last_brigthness) < brightness_tolerance) {
                line += last_char;
                continue;
            }
            if (!has_color) {
                cout << line;
            } else {
                cout << "\033[38;2;" << r << ";" << g << ";" << b << "m" << line
                     << "\033[0m";
            }
            int density_index =
                get_ascii_char_index((curr_brightness - min) / difference);
            last_char = DENSITY[density_index];
            last_brigthness = curr_brightness;
            Vec3b pixel = frame0.at<Vec3b>(i, j);
            r = pixel[2];
            g = pixel[1];
            b = pixel[0];
            line = last_char;
        }
        line += '\n';
    }
}

int main(int argc, char *argv[]) {
    int tolerance = INITIAL_TOLERANCE;
    int mode = INITIAL_MODE;
    bool has_color = INITAL_HAS_COLOR;
    string path;
    for (int i = 1; i < argc; i++) {
        if (i < argc - 1 && (strcmp(argv[i], "--tolerance") == 0 ||
                             strcmp(argv[i], "-t") == 0)) {
            string tol_string = argv[i + 1];
            if (!is_number(tol_string) || stoi(tol_string) > 255) {
                cout << "Please enter a valid tolerance (0-255)" << endl;
                return 1;
            }
            tolerance = stoi(tol_string);
        }
        if (strcmp(argv[i], "--webcam") == 0 || strcmp(argv[i], "-w") == 0) {
            mode = 0;
        }
        if (i < argc - 1 &&
            (strcmp(argv[i], "--image") == 0 || strcmp(argv[i], "-i") == 0)) {
            path = argv[i + 1];
            mode = 1;
        }
        if (i < argc - 1 &&
            (strcmp(argv[i], "--video") == 0 || strcmp(argv[i], "-v") == 0)) {
            path = argv[i + 1];
            mode = 2;
        }
        if (strcmp(argv[i], "--no-color") == 0) {
            has_color = false;
        }
    }
    if (mode == 0) {
        VideoCapture cap(0);
        while (true) {
            Mat captured_frame;
            cap.read(captured_frame);
            convert_to_ascii(captured_frame, tolerance, has_color);
        }
        return 0;
    }
    if (mode == 1) {
        Mat captured_frame = imread(path, IMREAD_COLOR);
        if (captured_frame.empty()) {
            cout << "Could not read the image: " << path << endl;
            return 1;
        }
        convert_to_ascii(captured_frame, tolerance, has_color);
        return 0;
    }
    if (mode == 2) {
        VideoCapture cap(path);
        while (true) {
            Mat captured_frame;
            cap.read(captured_frame);
            if (captured_frame.empty())
                break;
            convert_to_ascii(captured_frame, tolerance, has_color);
        }
        return 0;
    }
    return 1;
}
