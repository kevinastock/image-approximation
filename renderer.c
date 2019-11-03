//////////////////////////////////////////////////////// 
// Kevin Stock 
//////////////////////////////////////////////////////// 

// This file contains all the OpenGL related code for rendering tri_images
// and showing the best image rendered.

#include <GL/glut.h> 
#include <stdio.h>
#include <stdlib.h>
#include "tr.h"
#include "wiproj.h"

#define BORDER 5

// Program State
int window_width=500, window_height=500;

float img_aspect;

/* Update Show Values:
 *  0 - No rerendering needed
 *  1 - Render best to screen
 */
int update_show = 0;

tri_image * render;
tri_image * shown;

tri_image*  (*mh_next)(void);
tri_image*  (*mh_best)(void);
void        (*mh_process)(tri_image*);

void draw_tri_image(tri_image* ti) {
  glClear(GL_COLOR_BUFFER_BIT);
  int i;
  glBegin(GL_TRIANGLES);
  for (i = 0; i < ti->size; i++) {
    triangle t = (ti->triangles)[i];
    glColor4f(t.r,t.g,t.b,t.a);
    glVertex2f(t.x1,t.y1);
    glVertex2f(t.x2,t.y2);
    glVertex2f(t.x3,t.y3);
  }
  glEnd();
}

void display() { 
  if (render->state == 0) {
    // render this tri_image
    TRcontext* t = trNew();
    trTileSize(t,window_width,window_height,BORDER);
    trImageSize(t,render->img->width,render->img->height);
    trImageBuffer(t, GL_RGB, GL_UNSIGNED_BYTE, render->img->values);
    trOrtho(t,0.0,1.0,0.0,1.0,-1.0,1.0);
    do {
      trBeginTile(t);
      draw_tri_image(render);
    } while (trEndTile(t));
    trDelete(t);
    render->state = 1;
  }

  if (update_show == 1) {
    float w_aspect = (float) window_width / (float) window_height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w_aspect > img_aspect) {
      gluOrtho2D(0.5-(w_aspect/img_aspect)/2.0,0.5+(w_aspect/img_aspect)/2.0,0,1);
    } else {
      gluOrtho2D(0,1,0.5-(img_aspect/w_aspect)/2.0,0.5+(img_aspect/w_aspect)/2.0);
    }
    glMatrixMode(GL_MODELVIEW);
    draw_tri_image(shown);
    glFlush();
    glutSwapBuffers();
    update_show = 0;
  }
} 

void reshape(GLsizei w, GLsizei h) {
  window_width = w;
  window_height = h;
  update_show = 1;
  glViewport(0,0,window_width,window_height);
  glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y) {
  if (key == 'q') exit(0);
  if (key == 's') {
    char buf[20];
    snprintf(buf,20,"%d.ppm",shown->generation);
    FILE* out = fopen(buf, "wb");
    write_ppm(out,shown->img);
    fclose(out);
  }
  glutPostRedisplay();
}

void idle() {
  glutPostRedisplay();
  if (render->state == 0) return;

  mh_process(render);
  render = mh_next();

  tri_image* temp = mh_best();
  if (temp != shown) {
    update_show = 1;
    shown = temp;
  }

}

// Initial GL settings
void myInit() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,1,0,1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}


void start(
    int argc, char** argv,
    tri_image*  (*next)(void),
    tri_image*  (*best)(void),
    void        (*process)(tri_image*)) { 
  // Renderer Initializations
  mh_next = next;
  mh_best = best;
  mh_process = process;
  render = mh_next();
  shown = render;
  update_show = 1;

  img_aspect = (float) render->img->width / (float) render->img->height;
  //img_aspect_inv = (float) render->img->height / (float) render->img->width;
  // Glut Initializations
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(window_width,window_height);
  glutCreateWindow("wiproj");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(idle);

  // GL Initialization
  myInit();

  glutMainLoop();
} 
