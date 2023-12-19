// Cartoonifier.cpp
// Cartoonify a video stream using OpenCV.

#include <iostream>
#include <opencv2/opencv.hpp>
#include "cartoon.h"

const int DESIRED_CAMERA_WIDTH = 640;
const int DESIRED_CAMERA_HEIGHT = 480;
const int NUM_STICK_FIGURE_ITERATIONS = 40;
const char *windowName = "Cartoonifier";

bool m_sketchMode = false;
bool m_alienMode = false;
bool m_evilMode = false;
bool m_debugMode = false;

int m_stickFigureIterations = 0;

#if !defined VK_ESCAPE
    #define VK_ESCAPE 0x1B      // Escape character (27)
#endif

void initWebcam(cv::VideoCapture &videoCapture, int cameraNumber) {
    try {   
        videoCapture.open(cameraNumber);
    } catch (cv::Exception &e) {}
    
    if (!videoCapture.isOpened()) {
        std::cerr << "ERROR: Could not access the camera!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Loaded camera " << cameraNumber << "." << std::endl;
}

void onKeypress(char key) {
    switch (key) {
    case 's':
        m_sketchMode = !m_sketchMode;
        std::cout << "Sketch / Paint mode: " << m_sketchMode << std::endl;
        break;
    case 'a':
        m_alienMode = !m_alienMode;
        std::cout << "Alien / Human mode: " << m_alienMode << std::endl;
        if (m_alienMode) {
            m_stickFigureIterations = NUM_STICK_FIGURE_ITERATIONS;
        }
        break;
    case 'e':
        m_evilMode = !m_evilMode;
        std::cout << "Evil / Good mode: " << m_evilMode << std::endl;
        break;
    case 'd':
        m_debugMode = !m_debugMode;
        std::cout << "Debug mode: " << m_debugMode << std::endl;
        break;
    }
}

int main(int argc, char *argv[]) {
    std::cout << "Cartoonifier, by Shervin Emami (www.shervinemami.info), June 2012." << std::endl;
    std::cout << "Converts real-life images to cartoon-like images." << std::endl;
    std::cout << "Compiled with OpenCV version " << CV_VERSION << std::endl;
    std::cout << std::endl;

    std::cout << "Keyboard commands (press in the GUI window):" << std::endl;
    std::cout << "    Esc:  Quit the program." << std::endl;
    std::cout << "    s:    change Sketch / Paint mode." << std::endl;
    std::cout << "    a:    change Alien / Human mode." << std::endl;
    std::cout << "    e:    change Evil / Good character mode." << std::endl;
    std::cout << "    d:    change debug mode." << std::endl;
    std::cout << std::endl;

    int cameraNumber = 0;
    if (argc > 1) {
        cameraNumber = atoi(argv[1]);
    }

    cv::VideoCapture camera;
    initWebcam(camera, cameraNumber);

    camera.set(CV_CAP_PROP_FRAME_WIDTH, DESIRED_CAMERA_WIDTH);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, DESIRED_CAMERA_HEIGHT);

    cv::namedWindow(windowName);

    while (true) {
        cv::Mat cameraFrame;
        camera >> cameraFrame;

        if (cameraFrame.empty()) {
            std::cerr << "ERROR: Couldn't grab the next camera frame." << std::endl;
            exit(EXIT_FAILURE);
        }

        cv::Mat displayedFrame = cv::Mat(cameraFrame.size(), CV_8UC3);

        int debugType = 0;
        if (m_debugMode) {
            debugType = 2;
        }

        cartoonifyImage(cameraFrame, displayedFrame, m_sketchMode, m_alienMode, m_evilMode, debugType);

        if (m_stickFigureIterations > 0) {
            drawFaceStickFigure(displayedFrame);
            m_stickFigureIterations--;
        }

        imshow(windowName, displayedFrame);

        char keypress = cv::waitKey(20);
        if (keypress == VK_ESCAPE) {
            break;
        }

        if (keypress > 0) {
            onKeypress(keypress);
        }
    }

    return EXIT_SUCCESS;
}
