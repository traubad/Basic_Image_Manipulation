/* Creates a negative image of the input bitmap file */
#include "lib/qdbmp.h"
#include <stdio.h>

void create_files(BMP** in, BMP** out, const char* filename){
  *in = BMP_ReadFile( filename );
  *out = BMP_ReadFile( filename );
}

void apply_left_rotation(BMP** bmp_in, BMP**  bmp_out, UINT x, UINT y){
    UCHAR val;
    /* Get pixel's RGB values */
    BMP_GetPixelIndex(*bmp_in, x, y, &val);
    /* Invert RGB values */
    BMP_SetPixelIndex(*bmp_out, y, x, val);
}

void apply_right_rotation(BMP** bmp_in, BMP**  bmp_out, UINT x, UINT y){
    UCHAR val;
    UINT height = BMP_GetHeight( *bmp_in );
    /* Get pixel's RGB values */
    BMP_GetPixelIndex(*bmp_in, x, y, &val);
    /* Invert RGB values */
    BMP_SetPixelIndex(*bmp_out, height - y, x, val);
}

void apply_negative(BMP** bmp_in, BMP**  bmp_out, UINT x, UINT y){
    UCHAR val;
    /* Get pixel's RGB values */
    BMP_GetPixelIndex(*bmp_in, x, y, &val);
    /* Invert RGB values */
    BMP_SetPixelIndex(*bmp_out, x, y, 255 - val);
}

int main( int argc, char* argv[] )
{
    BMP*    bmp_in;
    BMP*    bmp_out;
    //UCHAR   val;
    UINT    width, height, depth;
    UINT    x, y;

    if ( argc != 3 )
    {
        fprintf( stderr, "Usage: %s <input file> <output file>\n", argv[ 0 ] );
        return 0;
    }

    /* Read an image file */
    // bmp_in = BMP_ReadFile( argv[ 1 ] );
    // bmp_out = BMP_ReadFile( argv[ 1 ] );
    create_files( &bmp_in, &bmp_out, argv[ 1 ] );
    BMP_CHECK_ERROR( stderr, -1 ); /* If an error has occurred, notify and exit */

    /* Get image's dimensions */
    width = BMP_GetWidth( bmp_in );
    height = BMP_GetHeight( bmp_in );

    BMP_GetError();

    /* Iterate through all the image's pixels */
    for ( x = 0 ; x < width ; ++x )
    {
        for ( y = 0 ; y < height ; ++y )
        {
            apply_negative(&bmp_in, &bmp_in, x, y);
            apply_left_rotation(&bmp_in, &bmp_out, x, y);
        }
    }
    /* Save result */
    BMP_WriteFile( bmp_out, argv[ 2 ] );
    BMP_CHECK_ERROR( stderr, -2 );

    /* Free all memory allocated for the image */
    BMP_Free( bmp_in );
    BMP_Free( bmp_out );

    return 0;
}
