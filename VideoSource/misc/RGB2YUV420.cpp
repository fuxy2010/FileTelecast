#include "stdlib.h"
#include "RGB2YUV420.h"

static float RGBYUV02990[256], RGBYUV05870[256], RGBYUV01140[256];
static float RGBYUV01684[256], RGBYUV03316[256];
static float RGBYUV04187[256], RGBYUV00813[256];

void InitLookupTable( )
{
	int i;
	for (i = 0; i < 256; i++) RGBYUV02990[i] = (float)0.2990 * i;
	for (i = 0; i < 256; i++) RGBYUV05870[i] = (float)0.5870 * i;
	for (i = 0; i < 256; i++) RGBYUV01140[i] = (float)0.1140 * i;
	for (i = 0; i < 256; i++) RGBYUV01684[i] = (float)0.1684 * i;
	for (i = 0; i < 256; i++) RGBYUV03316[i] = (float)0.3316 * i;
	for (i = 0; i < 256; i++) RGBYUV04187[i] = (float)0.4187 * i;
	for (i = 0; i < 256; i++) RGBYUV00813[i] = (float)0.0813 * i;
}

int RGB2YUV420(int x_dim, int y_dim, void *bmp, void *y_out, void *u_out, void *v_out, int flip)
{
	long i, j,size;
	unsigned char *r, *g, *b;
	unsigned char *y, *u, *v;
	unsigned char *pu1, *pv1, *psu, *psv;
	unsigned char *y_buffer, *u_buffer, *v_buffer;
	unsigned char *sub_u_buf, *sub_v_buf;


	InitLookupTable();
	
	// allocate memory
	size=x_dim*y_dim;
	y_buffer = (unsigned char *)y_out;
	sub_u_buf = (unsigned char *)u_out;
	sub_v_buf = (unsigned char *)v_out;
	u_buffer = (unsigned char *)malloc(size * sizeof(unsigned char));
	v_buffer = (unsigned char *)malloc(size * sizeof(unsigned char));
   

	if (!(u_buffer && v_buffer))
	{
		if (u_buffer) free(u_buffer);
		if (v_buffer) free(v_buffer);
		return 2;
	}
	
	b = (unsigned char *)bmp;
	y = y_buffer;
	u = u_buffer;
	pu1 = u_buffer;
	v = v_buffer;
	pv1= v_buffer;
	psu = sub_u_buf;
	psv = sub_v_buf;
	
	if(!flip) 
	{
		for (j = 0; j < y_dim; )
		{

			y = y_buffer + (y_dim - j - 1) * x_dim;
			u = u_buffer + (y_dim - j - 1) * x_dim;
			v = v_buffer + (y_dim - j - 1) * x_dim;
			for (i = 0; i < x_dim; i ++) 
			{
				g = b + 1;
				r = b + 2;
				*y = (unsigned char)(  RGBYUV02990[*r] + RGBYUV05870[*g] + RGBYUV01140[*b]);
				*u = (unsigned char)(- RGBYUV01684[*r] - RGBYUV03316[*g] + (*b)/2          + 128);
				*v = (unsigned char)(  (*r)/2          - RGBYUV04187[*g] - RGBYUV00813[*b] + 128);
				b += 3;
				y ++;
				u ++;
				v ++;
			}

			j++;
		}
	}
	else
	{
		for (j = 0; j < y_dim; )
		{
			for (i = 0; i < x_dim; i ++) 
			{
					g = b + 1;
					r = b + 2;
					*y = (unsigned char)(  RGBYUV02990[*r] + RGBYUV05870[*g] + RGBYUV01140[*b]);
					*u = (unsigned char)(- RGBYUV01684[*r] - RGBYUV03316[*g] + (*b)/2          + 128);
					*v = (unsigned char)(  (*r)/2          - RGBYUV04187[*g] - RGBYUV00813[*b] + 128);
					b += 3;
					y ++;
					u ++;
					v ++;
			}
	
			j++;
		}
	}
	
	// subsample UV
	for (j=0;j<y_dim/2;j++)
	{
		for (i=0;i<x_dim/2;i++)
		{
			*psu=*pu1;
			*psv=*pv1;
			psu++;
			psv++;
			pv1+=2;
			pu1+=2;
		}
		pu1+=x_dim;
		pv1+=x_dim;
	}

	free(u_buffer);
	free(v_buffer);
	
	return 0;
}
