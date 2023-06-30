#include "graphics.h"

#include "event_mgr.h"
#include "phantom.h"


Graphics::Graphics(EventMgr* event_mgr, Phantom *phantom) {
    event_mgr_ = event_mgr;
    phantom_ = phantom;
}

Graphics::~Graphics() {

}

bool Graphics::Init(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("MinVR3 Force Server");

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    static const GLfloat light_model_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    static const GLfloat light0_diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    static const GLfloat light0_direction[] = { 0.0f, 0.5f, 0.5f, 0.0f };
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
    glEnable(GL_LIGHT0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 1, 1, 1400);

    return true;
}

//void glutIdle() {
//    glutPostRedisplay();
//}

void Graphics::Run(void (*mainloop_func)(void)) {
    //glutIdleFunc(glutIdle);
    glutDisplayFunc(mainloop_func);
    glutMainLoop(); // note: doesn't return until the program closes
}

void Graphics::Clear(bool clear_color, bool clear_depth) {
    if (clear_color && clear_depth) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else if (clear_depth) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    else if (clear_color) {
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void Graphics::DrawGraphics() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 850, 0, 0, 0, 0, 1, 0);

    // DRAW FLOOR PLANE
    glDisable(GL_LIGHTING);

    // solid gray square
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_TRIANGLES);
    glVertex3f(-300, -300.5, 300);
    glVertex3f(300, -300.5, 300);
    glVertex3f(-300, -300.5, -300);
    glVertex3f(300, -300.5, 300);
    glVertex3f(300, -300.5, -300);
    glVertex3f(-300, -300.5, -300);
    glEnd();

    // grid of lines
    glColor3f(0.2, 0.2, 0.2);
    glBegin(GL_LINES);
    for (float f = -300; f <= 300; f += 10) {
        glVertex3f(f, -300, -300);
        glVertex3f(f, -300, 300);

        glVertex3f(-300, -300, f);
        glVertex3f(300, -300, f);
    }
    glEnd();
    glEnable(GL_LIGHTING);


    if (phantom_ != NULL) {
        phantom_->DrawGraphics();
    }
}

void Graphics::SwapBuffers() {
    glutSwapBuffers();
}
