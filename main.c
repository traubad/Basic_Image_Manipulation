/* Creates a negative image of the input bitmap file */
#include "lib/qdbmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define temp_filename ".temp.bmp"

int strlength(const char* string) //https://stackoverflow.com/questions/25578886/
{
    int i;
    for(i=0;string[i]!='\0';i++);
    return i;
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

void apply_negative(BMP** bmp_in, BMP**  bmp_out, UINT x, UINT y){
    UCHAR val;
    /* Get pixel's RGB values */
    BMP_GetPixelIndex(*bmp_in, x, y, &val);
    /* Invert RGB values */
    BMP_SetPixelIndex(*bmp_out, x, y, 255 - val);
}

void adjust_contrast(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y, float contrast){
    UCHAR val;
    float factor = (259 * (contrast + 255)) / (255 * (259 - contrast));
    /* Get pixel's RGB values */
    BMP_GetPixelIndex(*bmp_in, x, y, &val);

    val = factor * (val - 128) + 128;
    if (val > 255){
      val = 255;
    } else if (val < 0){
      val = 0;
    }

    /* Invert RGB values */
    BMP_SetPixelIndex(*bmp_out, x, y, val);
}

void black_and_white(BMP** bmp_in, BMP** bmp_out, UINT x, UINT y){
    UCHAR val;
    /* Get pixel's RGB values */
    BMP_GetPixelIndex(*bmp_in, x, y, &val);

    if (val > 255/2){
      val = 255;
    } else{
      val = 0;
    }

    /* Invert RGB values */
    BMP_SetPixelIndex(*bmp_out, x, y, val);
}

int main( int argc, char* argv[] )
{
    BMP*    bmp_in;
    BMP*    bmp_out;
    UINT    width, height;
    UINT    x, y;

    if ( strcmp(argv [ 1 ], "--help") == 0 )
    {
        printf("\n\nFormat: inFilename, outfilename, commands\n");
        printf("Commands include:\n");
        printf("\tl: rotate left\n");
        printf("\tr: rotate right\n");
        printf("\tb: black and white\n");
        printf("\tn: negative\n");
        printf("\tc: increase contrast by 10\n\n");
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
              black_and_white(&bmp_in, &bmp_out, x, y);
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

            case 'c':
              adjust_contrast(&bmp_in, &bmp_out, x, y, 10);
              break;

            default:
              fprintf(stderr, "\nError: '%c' is a bad command\n\n", argv[ 3 ][ i ]);
              clear_and_delete(&bmp_in, &bmp_out);
              return 1;
          }
        }
      }

      //.temp.bmp is an annoying workaround because of the difficulty in copying structs
      if(i+1 < command_length){
          BMP_WriteFile( bmp_out, temp_filename );
          BMP_GetError();
          bmp_in = bmp_out;
          bmp_out = BMP_ReadFile( temp_filename );
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
