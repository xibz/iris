#ifndef __VIDEO__H
#define __VIDEO__H
#define FRAMEMAX 150;
void handleVideo(const unsigned char []);
inline void showFrame(IplImage *);
inline void grabFrames(IplImage *frame[], CvCapture *capture)
void sendFrames(const int *, IplImage *[]);
inline void recvFrames(const int *, IplImage []);
#endif
