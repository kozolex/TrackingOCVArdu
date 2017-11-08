#include <windows.h>
#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <sstream>

using namespace cv;
using namespace std;


string IntToString (int a)
{
    ostringstream temp;
    temp<<a;
    return temp.str();
}

int main( int argc, char** argv )
{
    //SERIAL PORT
    HANDLE hComm;
    bool Status;

    hComm = CreateFile("COM8",                //port name
                       GENERIC_READ | GENERIC_WRITE, //Read/Write
                       0,                            // No Sharing
                       NULL,                         // No Security
                       OPEN_EXISTING,// Open existing port only
                       0,            // Non Overlapped I/O
                       NULL);        // Null for Comm Devices

    if (hComm == INVALID_HANDLE_VALUE)
        cout<<"Error in opening serial port"<<endl;
    else
        cout<<"opening serial port successful"<<endl;


    DCB dcbSerialParams = { 0 }; // Initializing DCB structure
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    Status = GetCommState(hComm, &dcbSerialParams);

    dcbSerialParams.BaudRate = CBR_9600;  // Setting BaudRate = 9600
    dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
    dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
    dcbSerialParams.Parity   = NOPARITY;  // Setting Parity = None

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout         = 500; // in milliseconds
    timeouts.ReadTotalTimeoutConstant    = 500; // in milliseconds
    timeouts.ReadTotalTimeoutMultiplier  = 100; // in milliseconds
    timeouts.WriteTotalTimeoutConstant   = 50; // in milliseconds
    timeouts.WriteTotalTimeoutMultiplier = 30; // in milliseconds


    string lpBuffer;

    DWORD dNoOFBytestoWrite;         // No of bytes to write into the port
    DWORD dNoOfBytesWritten = 0;     // No of bytes written to the port
    dNoOFBytestoWrite = sizeof(lpBuffer);
    /*
        Status = WriteFile(hComm,        // Handle to the Serial port
                           lpBuffer.c_str(),     // Data to be written to the port
                           dNoOFBytestoWrite,  //No of bytes to write
                           &dNoOfBytesWritten, //Bytes written
                           NULL);
    */
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
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    RNG rng(12345);
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

                lpBuffer = IntToString(posX/3.6)+"\n";
                //cout<<lpBuffer;
                cout<<lpBuffer.c_str();

                Status = WriteFile(hComm,        // Handle to the Serial port
                                   lpBuffer.c_str(),     // Data to be written to the port
                                   dNoOFBytestoWrite,  //No of bytes to write
                                   &dNoOfBytesWritten, //Bytes written
                                   NULL);
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
    CloseHandle(hComm);//Closing the Serial Port
    //END SERIAL PORT
}
