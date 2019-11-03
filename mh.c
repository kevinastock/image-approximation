// Kevin Stock

// This file contains the metaheuristics for the program. There is a section
// of general purpose functions, followed by sets of functions for 
// each metaheuristic (mh)
//
// Each mh is composed of four functions: and initialization,
// a next for returning the next tri_image to render,
// a best which returns the tri_image with the best fitness,
// and a process callback for processing a tri_image after it has been rendered

#include "wiproj.h"
#include "mt64.h"
#include <stdlib.h>

// General Purpose Functions
void mh_init() {
  //unsigned long long init[4] = {0x42345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL}, length=4;
  //init_by_array64(init, length);
  init_genrand64(time(NULL));
}

void free_tri_image(tri_image* ti) {
  if (ti) {
    if (ti->img) {
      if (ti->img->values) {
        free(ti->img->values);
      }
      free(ti->img);
    }
    if (ti->triangles) {
      free(ti->triangles);
    }
    free(ti);
  }
}

tri_image* new_tri_image(int size, int gen, int w, int h) {
  tri_image* ret = malloc(sizeof(tri_image));
  ret->size = size;
  ret->state = 0;
  ret->generation = gen;
  ret->triangles = malloc(size * sizeof(triangle));
  ret->img = malloc(sizeof(image));
  ret->img->width = w;
  ret->img->height = h;
  ret->img->values = malloc(w*h*3*sizeof(GLubyte));
  return ret;
}

void new_triangle(triangle* t) {
  t->x1 = genrand64_real2();
  t->y1 = genrand64_real2();
  t->x2 = genrand64_real2();
  t->y2 = genrand64_real2();
  t->x3 = genrand64_real2();
  t->y3 = genrand64_real2();
  t->r = genrand64_real2();
  t->g = genrand64_real2();
  t->b = genrand64_real2();
  t->a = genrand64_real2();
}

tri_image* random_tri_image(int size, int gen, int w, int h) {
  tri_image* ret = new_tri_image(size,gen,w,h);
  int i;
  for (i=0;i<size;i++) {
    new_triangle(&(ret->triangles)[i]);
  }
  return ret;
}

void copy_triangle(triangle* tn, triangle* to) {
  tn->x1 = to->x1;
  tn->y1 = to->y1;
  tn->x2 = to->x2;
  tn->y2 = to->y2;
  tn->x3 = to->x3;
  tn->y3 = to->y3;
  tn->r = to->r;
  tn->g = to->g;
  tn->b = to->b;
  tn->a = to->a;
}

tri_image* copy_tri_image(tri_image* in, int gen) {
  tri_image* ret = new_tri_image(in->size, gen, in->img->width, in->img->height);
  int i;
  for (i=0;i < in->size; i++)
    copy_triangle(&(ret->triangles)[i],&(in->triangles)[i]);
  return ret;
}

tri_image* cross_tri_image(tri_image* a, tri_image* b, int cross, int gen) {
  tri_image* ret = new_tri_image(a->size, gen, a->img->width, a->img->height);
  int i;
  for (i=0;i < cross; i++)
    copy_triangle(&(ret->triangles)[i],&(a->triangles)[i]);
  for (i=cross; i<a->size; i++)
    copy_triangle(&(ret->triangles)[i],&(b->triangles)[i]);
  return ret;
}

tri_image* expand_tri_image(tri_image* in, int gen, int extra) {
  tri_image* ret = new_tri_image(in->size+extra, gen, in->img->width, in->img->height);
  int i;
  for (i=0;i < in->size; i++)
    copy_triangle(&(ret->triangles)[i],&(in->triangles)[i]);
  for (i=0;i < extra; i++)
    new_triangle(&(ret->triangles)[i+in->size]);
  return ret;
}

void float_mutate(float* f, float bw) {
  float max = *f + bw;
  float min = *f - bw;
  if (max > 1.0)
    max = 1.0;
  if (min < 0.0)
    min = 0.0;
  *f = genrand64_real2() * (max - min) + min;
}

void tri_mutate(tri_image* t, float bw, int tri) {
  int mx = genrand64_int64() % (t->size*10);
  int mt = mx / 10;
  int mp = mx % 10;
  if (tri >= 0)
    mt = tri;

  switch (mp) {
    case 0:
      float_mutate(&((t->triangles)[mt].x1),bw);
      break;
    case 1:
      float_mutate(&((t->triangles)[mt].y1),bw);
      break;
    case 2:
      float_mutate(&((t->triangles)[mt].x2),bw);
      break;
    case 3:
      float_mutate(&((t->triangles)[mt].y2),bw);
      break;
    case 4:
      float_mutate(&((t->triangles)[mt].x3),bw);
      break;
    case 5:
      float_mutate(&((t->triangles)[mt].y3),bw);
      break;
    case 6:
      float_mutate(&((t->triangles)[mt].r),bw);
      break;
    case 7:
      float_mutate(&((t->triangles)[mt].g),bw);
      break;
    case 8:
      float_mutate(&((t->triangles)[mt].b),bw);
      break;
    case 9:
      float_mutate(&((t->triangles)[mt].a),bw);
      break;
    default:
      printf("RNG Fail.\n");
      exit(0);
  }
}

