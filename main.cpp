#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
    VideoCapture cap(0); //capture the video from webcam

    if ( !cap.isOpened() )  // if not success, exit program
    {
        cout << "Cannot open the web cam" << endl;
        return -1;
    }

    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

    int iLowH = 10;
    int iHighH = 39;

    int iLowS = 25;
    int iHighS = 255;

    int iLowV = 191;
    int iHighV = 255;

    int sizeEroDil = 10;

    //Create trackbars in "Control" window
    createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    createTrackbar("HighH", "Control", &iHighH, 179);

    createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    createTrackbar("HighS", "Control", &iHighS, 255);

    createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
    createTrackbar("HighV", "Control", &iHighV, 255);

    createTrackbar("EroDill", "Control", &sizeEroDil, 20);

    int iLastX = -1;
    int iLastY = -1;

    //Capture a temporary image from the camera
    Mat imgTmp;
    cap.read(imgTmp);

    //Create a black image with the size as the camera output
    //Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );;

    Mat imgHSV;
    Mat imgThresholded;
    while (true)
    {
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // read a new frame from video

        if (!bSuccess) //if not success, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
         cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

        if (sizeEroDil>0)
        {
            //morphological opening (removes small objects from the foreground)
            erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(sizeEroDil, sizeEroDil)) );
            dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(sizeEroDil, sizeEroDil)) );

            //morphological closing (removes small holes from the foreground)
            dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(sizeEroDil, sizeEroDil)) );
            erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(sizeEroDil, sizeEroDil)) );
        }

        //Calculate the moments of the thresholded image
        Moments oMoments = moments(imgThresholded);

        double dM01 = oMoments.m01;
        double dM10 = oMoments.m10;
        double dArea = oMoments.m00;

        // if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
        if (dArea > 10000)
        {
            //calculate the position of the ball
            int posX = dM10 / dArea;
            int posY = dM01 / dArea;

            if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
            {
                //Draw a red line from the previous point to the current point
                line(imgOriginal,Point(posX, posY),Point(posX, posY) , Scalar(0,0,255), 2);
                circle(imgOriginal,Point(posX, posY), dArea, (0,0,255), -1);
                putText(imgOriginal,format("(%d,%d)", posX,posY) ,Point(posX, posY), FONT_HERSHEY_SCRIPT_SIMPLEX, 0.5,(255,255,255),1,false);
            }

            iLastX = posX;
            iLastY = posY;
        }

        imshow("Thresholded Image", imgThresholded); //show the thresholded image

        imshow("Original", imgOriginal); //show the original image

        if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            break;
        }
    }

    return 0;
}
