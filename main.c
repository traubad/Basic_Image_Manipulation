#include "lib/qdbmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define temp_filename ".temp.bmp"

//Get length of string, from Stack Overflow
int strlength(const char* string){ //https://stackoverflow.com/questions/25578886/
    int i;
    for(i=0;string[i]!='\0';i++);
    return i;
}

//creates a comma separated string of ints from an array (taken from Stack Overflow)
size_t join_integers(UINT *num, size_t num_len, char *buf, size_t buf_len) {
    size_t i;
    UINT written = strlength(buf);

    for(i = 0; i < num_len; i++) {
        written += snprintf(buf + written, buf_len - written, (i != 0 ? ",%lu" : "%lu"),
            *(num + i));
        if(written == buf_len)
            break;
    }

    return written;
}

void create_files(BMP** in, BMP** out, const char* filename){
    *in = BMP_ReadFile( filename );
    *out = BMP_ReadFile( filename );
}

void clear_and_delete(BMP** in, BMP** out){
    if(*in != *out){ //avoids a seg fault
      BMP_Free( *out );
    }
    BMP_Free( *in );
    remove(temp_filename);
}

void apply_left_rotation(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y, UINT height){
    UCHAR val;
    BMP_GetPixelIndex(*bmp_in, height-y-1, x, &val);
    BMP_SetPixelIndex(*bmp_out, x, y, val);
}

void apply_right_rotation(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y, UINT height){
    UCHAR val;
    BMP_GetPixelIndex(*bmp_in, y, height-x-1, &val);
    BMP_SetPixelIndex(*bmp_out, x, y, val);
}

void apply_mirror_vertical(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y, UINT height){
    UCHAR val;
    BMP_GetPixelIndex(*bmp_in, height-x-1, y, &val);
    BMP_SetPixelIndex(*bmp_out, x, y, val);
}

void apply_mirror_horizontal(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y, UINT height){
    UCHAR val;
    BMP_GetPixelIndex(*bmp_in, x, height-y-1, &val);
    BMP_SetPixelIndex(*bmp_out, x, y, val);
}

void apply_negative(BMP** bmp_in, BMP**  bmp_out, UINT x, UINT y){
    UCHAR val;
    BMP_GetPixelIndex(*bmp_in, x, y, &val);
    BMP_SetPixelIndex(*bmp_out, x, y, 255 - val);//Invert color value
}

void adjust_contrast(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y, float contrast){
    UCHAR val;

    float factor = (259 * (contrast + 255)) / (255 * (259 - contrast));
    BMP_GetPixelIndex(*bmp_in, x, y, &val);

    val = factor * (val - 128) + 128;
    if (val > 255){
      val = 255;
    } else if (val < 0){
      val = 0;
    }

    BMP_SetPixelIndex(*bmp_out, x, y, val);
}

void black_and_white(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y, unsigned short threshold){
    UCHAR val;

    BMP_GetPixelIndex(*bmp_in, x, y, &val);

    if (val > threshold){
      val = 255;
    } else{
      val = 0;
    }
    BMP_SetPixelIndex(*bmp_out, x, y, val);
}

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

//Based on this paper: https://www.ijsr.net/archive/v6i3/25031706.pdf
void apply_median_filter(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y){
  UCHAR val;
  UCHAR vals[9] = {0};
  short i = 0;

  for (short z=-1; z<=1; z++){
    for (short w=-1; w<=1; w++){
      BMP_GetPixelIndex(*bmp_in, x+w, y+z, &val);
      vals[i++] = val;
    }
  }
  qsort(vals, 9, sizeof(UCHAR), cmpfunc);
  BMP_SetPixelIndex(*bmp_out, x, y, vals[5]);
}

//Based on this paper: https://www.ijsr.net/archive/v6i3/25031706.pdf
void apply_smoothing_filter(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y){
  UCHAR val;
  unsigned short sum = 0;
  short i = 0;

  for (short z=-1; z<=1; z++){
    for (short w=-1; w<=1; w++){
      BMP_GetPixelIndex(*bmp_in, x+w, y+z, &val);
      sum += val;
    }
  }
  BMP_SetPixelIndex(*bmp_out, x, y, sum/9);
}

//Based on https://towardsdatascience.com/edge-detection-in-python-a3c263a13e03
void apply_edge_detection(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y){
  UCHAR val, final_score;

  UCHAR adjustment = 1;//my tinkering w/ the algorithm (set to 1 for normal behavior)

  int vertical_filter[3][3] = {
    {-1,-2,-1},
    { 0, 0, 0},
    { 1, 2, 1}
  };

  int horizontal_filter[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
  };

  double vertical_score = 0;
  double horizontal_score = 0;

  for (short z=-1; z<=1; z++){
    for (short w=-1; w<=1; w++){
      BMP_GetPixelIndex(*bmp_in, x+w, y+z, &val);
      vertical_score += vertical_filter[w+1][z+1]*adjustment * val;
      horizontal_score += horizontal_filter[w+1][z+1]*adjustment * val;
    }
  }

  /** (taken from artical)
   * Since we are doing detection on both horizontal and vertical edges, we just
   * divide the raw scores by 4 (rather than adding 4 and then dividing by 8).
   * It is not a major change but one which will better highlight the edges
   * of our image.
   **/

  vertical_score /= 4;
  horizontal_score /= 4;
  //re-normalize and sum scores
  final_score = sqrt((vertical_score*vertical_score) + (horizontal_score*horizontal_score));
  BMP_SetPixelIndex(*bmp_out, x, y, final_score);
}