// Shared variables
int generation = 0;
image* isource;

// Stochastic Hill Climber
int shc_size;
tri_image* shc_current = NULL;

tri_image* shc_next() {
  if (shc_current == NULL) {
    return random_tri_image(shc_size, 0, isource->width, isource->height);
  }
  tri_image* next = copy_tri_image(shc_current,++generation);
  tri_mutate(next, 1.0, -1);

  return next;
}

tri_image* shc_best() {
  return shc_current;
}

void shc_process(tri_image* ti) {
  ti->error = image_diff(ti->img,isource);

  if (shc_current == NULL) {
    shc_current = ti;
  } else if (ti->error < shc_current->error) {
    printf("%d,%ld\n", ti->generation, ti->error);
    free_tri_image(shc_current);
    shc_current = ti;
  } else {
    free_tri_image(ti);
  }
}

void shc_init(image* source, int size) {
  isource = source;
  shc_size = size;
}

// Adjusting 1+x Stochastic Hill Climber
int ashc_size, ashc_pop_size, ashc_p;
int ashc_imps, ashc_produced;
long ashc_last_error;
tri_image** ashc_pop;
float ashc_adj_bw;
int ashc_adj_count;

tri_image* ashc_next() {
  if (ashc_p == 0) {
    return random_tri_image(ashc_size, generation++, isource->width, isource->height);
  }
  tri_image* next = copy_tri_image(ashc_pop[0],generation);

  int i;
  //for (i = genrand64_int64() % ashc_adj_count; i < ashc_adj_count; i++) {
  for (i = 0; i < ashc_adj_count; i++) {
    tri_mutate(next, ashc_adj_bw, -1);
  }

  return next;
}

tri_image* ashc_best() {
  return ashc_pop[0];
}

void ashc_children_sort() {
  int i, j;
  for (i = 1; i < ashc_pop_size; i++) {
    tri_image* v = ashc_pop[i];
    j = i-1;
    while (j >= 0 && (ashc_pop[j])->error > v->error) {
      ashc_pop[j+1] = ashc_pop[j];
      j = j - 1;
    }
    ashc_pop[j+1] = v;
  }
}

void ashc_adjust_rates() {
  if (ashc_produced > 1000) {
    ashc_produced /= 2;
    ashc_imps /=2;
  }
  int error = ashc_imps * (ashc_pop_size - 1) - ashc_produced;
  if (error <= 0) {
    if (ashc_adj_count > 1) {
      ashc_adj_count--;
    } else {
      ashc_adj_bw *= .95;
      if (ashc_adj_bw < 0.0001)
        ashc_adj_bw = 0.0001;
    }
  } else {
    if (ashc_adj_bw > .7) {
      ashc_adj_bw = 0.7;
      ashc_adj_count = 2;
      //if (ashc_adj_count > 2)
        //ashc_adj_count = 2;
    } else {
      ashc_adj_bw *= 1.05;
    }
  }
}

void ashc_process(tri_image* ti) {
  ti->error = image_diff(ti->img,isource);

  ashc_pop[ashc_p++] = ti;

  if (ashc_p == ashc_pop_size) {
    ashc_children_sort();

    if ((ashc_pop[0])-> error < ashc_last_error) {
      printf("%d,%ld\n", (ashc_pop[0])->generation, (ashc_pop[0])->error);
      ashc_imps++;
    }

    int i;
    for (i = 1; i < ashc_pop_size; i++) {
      if ((ashc_pop[i])->error < ashc_last_error)
        ashc_imps++;
      free_tri_image(ashc_pop[i]);
    }
    ashc_produced += ashc_pop_size - 1;
    ashc_adjust_rates();
    ashc_p = 1;
    generation++;
    ashc_last_error = (ashc_pop[0])->error;
  }
}

void ashc_init(image* source, int size) {
  isource = source;
  ashc_size = size;
  ashc_pop_size = 7;
  ashc_p = 0;
  ashc_pop = (tri_image**)calloc(ashc_pop_size,sizeof(tri_image*));
  ashc_adj_bw = 1.0;
  ashc_adj_count = size / 10;
  ashc_imps = 0;
  ashc_produced = 0;
}

// Simulated Annealing 
int sa_size;
float sa_bw;
float sa_base = 0.97;
// 1/sa_cut > sa_imps / sa_i is the point at which sa_bw is multiplied by sa_base
int sa_min, sa_i = 0, sa_imps = 0, sa_cut = 15;
tri_image* sa_current = NULL;

