#include <iostream>

#ifdef __APPLE__

#include <OpenGL/gl.h>
#include <GLUT/glut.h>

#else

#include <GL/gl.h>
#include <GL/glut.h>

#endif

#include <neu/NProgram.h>
#include <neu/NPQueue.h>
#include <neu/NPLParser.h>
#include <neu/NPLVector.h>
#include <neu/NPLObject.h>
#include <neu/NPLModule.h>
#include <neu/NSys.h>

using namespace std;
using namespace neu;
using namespace npl;

static const size_t WINDOW_WIDTH = 1024;
static const size_t WINDOW_HEIGHT = 1024;

static const float WIDTH = 35;
static const float HEIGHT = 35;
static const float ROWS = 256;
static const float COLUMNS = 256;
static const float HEIGHT_MULTIPLIER = 2;

// the run queue will execute Cell::run() 
static NPQueue runQueue(8);

// the run queue will execute Cell::interact() 
static NPQueue interactQueue(8);

inline void glVertex(const float3& u){
  glVertex3f(u[0], u[1], u[2]);
}

inline void toRGB(const float3& hsv, float3& rgb){
  static int i;
  static float f, p, q, t, h;
  
  if(hsv[1] == 0){
    rgb[0] = rgb[1] = rgb[2] = hsv[2];
    return;
  }

  h = hsv[0] / 60;

  i = h;
  f = h - i;
  p = hsv[2] * (1 - hsv[1]);
  q = hsv[2] * (1 - hsv[1] * f);
  t = hsv[2] * (1 - hsv[1] * (1 - f));

  switch(i){
    case 0:
      rgb[0] = hsv[2];
      rgb[1] = t;
      rgb[2] = p;
      break;
    case 1:
      rgb[0] = q;
      rgb[1] = hsv[2];
      rgb[2] = p;
      break;
    case 2:
      rgb[0] = p;
      rgb[1] = hsv[2];
      rgb[2] = t;
      break;
    case 3:
      rgb[0] = p;
      rgb[1] = q;
      rgb[2] = hsv[2];
      break;
    case 4:
      rgb[0] = t;
      rgb[1] = p;
      rgb[2] = hsv[2];
      break;
    default:
      rgb[0] = hsv[2];
      rgb[1] = p;
      rgb[2] = q;
      break;
  }
}

class Cell : public NPLObject{
public:
  Cell()
    : h(0),
      s(0){
    
  }

  // the C++ side object's data layout should match the NPL-side
  // defined in kernel.npl
  double h;
  double s;

  // however we could add additional C++ only data items here
};

typedef NVector<Cell*> CellVec;

CellVec cellVec;

void initCells(){
  for(size_t i = 0; i < ROWS; ++i){
    for(size_t j = 0; j < COLUMNS; ++j){
      cellVec.push_back(new Cell);
    }
  }
}

inline Cell* getCell(int i, int j){
  if(i >= ROWS){
    return 0;
  }
  else if(i < 0){
    return 0;
  }
  
  if(j >= COLUMNS){
    return 0;
  }
  else if(j < 0){
    return 0;
  }

  return cellVec[i + j * ROWS];
}

NPLModule module;

class InteractFunc : public NPLFunc{
public:
  // normally the return type would go here, but in this case it is void

  // the arguments to the method go here in order of appearance
  Cell* c;
};

void addNeighbor(Cell* c1, Cell* c2){
  if(c1 && c2){
    InteractFunc* f = new InteractFunc;
    // get the interact method we required
    module.getFunc({"Cell", "interact", 1 /* number of args */}, f);

    // we need to set the object (the this pointer)
    f->o = c1;

    // set the first and only argumet to the method
    f->c = c2;

    interactQueue.add(f);
  }
}

void initBehavior(){
  for(size_t i = 0; i < ROWS; ++i){
    for(size_t j = 0; j < COLUMNS; ++j){
      Cell* c = getCell(i, j);

      NPLFunc* f = new NPLFunc;
      module.getFunc({"Cell", "run", 0}, f);
      f->o = c;

      runQueue.add(f);

      addNeighbor(c, getCell(i - 1, j));
      addNeighbor(c, getCell(i + 1, j));
      addNeighbor(c, getCell(i, j - 1));
      addNeighbor(c, getCell(i, j + 1));
      addNeighbor(c, getCell(i - 1, j - 1));
      addNeighbor(c, getCell(i + 1, j + 1));
      addNeighbor(c, getCell(i - 1, j + 1));
      addNeighbor(c, getCell(i + 1, j - 1));
    }
  }
}

void drawCells(){
  glMatrixMode(GL_MODELVIEW);

  static float3 v1;
  static float3 v2;
  static float3 rgb;
  static float3 hsv = {1, 1, 1};
  static const float cellWidth = WIDTH/(COLUMNS - 1);
  static const float cellHeight = HEIGHT/(ROWS - 1);
  static const float left = -cellWidth*(COLUMNS - 1)/2;
  static const float top = -cellHeight*(ROWS - 1)/2;
  
  for(size_t i = 0; i < COLUMNS - 1; i++){
    glBegin(GL_TRIANGLE_STRIP);

    for(size_t j = 0; j < ROWS; j++){

      Cell* c1 = getCell(i, j);
      Cell* c2 = getCell(i + 1, j);
      
      hsv[0] = 200.0 + c1->h*160.0;
      toRGB(hsv, rgb);

      glColor4f(rgb[0], rgb[1], rgb[2], 1);

      v1[0] = left + i * cellWidth;
      v1[1] = top + j * cellHeight;
      v1[2] = c1->h * HEIGHT_MULTIPLIER;

      v2[0] = left + (i + 1) * cellWidth;
      v2[1] = top + j * cellHeight;
      v2[2] = c2->h * HEIGHT_MULTIPLIER;
        
      glVertex3f(v1[0], v1[1], v1[2]);
      glVertex3f(v2[0], v2[1], v2[2]);
    }

    glEnd();
  }
}

void animate(){
  runQueue.run();
  interactQueue.run();

  glutPostRedisplay();
}

void display(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(0.0, 0.0, 50.0,
	    0.0, 0.0, 0.0,
	    0.0, 1.0, 0.0);

  glRotatef(-70.0, 1.0, 0.0, 0.0);

  drawCells();

  NSys::sleep(0.0001);

  glutSwapBuffers();
}

void initGL(void){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  gluPerspective(60.0,
                 static_cast<float>(WINDOW_WIDTH)/WINDOW_HEIGHT, 0.1, 1024); 

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  
  glShadeModel(GL_SMOOTH);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glClearColor(0.717647, 0.717647, 0.717647, 1);
}

int main(int argc, char** argv){
  NProgram program(argc, argv);

  srand48(818);

  glutInit(&argc, argv);

  NPLParser parser;
  nvar code = parser.parseFile("kernel.npl");

  cout << "code is: " << code << endl;
  
  if(!module.compile(code)){
    cerr << "failed to compile" << endl;
    return 1;
  }

  initCells();
  initBehavior();

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutCreateWindow("Waves");
  initGL();
  glutDisplayFunc(display);
  glutIdleFunc(animate);
  glutMainLoop();  

  return 0;
}
