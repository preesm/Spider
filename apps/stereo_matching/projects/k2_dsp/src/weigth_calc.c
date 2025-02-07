#include <string.h>
#include <math.h>

#include "stereomatch.h"

#define LIN_METH
#define THR_WEIGTH 63
#define CST_CD 4

/*
 * \brief compute vertical weight for a specific delta.
 * \param height : height of image in pixels.
 * \param width : width of image in pixels.
 * \param delta : aggregation distance in pixels.
 * \param imr : red values of input image. Size must be (height*width).
 * \param img : green values of input image. Size must be (height*width).
 * \param imb : blues values of input image. Size must be (height*width).
 * \param out : Ouput weight table. Size must be (height*width).
 */
void compute_vweigth(
	int heigth, int width,
	int delta,
	/*input buffer*/
	const uint8_t* restrict img,
	/*output buffer*/
	uint8_t* restrict out)
{

	/*index variables*/
	unsigned int x, y;
	unsigned int idxp = 0, idx = 0;

	/*compute variables*/
#ifdef LIN_METH
	uint16_t acc;
	uint8_t cr, cg, cb, d1, temp;

	/*constants*/
	uint8_t constdistance = 255-(delta)*CST_CD;
#else

	float acc, temp;

	float constdistance = -delta/36.;
	float cr, cg, cb, d1;
#endif


	idx = 0;
	idxp = 3*delta*width;

	/*loop around image*/
	/* we takes a pixel at y and y + delta, stops at height-delta */
	for(y=0; y<(heigth-delta); y++)
	{
		for(x=0; x<width; x++)
		{
			/*get cur rgb*/
			cr = img[idx++];
			cg = img[idx++];
			cb = img[idx++];

			/*compute formula*/

#ifdef LIN_METH
			/*pixel similarity (sum of absolute difference of RGB values)*/
			d1 = img[idxp++];
			acc =  (d1>cr)?(d1-cr):(cr-d1);
			d1 = img[idxp++];
			acc += (d1>cg)?(d1-cg):(cg-d1);
			d1 = img[idxp++];
			acc += (d1>cb)?(d1-cb):(cb-d1);

			/*saturation*/
			temp = (acc > THR_WEIGTH)?(THR_WEIGTH):acc;
			/*reverse value and enlarge dynamic*/
			temp = (THR_WEIGTH - temp)*(255/THR_WEIGTH);
			/*Adjusts value with a distance coefficient (fixed multiplication)*/
			temp = ((uint16_t)(temp*constdistance+0x80))>>8;
#else
			d1 = (float)imr[idxp] - cr;
			acc =  d1*d1;
			d1 = (float)img[idxp] - cg;
			acc +=  d1*d1;
			d1 = (float)imb[idxp] - cb;
			acc +=  d1*d1;

			acc = sqrtf(acc)/16.;
			temp = exp(constdistance - acc)*255.;
#endif


			/*store results*/
			out[idx] = temp>>2;

//			idx++;
//			idxp++;
		}
	}
}

/*
 * \brief compute horizontal weight for a specific delta.
 * \param height : height of image in pixels.
 * \param width : width of image in pixels.
 * \param delta : aggregation distance in pixels.
 * \param imr : red values of input image. Size must be (height*width).
 * \param img : green values of input image. Size must be (height*width).
 * \param imb : blues values of input image. Size must be (height*width).
 * \param out : Ouput weight table. Size must be (height*width).
 */
void compute_hweigth(
	int heigth, int width,
	int delta,
	/*input buffer*/
	const uint8_t* restrict img,
	/*output buffer*/
	uint8_t* restrict out)
{

	/*index variables*/
	unsigned int x, y;
	unsigned int idxp = 0, idx = 0;

#ifdef LIN_METH
	/*compute variables*/
	uint16_t acc;
	uint8_t cr, cg, cb, d1, temp;

	/*constants*/
	uint8_t constdistance = 255-(delta)*CST_CD;
#else

	float acc, temp;

	float constdistance = -delta/36.;
	float cr, cg, cb, d1;
#endif

	/*loop around image*/
	for(y=0; y<heigth; y++)
	{
		idx = 3*(width*y);
		idxp = 3*(idx + delta);
		/* we takes a pixel at x and x + delta, stops at width-delta */
		for(x=0; x<(width-delta); x++)
		{
			/*get cur rgb*/
			cr = img[idx++];
			cg = img[idx++];
			cb = img[idx++];

			/*compute formula*/

#ifdef LIN_METH
			/*pixel similarity (sum of absolute difference of RGB values)*/
			d1 = img[idxp++];
			acc =  (d1>cr)?(d1-cr):(cr-d1);
			d1 = img[idxp++];
			acc += (d1>cg)?(d1-cg):(cg-d1);
			d1 = img[idxp++];
			acc += (d1>cb)?(d1-cb):(cb-d1);

			/*saturation*/
			temp = (acc > THR_WEIGTH)?(THR_WEIGTH):acc;
			/*reverse value and enlarge dynamic*/
			temp = (THR_WEIGTH - temp)*(255/THR_WEIGTH);
			/*Adjusts value with a distance coefficient (fixed point  multiplication)*/
			temp = ((uint16_t)(temp*constdistance))>>8;
#else

			d1 = (float)imr[idxp] - cr;
			acc =  d1*d1;
			d1 = (float)img[idxp] - cg;
			acc +=  d1*d1;
			d1 = (float)imb[idxp] - cb;
			acc +=  d1*d1;

			acc = sqrtf(acc)/16.;
			temp = exp(constdistance - acc)*255.;
#endif

			out[idx] = temp >> 2;

//			idx++;
//			idxp++;
		}
	}
}
