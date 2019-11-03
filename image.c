// Kevin Stock

// This file includes functions for manipulating ppm images,
// namely reading, writing, and a comparison for checking fitness.

#include "wiproj.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void rgb2hsv(float r, float g, float b, float *h, float *s, float *v) {
  float minVal = MIN(r, MIN(g,b));
  float maxVal = MAX(r, MAX(g,b));
  float delta = maxVal - minVal;
  *v = maxVal;
  if (delta == 0.0f) {
    *h = 0.0f;
    *s = 0.0f;
  } else {
    *s = delta / maxVal;
    float del_R = (((maxVal - r) / 6.0f) + (delta / 2.0f)) / delta;
    float del_G = (((maxVal - g) / 6.0f) + (delta / 2.0f)) / delta;
    float del_B = (((maxVal - b) / 6.0f) + (delta / 2.0f)) / delta;
    if (r == maxVal) {
      *h = del_B - del_G;
    } else if (g == maxVal) {
      *h = (1.0f / 3.0f) + del_R - del_B;
    } else { // b == maxVal
      *h = (2.0f / 3.0f) + del_G - del_R;
    }
    if (*h < 0.0f)
      *h += 1.0f;
    if (*h > 1.0f)
      *h -= 1.0f;
  }
}

long image_diff(image *a, image *b) {
  long ret = 0;
  int i = 0;

  if (a->width != b->width || a->height != b->height) {
    printf("Image diff of different sized images attempted.\n");
    exit(0);
  }

  #pragma omp parallel for reduction(+:ret)
  for (i=0; i < a->width * a->height; i++) {
    int ar, ag, ab, br, bg, bb;
    ar = (a->values)[3*i];
    ag = (a->values)[3*i+1];
    ab = (a->values)[3*i+2];
    br = (b->values)[3*i];
    bg = (b->values)[3*i+1];
    bb = (b->values)[3*i+2];
    // Sum of squares of difference in r g b channels
#if RGB_COMP
    int val;
    val = ar - br;
    ret += val * val;
    val = ag - bg;
    ret += val * val;
    val = ab - bb;
    ret += val * val;
#else
    // Sum of squares of differences in h s v channels
    float ah, as, av, bh, bs, bv;
    float hw = 1000, sw = 1000, vw = 1000;
    rgb2hsv(ar/255.0f, ag/255.0f, ab/255.0f, &ah, &as, &av);
    rgb2hsv(br/255.0f, bg/255.0f, bb/255.0f, &bh, &bs, &bv);
    ret += hw*(ah-bh)*(ah-bh);
    ret += sw*(as-bs)*(as-bs);
    ret += vw*(av-bv)*(av-bv);
#endif
  }

  return ret;
}

void flip_image(image* img) {
  int i, j;
  GLubyte t;
  for (i = 0; i < img->height / 2; i++) {
    int first = i * img->width * 3;
    int second = (img->height - i - 1) * img->width * 3;
    for (j = 0; j < img->width * 3; j++) {
      t = (img->values)[first+j];
      (img->values)[first+j] = (img->values)[second+j];
      (img->values)[second+j] = t;
    }
  }
}

int get_int(FILE *file) {
  int x = 0;
  int val;

  while (!isdigit(val = getc(file))) {
    if (val == '#') {
      while ((val = getc(file)) != '\n') {
        if (val == EOF) return 0;
      }
    }
    if (val == EOF) return 0;
  }

  do {
    x = x*10 + (val - '0');
  } while (isdigit(val = getc(file))); 

  return x;
}

image* load_ppm(FILE *file) {
  image *bad = malloc(sizeof(image));
  image *ret = malloc(sizeof(image));
  bad->width = 0;
  bad->height = 0;
  bad->values = NULL;

  if (file==NULL) return bad;
  if (getc(file) != 'P') return bad;
  if (getc(file) != '6') return bad;

  ret->width = get_int(file);
  ret->height = get_int(file);
  if (get_int(file) != 255) return bad;

  ret->values = malloc(ret->width*ret->height*3*sizeof(GLubyte));
  if (ret->values == NULL) {
    printf("Failed to allocate memory.\n");
    exit(0);
  }
  if (fread(ret->values,1, ret->width*ret->height*3, file) < ret->width*ret->height*3) {
    printf("Corrupt ppm file.\n");
    exit(0);
  }

  flip_image(ret);

  free(bad);
  return ret;
}

void write_ppm(FILE *file, image *img) {
  flip_image(img);
  if (file == NULL) return;
  fprintf(file, "P6 %d %d 255\n",img->width, img->height);
  int i;
  for (i=0;i < img->width*img->height*3; i++) {
    fprintf(file, "%c",(img->values)[i]);
  }
  flip_image(img);
}

/*
int main(int argc, char** argv) {
  if (argc < 2) return 1;
  FILE* fin = fopen(argv[1],"r");
  FILE* fout = fopen("out.ppm","w");

  image* ia = load_ppm(fin);

  if (ia->width == 0 || ia->height == 0) {
    printf("Error 1\n");
    return 0;
  } 

  (ia->values)[0] = (GLubyte) 255;

  write_ppm(fout,ia);
  fclose(fout);

  (ia->values)[0] = (GLubyte) 0;

  FILE* fin2 = fopen("out.ppm","r");
  image* ib = load_ppm(fin2);

  printf("%ld\n",image_diff(ia,ib));
  return 0;
}
*/
