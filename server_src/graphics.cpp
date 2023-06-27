#include "graphics.h"

#include "event_mgr.h"
#include "phantom.h"


Graphics::Graphics(EventMgr* event_mgr, Phantom *phantom) {
    event_mgr_ = event_mgr;
    phantom_ = phantom;
}

Graphics::~Graphics() {
}

bool Graphics::Init() {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("MinVR3 Force Server");
    
    // This simple projection makes it so that you can see the phantom
    // tip position anywhere that it moves.. note, seeing the tip is
    // critical--if the tip goes off screen, then the haptics won't render.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-3.0, 3.0, -3.0, 3.0, -3.0, 3.0);
    
    static const GLfloat light_model_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    static const GLfloat light0_diffuse[] = {0.9f, 0.9f, 0.9f, 0.9f};
    static const GLfloat light0_direction[] = {0.0f, -0.4f, 1.0f, 0.0f};
    
    // Enable depth buffering for hidden surface removal.
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    // Cull back faces.
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    
    // Setup other misc features.
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    
    // Setup lighting model.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
    glEnable(GL_LIGHT0);
}

void Graphics::Run(void (*mainloop_func)(void)) {
    glutDisplayFunc(mainloop_func);
    glutMainLoop(); // note: doesn't return until the program closes
}

void Graphics::Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);

    // DRAW HAPTIC CURSOR
    // from openhaptics example code
    if (phantom_ptr_ != NULL) {
        static const double kCursorRadius = 0.5;
        static const double kCursorHeight = 1.5;
        static const int kCursorTess = 15;

        glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
        glPushMatrix();
        if (!cursor_display_list_) {
            cursor_display_list_ = glGenLists(1);
            glNewList(cursor_display_list_, GL_COMPILE);
            GLUquadricObj* qobj = gluNewQuadric();
            gluCylinder(qobj, 0.0, kCursorRadius, kCursorHeight, kCursorTess, kCursorTess);
            glTranslated(0.0, 0.0, kCursorHeight);
            gluCylinder(qobj, kCursorRadius, 0.0, kCursorHeight / 5.0, kCursorTess, kCursorTess);
            gluDeleteQuadric(qobj);
            glEndList();
        }
        
        // Get the proxy transform in world coordinates.
        glMultMatrixd(phantom->transform());
        // Apply the local cursor scale factor.
        glScaled(gCursorScale, gCursorScale, gCursorScale);
        glEnable(GL_COLOR_MATERIAL);
        if (phantom_->is_primary_btn_down()) {
            glColor3f(1.0f, 0.5f, 1.0f);
        }
        else {
            glColor3f(0.0f, 0.5f, 1.0f);
        }
        glCallList(cursor_display_list_);
        glPopMatrix();
        glPopAttrib();
    }
    

    // DRAW SCENE -- E.G., FORCE EFFECT DEBUGGING GRAPHICS
    glBegin(GL_POLYGON);
    glVertex3f(0.25, 0.25, 0.0);
    glVertex3f(0.75, 0.25, 0.0);
    glVertex3f(0.75, 0.75, 0.0);
    glVertex3f(0.25, 0.75, 0.0);
    glEnd();
}
