//#include <windows.h>
#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <serialport.h>

#include <sstream>

using namespace cv;
using namespace std;


string IntToString (int a)
{
    ostringstream temp;
    temp<<a;
    return temp.str();
}

/*Portname must contain these backslashes, and remember to
replace the following com port*/
char *port_name = "\\\\.\\COM8";

int main( int argc, char** argv )
{
    SerialPort arduino(port_name);
    if (arduino.isConnected()) cout << "Connection Established" << endl;
    else cout << "ERROR, check port name";

    VideoCapture cap(0); //capture the video from webcam

    if ( !cap.isOpened() )  // if not success, exit program
    {
        cout << "Cannot open the web cam" << endl;
        return -1;
    }

    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

    int iLowH = 10;
    int iHighH = 29;

    int iLowS = 129;
    int iHighS = 255;

    int iLowV = 180;
    int iHighV = 255;

    int sizeEroDil = 9;

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
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    RNG rng(12345);
    int posXOld;
    string input_string;

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
        Canny( imgThresholded, canny_output, 100, 100*2, 3 );
        findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
        drawContours( imgOriginal, contours, 0, Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) ), 2, 8, hierarchy, 0, Point() );

        //Calculate the moments of the thresholded image
        Moments oMoments = moments(imgThresholded);

        double dM01 = oMoments.m01;
        double dM10 = oMoments.m10;
        double dArea = oMoments.m00;

        // if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
        if (dArea > 50000)
        {
            //calculate the position of the ball
            int posX = dM10 / dArea;
            int posY = dM01 / dArea;

            if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
            {
                //Draw a red line from the previous point to the current point
                line(imgOriginal,Point(posX, posY),Point(posX, posY), Scalar(0,0,255), 3);
                //circle(imgOriginal,Point(posX, posY), dArea/100000, (0,255,0), -1);
                //putText(imgOriginal,lpBuffer ,Point(15,15), FONT_HERSHEY_SIMPLEX, 0.5,(0,255,255),1,false);
                putText(imgOriginal,format("(%d,%d)", posX,posY),Point(10, 10), FONT_HERSHEY_SIMPLEX, 0.5,(255,255,255),1,false);

                if (input_string != IntToString(posX/3.55)+"\n")
                {
                    input_string = IntToString(posX/3.55)+"\n";
                    cout<<input_string<<endl;
                    //Creating a c string
                    char *c_string = new char[input_string.size()];
                    //copying the std::string to c string
                    //copy(input_string.begin(), input_string.end(), c_string);

                    for (int i=0; i<=input_string.size(); i++) c_string[i]=input_string[i];
                    //Writing string to arduino
                     for (int i=0; i<=input_string.size(); i++)cout<<c_string[i];
                    arduino.writeSerialPort(c_string, MAX_DATA_LENGTH);
                }




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