int main( int argc, char* argv[] ){
    BMP*    bmp_in;
    BMP*    bmp_out;
    UINT    width, height;
    UINT    x, y;
    UCHAR   val;

    UINT histogram[256] = {0};

    if ( argc < 3 || strcmp(argv [ 1 ], "--help") == 0 ){
        printf("\n\nFormat: inFilename, outfilename, commands\n\n");
        printf("Commands include:\n");
        printf("\tl: rotate left\n");
        printf("\tr: rotate right\n");
        printf("\th: mirror horizontal\n");
        printf("\tv: mirror vertical\n");
        printf("\tb: black and white\n");
        printf("\tn: negative\n");
        printf("\tc: increase contrast by 10\n");
        printf("\te: Apply edge detection\n");
        printf("\tm: Apply Median Filter\n");
        printf("\ts: Apply Smoothing Filter\n");
        printf("\tg: Create a histogram\n\n");
        return 0;
    }

    /* Read an image file */
    create_files( &bmp_in, &bmp_out, argv[ 1 ] );
    BMP_CHECK_ERROR( stderr, -1 ); /* If an error has occurred, notify and exit */

    /* Get image's dimensions */
    width = BMP_GetWidth( bmp_in );
    height = BMP_GetHeight( bmp_in );

    BMP_GetError();
    int command_length = strlength(argv[ 3 ]);

    for (int i = 0; i < command_length; i++){
      /* Iterate through all the image's pixels */
      for ( x = 0 ; x < width ; ++x ){
        for ( y = 0 ; y < height ; ++y ){
          switch( argv[ 3 ][i] ){
            case 'b':
              black_and_white(&bmp_in, &bmp_out, x, y, 100);
              break;

            case 'n':
              apply_negative(&bmp_in, &bmp_out, x, y);
              break;

            case 'l':
              apply_left_rotation(&bmp_in, &bmp_out, x, y, height);
              break;

            case 'r':
              apply_right_rotation(&bmp_in, &bmp_out, x, y, height);
              break;

            case 'h':
              apply_mirror_horizontal(&bmp_in, &bmp_out, x, y, height);
              break;

            case 'v':
              apply_mirror_vertical(&bmp_in, &bmp_out, x, y, height);
              break;

            case 'c':
              adjust_contrast(&bmp_in, &bmp_out, x, y, 10);
              break;

            case 'g':
              BMP_GetPixelIndex(bmp_in, x, height-y-1, &val);
              histogram[val] += 1;
              break;

            case 'm':
            case 's':
            case 'e':
              if(x > 1 && x+1 < width && y > 0 && y+1 < height){
                if(argv[ 3 ][i] == 'm'){
                  apply_median_filter(&bmp_in, &bmp_out, x, y);
                } else if(argv[ 3 ][i] == 's'){
                  apply_smoothing_filter(&bmp_in, &bmp_out, x, y);
                } else if(argv[ 3 ][i] =='e'){
                  apply_edge_detection(&bmp_in, &bmp_out, x, y);
                }
              }
              break;

            default: //TODO this could be better
              fprintf(stderr, "\nError: '%c' is a bad command\n\n", argv[ 3 ][ i ]);
              clear_and_delete(&bmp_in, &bmp_out);
              return 1;
          }
        }
      }

      //.temp.bmp is an annoying workaround because of the difficulty in copying structs
      if(i+1 < command_length){
          BMP_WriteFile( bmp_out, temp_filename );
          bmp_in = bmp_out;
          bmp_out = BMP_ReadFile( temp_filename );
      }
      if(argv[ 3 ][i] == 'g'){
        size_t size;
        char buf[1100] = "python3 histogram.py ";
        for(int j=0; j< i; j++)
          buf[strlength(buf)] = argv[3][j];
        strcat(buf, " ");

        size = join_integers(histogram, 256, buf, 1100);
        strcat(buf," &");
        system(buf);
        memset(histogram, 0, 256 * sizeof(histogram[0]));
      }
    }
    bmp_in = bmp_out;
    /* Save result */
    BMP_WriteFile( bmp_in, argv[ 2 ] );
    BMP_CHECK_ERROR( stderr, -2 );

    /* Free all memory allocated for the image */
    clear_and_delete(&bmp_in, &bmp_out);

    return 0;
}
