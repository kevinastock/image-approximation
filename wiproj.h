// Kevin Stock

#ifndef WIPROJ_H
#define WIPROJ_H

#include <GL/gl.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

#define WIPROJ_VERSION "0.1"

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)<(b)?(b):(a))

// Compare with rgb or hsv?
#define RGB_COMP 1

typedef struct _image {
  int width, height;
  GLubyte * values;
} image;

typedef struct _triangle {
  /* Location, bounded to [0,1] */
  GLfloat x1, x2, x3;
  GLfloat y1, y2, y3;

  /* Color, RGBA */
  GLfloat r, g, b, a;
} triangle;

typedef struct _tri_image {
  long error;
  /* State Values:
   * 0 - Incomplete: img->values = null, error = 0 // Made by mh
   * 1 - Partial: img complete, error = 0          // Done by renderer
   * 2 - Complete                                  // Set by mh in process
   */
  int state;
  int generation; 
  int size; // number of triangles
  triangle * triangles;
  image * img;
} tri_image;

/* renderer.c */
extern void start (
    int argc, char** argv,
    tri_image*  (*next)(void), 
    tri_image*  (*best)(void), 
    void        (*process)(tri_image*));

/* image.c */
extern long image_diff (
    image * a,
    image * b);
extern image* load_ppm(FILE* file);
extern void write_ppm(FILE* file,image* img);



/* mh.c */
extern void mh_init(void);

extern tri_image* shc_next(void);
extern tri_image* shc_best(void);
extern void shc_process(tri_image* ti);
extern void shc_init(image* source, int size);

extern tri_image* ashc_next(void);
extern tri_image* ashc_best(void);
extern void ashc_process(tri_image* ti);
extern void ashc_init(image* source, int size);

extern tri_image* sa_next(void);
extern tri_image* sa_best(void);
extern void sa_process(tri_image* ti);
extern void sa_init(image* source, int size);

extern tri_image* acc_next(void);
extern tri_image* acc_best(void);
extern void acc_process(tri_image* ti);
extern void acc_init(image* source, int size);

extern tri_image* ga_next(void);
extern tri_image* ga_best(void);
extern void ga_process(tri_image* ti);
extern void ga_init(image* source, int tsize, int psize);

#endif
