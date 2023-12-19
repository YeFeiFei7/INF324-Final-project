#include "cartoon.h"
#include "ImageUtils.h" 

void cartoonifyImage(cv::Mat srcColor, cv::Mat dst, bool sketchMode, bool alienMode, bool evilMode, int debugType)
{
    cv::Mat srcGray;
    cv::cvtColor(srcColor, srcGray, cv::COLOR_BGR2GRAY);

    cv::medianBlur(srcGray, srcGray, 7);

    cv::Size size = srcColor.size();
    cv::Mat mask = cv::Mat(size, CV_8U);
    cv::Mat edges = cv::Mat(size, CV_8U);
    if (!evilMode) {
        cv::Laplacian(srcGray, edges, CV_8U, 5);
        cv::threshold(edges, mask, 80, 255, cv::THRESH_BINARY_INV);
        removePepperNoise(mask);
    } else {
        // Evil mode, making everything look like a scary bad guy.
        cv::Mat edges2;
        cv::Scharr(srcGray, edges, CV_8U, 1, 0);
        cv::Scharr(srcGray, edges2, CV_8U, 1, 0, -1);
        edges += edges2;
        cv::threshold(edges, mask, 12, 255, cv::THRESH_BINARY_INV);
        cv::medianBlur(mask, mask, 3);
    }

    // sketch mode
    if (sketchMode) {
        cv::cvtColor(mask, dst, cv::COLOR_GRAY2BGR);
        return;
    }

    // Do the bilateral filtering at a shrunken scale
    cv::Size smallSize;
    smallSize.width = size.width / 2;
    smallSize.height = size.height / 2;
    cv::Mat smallImg = cv::Mat(smallSize, CV_8UC3);
    cv::resize(srcColor, smallImg, smallSize, 0, 0, cv::INTER_LINEAR);

    // Perform many iterations of weak bilateral filtering
    cv::Mat tmp = cv::Mat(smallSize, CV_8UC3);
    int repetitions = 7; // Repetitions for a strong cartoon effect.
    for (int i = 0; i < repetitions; i++) {
        int size = 9;          // Filter size. Has a large effect on speed.
        double sigmaColor = 9; // Filter color strength.
        double sigmaSpace = 7; // Positional strength. Effects speed.
        cv::bilateralFilter(smallImg, tmp, size, sigmaColor, sigmaSpace);
        cv::bilateralFilter(tmp, smallImg, size, sigmaColor, sigmaSpace);
    }

    if (alienMode) {
        // Apply an "alien" filter
        changeFacialSkinColor(smallImg, edges, debugType);
    }

    // Go back to the original scale.
    cv::resize(smallImg, srcColor, size, 0, 0, cv::INTER_LINEAR);

    // Clear the output image to black
    memset((char *)dst.data, 0, dst.step * dst.rows);

    // Blurry cartoon image
    srcColor.copyTo(dst, mask);
}

// Apply an "alien" filter, when given a shrunken BGR image and the full-res edge mask.
// Detects the color of the pixels in the middle of the image, then changes the color of that region to green.
void changeFacialSkinColor(cv::Mat smallImgBGR, cv::Mat bigEdges, int debugType)
{
    // Convert to Y'CrCb color-space
    cv::Mat yuv = cv::Mat(smallImgBGR.size(), CV_8UC3);
    cv::cvtColor(smallImgBGR, yuv, cv::COLOR_BGR2YCrCb);

    int sw = smallImgBGR.cols;
    int sh = smallImgBGR.rows;
    cv::Mat maskPlusBorder = cv::Mat::zeros(sh + 2, sw + 2, CV_8U);
    cv::Mat mask = maskPlusBorder(cv::Rect(1, 1, sw, sh)); // mask is an ROI in maskPlusBorder.
    cv::resize(bigEdges, mask, smallImgBGR.size());

    cv::threshold(mask, mask, 80, 255, cv::THRESH_BINARY);
    // Connect the edges together if there was a pixel gap between them.
    cv::dilate(mask, mask, cv::Mat());
    cv::erode(mask, mask, cv::Mat());
    // imshow("constraints for floodFill", mask);

    int const NUM_SKIN_POINTS = 6;
    cv::Point skinPts[NUM_SKIN_POINTS];
    skinPts[0] = cv::Point(sw / 2, sh / 2 - sh / 6);
    skinPts[1] = cv::Point(sw / 2 - sw / 11, sh / 2 - sh / 6);
    skinPts[2] = cv::Point(sw / 2 + sw / 11, sh / 2 - sh / 6);
    skinPts[3] = cv::Point(sw / 2, sh / 2 + sh / 16);
    skinPts[4] = cv::Point(sw / 2 - sw / 9, sh / 2 + sh / 16);
    skinPts[5] = cv::Point(sw / 2 + sw / 9, sh / 2 + sh / 16);
    // Skin might be fairly dark or slightly less colorful.
    // Skin might be very bright or slightly more colorful but not much more blue.
    const int LOWER_Y = 60;
    const int UPPER_Y = 80;
    const int LOWER_Cr = 25;
    const int UPPER_Cr = 15;
    const int LOWER_Cb = 20;
    const int UPPER_Cb = 15;
    cv::Scalar lowerDiff = cv::Scalar(LOWER_Y, LOWER_Cr, LOWER_Cb);
    cv::Scalar upperDiff = cv::Scalar(UPPER_Y, UPPER_Cr, UPPER_Cb);

    cv::Mat edgeMask = mask.clone(); // Keep a duplicate copy of the edge mask.
    for (int i = 0; i < NUM_SKIN_POINTS; i++) {
        const int flags = 4 | cv::FLOODFILL_FIXED_RANGE | cv::FLOODFILL_MASK_ONLY;
        cv::floodFill(yuv, maskPlusBorder, skinPts[i], cv::Scalar(), NULL, lowerDiff, upperDiff, flags);
        if (debugType >= 1)
            cv::circle(smallImgBGR, skinPts[i], 5, CV_RGB(0, 0, 255), 1, cv::LINE_AA);
    }
    if (debugType >= 2)
        cv::imshow("flood mask", mask * 120); // Draw the edges as white and the skin region as grey.

    mask -= edgeMask;

    int Red = 0;
    int Green = 70;
    int Blue = 0;
    cv::add(smallImgBGR, cv::Scalar(Blue, Green, Red), smallImgBGR, mask);
}