tri_image* sa_next() {
  if (sa_current == NULL) {
    return random_tri_image(sa_size, 0, isource->width, isource->height);
  }
  tri_image* next = copy_tri_image(sa_current,++generation);

  int i;
  for (i=0;i<(int)sa_bw+1;i++)
    tri_mutate(next, sa_bw, -1);

  if (sa_i >= sa_min && sa_imps * sa_cut < sa_i) {
    sa_i = 0;
    sa_imps = 0;
    sa_bw *= sa_base;
    if (sa_bw < 0.01) {
      // 'Reheat' the system
      sa_bw = 4;
      // Cool slower
      sa_cut++;
    }
  } 

  return next;
}

tri_image* sa_best() {
  return sa_current;
}

void sa_process(tri_image* ti) {
  ti->error = image_diff(ti->img,isource);

  if (sa_current == NULL) {
    sa_current = ti;
  } else if (ti->error < sa_current->error) {
    printf("%d,%ld,%f\n", ti->generation, ti->error, sa_bw);
    free_tri_image(sa_current);
    sa_current = ti;
    sa_imps++;
  } else {
    free_tri_image(ti);
  }

  sa_i++;
}

void sa_init(image* source, int size) {
  isource = source;
  sa_size = size;
  sa_bw = (float)size;
  sa_min = 2*sa_size;
}

// Accumulator
int acc_max, acc_size = 1, acc_i = 0, acc_per = 100, acc_freq = 2, acc_m=10, acc_force = 0;
tri_image* acc_current = NULL;

tri_image* acc_next() {
  if (acc_current == NULL) {
    return random_tri_image(acc_size, generation, isource->width, isource->height);
  }

  if (acc_size == acc_max) {
    return sa_next();
  }

  tri_image *next;

  int i;
  if (acc_i == acc_per + acc_size/10) {
    acc_i = 0;
    acc_size++;
    next = expand_tri_image(acc_current, ++generation, 1);
    acc_force = 1;
  } else {
    next = copy_tri_image(acc_current, ++generation);
    for (i=0;i<acc_m;i++) {
      if (acc_i % acc_freq == 0) {
        tri_mutate(next, 1.0/((float)i+1), next->size-1);
      } else {
        tri_mutate(next, 1.0/((float)i+1), -1);
      }
    }
  }

  return next;
}

tri_image* acc_best() {
  if (acc_size == acc_max && !acc_force) {
    return sa_best();
  }
  return acc_current;
}

void acc_process(tri_image *ti) {
  ti->error = image_diff(ti->img, isource);

  if (acc_size == acc_max && !acc_force) {
    sa_process(ti);
    return;
  }

  if (acc_current == NULL) {
    acc_current = ti;
  } else if (ti->error < acc_current->error || acc_force) {
    printf("%d,%ld,%d\n", ti->generation, ti->error,acc_size);
    free_tri_image(acc_current);
    acc_current = ti;
    acc_force = 0;
    if (acc_size==acc_max)
      sa_current = acc_current;
  } else {
    free_tri_image(ti);
  }

  acc_i++;
}

void acc_init(image* source, int size) {
  isource = source;
  acc_max = size;
  sa_init(source, size);
  sa_bw = 10;
}

// Genetic Algorithm
tri_image** ga_pop;
int ga_psize, ga_tsize;
float ga_mutate_prob;
float ga_bw;

tri_image* ga_next() {
  if (!ga_pop[ga_psize-1]) {
    return random_tri_image(ga_tsize, generation++, isource->width, isource->height);
  }

  tri_image *mother, *father, *child;

  // Selection (unbiased random)
  // Roulette wheel selection may be interesting to try
  mother = ga_pop[genrand64_int64() % ga_psize];
  father = ga_pop[genrand64_int64() % ga_psize];

  // Reproduction (one point crossover)
  int cross = genrand64_int64() % (ga_tsize + 1);
  child = cross_tri_image(mother, father, cross, generation++);

  // Mutation
  while (genrand64_real2() < ga_mutate_prob) {
    tri_mutate(child, ga_bw, -1);
  }

  return child;
}

tri_image* ga_best() {
  return ga_pop[0];
}

void ga_process(tri_image *ti) {
  ti->error = image_diff(ti->img, isource);
  tri_image *temp;
  int i;
  // Add to population and remove least fit
  for (i=0;i<ga_psize;i++) {
    if (!ga_pop[i]) {
      // Fill population to ga_psize
      ga_pop[i] = ti;
      return;
    }
    if (ti->error < ga_pop[i]->error) {
      if (i == 0)
        printf("%d, %ld\n", ti->generation, ti->error);
      temp = ga_pop[i];
      ga_pop[i] = ti;
      ti = temp;
    }
  }
  free_tri_image(ti);
}

void ga_init(image* source, int tsize, int psize) {
  isource = source;
  ga_tsize = tsize; // Number of triangles
  ga_psize = psize; // Max size of the population
  ga_pop = (tri_image**)calloc(ga_psize, sizeof(tri_image*));
  ga_mutate_prob = 0.5;
  ga_bw = 0.2;
}
