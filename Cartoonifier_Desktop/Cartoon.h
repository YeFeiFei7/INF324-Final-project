#pragma once

#include <stdio.h>
#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>

void cartoonifyImage(cv::Mat srcColor, cv::Mat dst, bool sketchMode, bool alienMode, bool evilMode, int debugType);

void drawFaceStickFigure(cv::Mat dst);

void changeFacialSkinColor(cv::Mat smallImgBGR, cv::Mat bigEdges, int debugType);
void removePepperNoise(cv::Mat &mask);
void drawFaceStickFigure(cv::Mat dst);