void removePepperNoise(cv::Mat &mask)
{
    for (int y = 2; y < mask.rows - 2; y++) {
        // Get access to each of the 5 rows near this pixel.
        uchar *pThis = mask.ptr(y);
        uchar *pUp1 = mask.ptr(y - 1);
        uchar *pUp2 = mask.ptr(y - 2);
        uchar *pDown1 = mask.ptr(y + 1);
        uchar *pDown2 = mask.ptr(y + 2);

        pThis += 2;
        pUp1 += 2;
        pUp2 += 2;
        pDown1 += 2;
        pDown2 += 2;
        for (int x = 2; x < mask.cols - 2; x++) {
            uchar v = *pThis;
            if (v == 0) {
                bool allAbove = *(pUp2 - 2) && *(pUp2 - 1) && *(pUp2) && *(pUp2 + 1) && *(pUp2 + 2);
                bool allLeft = *(pUp1 - 2) && *(pThis - 2) && *(pDown1 - 2);
                bool allBelow = *(pDown2 - 2) && *(pDown2 - 1) && *(pDown2) && *(pDown2 + 1) && *(pDown2 + 2);
                bool allRight = *(pUp1 + 2) && *(pThis + 2) && *(pDown1 + 2);
                bool surroundings = allAbove && allLeft && allBelow && allRight;
                if (surroundings == true) {

                    *(pUp1 - 1) = 255;
                    *(pUp1 + 0) = 255;
                    *(pUp1 + 1) = 255;
                    *(pThis - 1) = 255;
                    *(pThis + 0) = 255;
                    *(pThis + 1) = 255;
                    *(pDown1 - 1) = 255;
                    *(pDown1 + 0) = 255;
                    *(pDown1 + 1) = 255;
                }

                pThis += 2;
                pUp1 += 2;
                pUp2 += 2;
                pDown1 += 2;
                pDown2 += 2;
            }
            // Move to the next pixel.
            pThis++;
            pUp1++;
            pUp2++;
            pDown1++;
            pDown2++;
        }
    }
}

void drawFaceStickFigure(cv::Mat dst)
{
    cv::Size size = dst.size();
    int sw = size.width;
    int sh = size.height;

    cv::Mat faceOutline = cv::Mat::zeros(size, CV_8UC3);
    cv::Scalar color = CV_RGB(255, 255, 0); // yellow
    int thickness = 4;
    int faceH = sh / 2 * 70 / 100;
    int faceW = faceH * 72 / 100;
    cv::ellipse(faceOutline, cv::Point(sw / 2, sh / 2), cv::Size(faceW, faceH), 0, 0, 360, color, thickness, cv::LINE_AA);
    int eyeW = faceW * 23 / 100;
    int eyeH = faceH * 11 / 100;
    int eyeX = faceW * 48 / 100;
    int eyeY = faceH * 13 / 100;
    int eyeA = 15; // angle in degrees.
    int eyeYshift = 11;
    // Draw the top of the right eye.
    cv::ellipse(faceOutline, cv::Point(sw / 2 - eyeX, sh / 2 - eyeY), cv::Size(eyeW, eyeH), 0, 180 + eyeA, 360 - eyeA, color, thickness, cv::LINE_AA);
    // Draw the bottom of the right eye.
    cv::ellipse(faceOutline, cv::Point(sw / 2 - eyeX, sh / 2 - eyeY - eyeYshift), cv::Size(eyeW, eyeH), 0, 0 + eyeA, 180 - eyeA, color, thickness, cv::LINE_AA);
    // Draw the top of the left eye.
    cv::ellipse(faceOutline, cv::Point(sw / 2 + eyeX, sh / 2 - eyeY), cv::Size(eyeW, eyeH), 0, 180 + eyeA, 360 - eyeA, color, thickness, cv::LINE_AA);
    // Draw the bottom of the left eye.
    cv::ellipse(faceOutline, cv::Point(sw / 2 + eyeX, sh / 2 - eyeY - eyeYshift), cv::Size(eyeW, eyeH), 0, 0 + eyeA, 180 - eyeA, color, thickness, cv::LINE_AA);

    // Draw the bottom lip of the mouth.
    int mouthY = faceH * 53 / 100;
    int mouthW = faceW * 45 / 100;
    int mouthH = faceH * 6 / 100;
    cv::ellipse(faceOutline, cv::Point(sw / 2, sh / 2 + mouthY), cv::Size(mouthW, mouthH), 0, 0, 180, color, thickness, cv::LINE_AA);

    // Draw anti-aliased text.
    int fontFace = cv::FONT_HERSHEY_COMPLEX;
    float fontScale = 1.0f;
    int fontThickness = 2;
    cv::putText(faceOutline, "Put your face here", cv::Point(sw * 23 / 100, sh * 10 / 100), fontFace, fontScale, color, fontThickness, cv::LINE_AA);
    // imshow("faceOutline", faceOutline);

    // Overlay the outline with alpha blending.
    cv::addWeighted(dst, 1.0, faceOutline, 0.7, 0, dst, CV_8UC3);
}
