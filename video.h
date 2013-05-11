#ifndef __VIDEO__H
#define __VIDEO__H
#define FRAMEMAX 150
#include <opencv/cv.h>
#include <opencv/highgui.h>
void handleVideo(unsigned char []);
inline void showFrame(IplImage *[]);
inline void grabFrames(IplImage *[], CvCapture *);
void sendFrames(const int *, IplImage *[]);
inline void recvFrames(const int *, IplImage []);
#endif
