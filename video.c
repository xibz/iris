#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
void handleVideo(const unsigned char []);
/*
* Handles video streaming data
*/
void handleVideo(const unsigned char stream[])
{
  OggVorbis_File vf;
  #ifdef _WIN32
    _setmodel(_fileno(stdin), _O_BINARY);
    _setmodel(_fileno(stdout), _O_BINARY);
  #endif
  if(ov_open_callbacks((void *)stream, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
  {
    printf("Input does not appear to be an Ogg bitstream\n");
    exit(-1);
  }

}

int main(int argc, char *argv[])
{
  unsigned char vidStream[STREAMSIZE];
  handleVideo(vidStream);
  return 0;
}
