#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>
#include <netdb.h>
#include <pthread.h>
#include "video.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
/*
* Handles video streaming data
*/
void handleVideo(const unsigned char stream[])
{
  CvCapture *capture = cvCaptureFromCAM(CV_CAP_ANY);
  if(!capture)
  {
    printf("ERROR: Capture is NULL\n");
    exit(-1);
  }
  cvNamedWindow("iRis", CV_WINDOW_AUTOSIZE);
  while(1)
  {
    IplImage *frame = cvQueryFrame(capture);
    if(!frame)
    {
      printf("ERROR: Unable to capture frame\n");
      break;
    }
    cvShowImage("iRis", frame);
  }
  cvDestroyWindow("iRis");
}

inline void showFrame(IplImage *frame[])
{
  int i;
  for(i = 0; i < FRAMEMAX; ++i) cvShowImage("iRis", frame[i]);
}

inline void grabFrames(IplImage *frame[], CvCapture *capture)
{
  int i;
  for(i = 0; i < FRAMEMAX; ++i) frame[i] = cvQueryFrame(capture);
}

void sendFrames(const int *cfd, IplImage *frame[])
{
  int i;
  IplImage f[FRAMEMAX];
  for(i = 0; i < FRAMEMAX; ++i)
  {
    f[i] = *frame[i];
    free(frame[i]);
  }
  send(cfd, f, sizeof(IplImage)*FRAMEMAX); 
}

inline void recvFrames(const int *cfd, IplImage frame[])
{
  recv(cfd, frame, sizeof(IplImage)*FRAMEMAX);
}

int main(int argc, char *argv[])
{
  return 0;
}
