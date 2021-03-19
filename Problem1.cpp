
#include <windows.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define pi (2*acos(0.0))
#define EPS 1e-6

double cameraHeight;
double cameraAngle;
int drawgrid;
int drawaxes;
double angle;
double rotationConst = 2;
double moveConst = 2;

struct point
{
	double x,y,z;
	point() {x = y = z = 0.0;}
    point(double _x, double _y, double _z): x(_x), y(_y), z(_z) {}

    bool operator == (point another) const{
        return fabs(x - another.x) < EPS && fabs(y - another.y) < EPS && fabs(z - another.z) < EPS;
    }

	point operator +(point another) {
		return point(x + another.x, y + another.y, z + another.z);
	}

	point operator +=(point another) {
		x += another.x, y += another.y, z += another.z;
		return point(x + another.x, y + another.y, z + another.z);
	}

	point operator -(point another) {
		return point(x - another.x, y - another.y, z - another.z);
	}

	point operator -=(point another) {
		x -= another.x, y -= another.y, z -= another.z;
		return point(x - another.x, y - another.y, z - another.z);
	}

	point operator *(double mul) {
		return point(x * mul, y * mul, z * mul);
	}

	point operator *=(double mul) {
		x *= mul, y *= mul, z *= mul;
		return point(x * mul, y * mul, z * mul);
	}

	point operator *(point another) {
		double xx = y * another.z - z * another.y;
		double yy = z * another.x - x * another.z;
		double zz = x * another.y - y * another.x;
		return point(xx, yy, zz);
	}

	point rotate(point axis, double degree) {
		point vect(x, y, z);
		return (vect * cos(degree * (pi/180)) + ( (axis * vect) * sin(degree* (pi/180))) ); // rotate r around u by rotationConst degree
	}

};

point makeUnitVector(point vect) {
	double r = sqrt(vect.x * vect.x + vect.y * vect.y + vect.z * vect.z);
	return point(vect.x/r, vect.y/r, vect.z/r);
}

// point makeUnitVector(point p1, point p2) {
// 	double x = fabs(p1.x - p2.x) < EPS ? 0.0 : (p1.x - p2.x)/(p1.x - p2.x);
// 	double y = fabs(p1.y - p2.y) < EPS ? 0.0 : (p1.y - p2.y)/(p1.y - p2.y);
// 	double z = fabs(p1.z - p2.z) < EPS ? 0.0 : (p1.z - p2.z)/(p1.z - p2.z);
// 	return point(x, y, z);
// }

point cameraPos( 100, 100, 0);
point u(0, 0, 1), r(-1/sqrt(2), 1/sqrt(2), 0), l(-1/sqrt(2), -1/sqrt(2), 0);

// gun constants
double gunAngle = 0;
double gunSphereAngle = 0;
double gunBarrelAngle = 0;
double gunBarrelRotateAngle = 0;

struct bullet {
	double gunAngle, gunSphereAngle, gunBarrelAngle, gunBarrelRotateAngle;
	bullet() {gunAngle = gunSphereAngle = gunBarrelAngle = gunBarrelRotateAngle = 0;}
	bullet(double _gunAngle, double _gunSphereAngle, double _gunBarrelAngle, double _gunBarrelRotateAngle) {
		gunAngle = _gunAngle;
		gunSphereAngle = _gunSphereAngle;
		gunBarrelAngle = _gunBarrelAngle;
		gunBarrelRotateAngle = _gunBarrelRotateAngle;
	}
};
#define maxBullet 50
int numberOfbullets = 0;
bullet bullets[maxBullet];
double whiteboardDistance = 800;
double whiteboardLength = 400;

void incrementAngle(double &angle, double angleLimit) {
	if (angle + rotationConst < angleLimit)
		angle += rotationConst;
}

void decrementAngle(double &angle, double angleLimit) {
	if (angle - rotationConst > -angleLimit)
		angle -= rotationConst;
}



void drawAxes()
{
	if(drawaxes==1)
	{
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_LINES);{
			glVertex3f( 100,0,0);
			glVertex3f(-100,0,0);

			glVertex3f(0,-100,0);
			glVertex3f(0, 100,0);

			glVertex3f(0,0, 100);
			glVertex3f(0,0,-100);
		}glEnd();
	}
}


void drawGrid()
{
	int i;
	if(drawgrid==1)
	{
		glColor3f(0.6, 0.6, 0.6);	//grey
		glBegin(GL_LINES);{
			for(i=-8;i<=8;i++){

				if(i==0)
					continue;	//SKIP the MAIN axes

				//lines parallel to Y-axis
				glVertex3f(i*10, -90, 0);
				glVertex3f(i*10,  90, 0);

				//lines parallel to X-axis
				glVertex3f(-90, i*10, 0);
				glVertex3f( 90, i*10, 0);
			}
		}glEnd();
	}
}

void drawSquare(double a)
{
    //glColor3f(1.0,0.0,0.0);
	glBegin(GL_QUADS);{
		glVertex3f( a, a,2);
		glVertex3f( a,-a,2);
		glVertex3f(-a,-a,2);
		glVertex3f(-a, a,2);
	}glEnd();
}


void drawCircle(double radius,int segments)
{
    int i;
    struct point points[100];
    glColor3f(0.7,0.7,0.7);
    //generate points
    for(i=0;i<=segments;i++)
    {
        points[i].x=radius*cos(((double)i/(double)segments)*2*pi);
        points[i].y=radius*sin(((double)i/(double)segments)*2*pi);
    }
    //draw segments using generated points
    for(i=0;i<segments;i++)
    {
        glBegin(GL_LINES);
        {
			glVertex3f(points[i].x,points[i].y,0);
			glVertex3f(points[i+1].x,points[i+1].y,0);
        }
        glEnd();
    }
}

void drawCylinder(double radius,double height,int segments)
{
    int i;
    double shade;
	int alternateColor = 1;
    struct point points[100];
    //generate points
    for(i=0;i<=segments;i++)
    {
        points[i].x=radius*cos(((double)i/(double)segments)*2*pi);
        points[i].y=radius*sin(((double)i/(double)segments)*2*pi);
    }
    //draw triangles using generated points
    for(i=0;i<segments;i++)
    {
        //create shading effect
        if(i<segments/2)shade=2*(double)i/(double)segments;
        else shade=2*(1.0-(double)i/(double)segments);
        // alternateColor colors
		if (alternateColor)
 	       glColor3f(0, 0, 0);
		else 
 	       glColor3f(1, 1, 1);
		alternateColor = 1 - alternateColor;

		glBegin(GL_QUADS);{
			glVertex3f(points[i].x, points[i].y, 0);
			glVertex3f(points[i+1].x, points[i+1].y, 0);
			glVertex3f(points[i].x, points[i].y, height);
			glVertex3f(points[i+1].x, points[i+1].y, height);
		}glEnd();
    }
}

void drawSemiSphere(double radius, int slices, int stacks) {
	struct point points[100][100];
	int i,j;
	double h,r;
	int alternateColor = 1;
	//generate points
	for(i=0;i<=stacks;i++)
	{
		h=radius*sin(((double)i/(double)stacks)*(pi/2));
		r=radius*cos(((double)i/(double)stacks)*(pi/2));
		for(j=0;j<=slices;j++)
		{
			points[i][j].x=r*cos(((double)j/(double)slices)*2*pi);
			points[i][j].y=r*sin(((double)j/(double)slices)*2*pi);
			points[i][j].z=h;
		}
	}
	//draw quads using generated points
	for(i=0;i<stacks;i++) {

		for(j=0;j<slices;j++) {
        // alternateColor colors
		if (alternateColor)
 	       glColor3f(0, 0, 0);
		else 
 	       glColor3f(1, 1, 1);
		alternateColor = 1 - alternateColor;
			glBegin(GL_QUADS);{

				glVertex3f(points[i][j].x,points[i][j].y,points[i][j].z);
				glVertex3f(points[i][j+1].x,points[i][j+1].y,points[i][j+1].z);
				glVertex3f(points[i+1][j+1].x,points[i+1][j+1].y,points[i+1][j+1].z);
				glVertex3f(points[i+1][j].x,points[i+1][j].y,points[i+1][j].z);

			}glEnd();
		}
	}
}


void drawSphere(double radius,int slices,int stacks)
{
	struct point points[100][100];
	int i,j;
	double h,r;
	//generate points
	for(i=0;i<=stacks;i++)
	{
		h=radius*sin(((double)i/(double)stacks)*(pi/2));
		r=radius*cos(((double)i/(double)stacks)*(pi/2));
		for(j=0;j<=slices;j++)
		{
			points[i][j].x=r*cos(((double)j/(double)slices)*2*pi);
			points[i][j].y=r*sin(((double)j/(double)slices)*2*pi);
			points[i][j].z=h;
		}
	}
	//draw quads using generated points
	for(i=0;i<stacks;i++)
	{
        glColor3f((double)i/(double)stacks,(double)i/(double)stacks,(double)i/(double)stacks);
		for(j=0;j<slices;j++)
		{
			glBegin(GL_QUADS);{
			    //upper hemisphere
				glVertex3f(points[i][j].x,points[i][j].y,points[i][j].z);
				glVertex3f(points[i][j+1].x,points[i][j+1].y,points[i][j+1].z);
				glVertex3f(points[i+1][j+1].x,points[i+1][j+1].y,points[i+1][j+1].z);
				glVertex3f(points[i+1][j].x,points[i+1][j].y,points[i+1][j].z);
                //lower hemisphere
                glVertex3f(points[i][j].x,points[i][j].y,-points[i][j].z);
				glVertex3f(points[i][j+1].x,points[i][j+1].y,-points[i][j+1].z);
				glVertex3f(points[i+1][j+1].x,points[i+1][j+1].y,-points[i+1][j+1].z);
				glVertex3f(points[i+1][j].x,points[i+1][j].y,-points[i+1][j].z);
			}glEnd();
		}
	}
}

void drawBullets() {
	for (int idx = 0; idx < numberOfbullets; idx ++) {
		glPushMatrix();
		{
			glColor3f(1,0,0);
			glRotatef(bullets[idx].gunAngle,0,1,0);
			glRotatef(-180 + bullets[idx].gunSphereAngle,1,0,0);
			glRotatef(-180 ,0,1,0);
			glRotatef(bullets[idx].gunBarrelAngle, 1, 0, 0);
			glTranslatef(0, 0, -(whiteboardDistance - 2));
			drawSquare(4);
		}
		glPopMatrix();
	}
}

void solve() {

	// making an initial rotation
	glRotatef(90,1,0,0);
	// drawing white board
	glPushMatrix();
    {
        // glRotatef(angle,0,0,1);
        glTranslatef(0, 0, -whiteboardDistance);
        glColor3f(0.6, 0.6, 0.6);
        drawSquare(whiteboardLength);
    }
    glPopMatrix();
	glPushMatrix();
    {
		// drawing the first semisphere
		{
			glColor3f(1,0,0);
			glRotatef(gunAngle,0,1,0);
			drawSemiSphere(30, 100, 20);
		}
		// drawing the second semisphere
		{
			glColor3f(1,0,0);
			glRotatef(-180 + gunSphereAngle,1,0,0);
			drawSemiSphere(30, 100, 20);
		}
		// drawing the litol semisphere
		{
			glColor3f(1,0,0);
			glTranslatef(0, 0, 30);
			glRotatef(-180 ,0,1,0);
			glRotatef(-gunBarrelAngle, 1, 0, 0);
			glRotatef(gunBarrelRotateAngle, 0, 0, 1);
			glTranslatef(0, 0, -10);
			drawSemiSphere(10, 100, 20);
		}

		// drawing the barrel
		{
			glRotatef(-180 ,0,1,0);
			drawCylinder(10, 100, 50);
		}
	}
    glPopMatrix();
	drawBullets();
}


void keyboardListener(unsigned char key, int x,int y){
	switch(key){

		case '1':
			r = r.rotate(u, rotationConst); // rotate r around u by rotationConst degree
			// r = makeUnitVector(r);
			l = l.rotate(u, rotationConst); // rotate l around u by rotationConst degree
			// l = makeUnitVector(l);
			break;
		case '2':
			r = r.rotate(u, -rotationConst); // rotate r around u by -rotationConst degree
			// r = makeUnitVector(r);
			l = l.rotate(u, -rotationConst); // rotate l around u by -rotationConst degree
			// l = makeUnitVector(l);
			break;
		case '3':
			l = l.rotate(r, rotationConst); // rotate l around r by -rotationConst degree
			// r = makeUnitVector(r);
			u = u.rotate(r, rotationConst); // rotate u around r by -rotationConst degree
			// l = makeUnitVector(l);
			break;
		case '4':
			l = l.rotate(r, -rotationConst); // rotate l around r by -rotationConst degree
			// r = makeUnitVector(r);
			u = u.rotate(r, -rotationConst); // rotate u around r by -rotationConst degree
			// l = makeUnitVector(l);
			break;
		case '5':
			r = r.rotate(l, rotationConst); // rotate l around r by -rotationConst degree
			// r = makeUnitVector(r);
			u = u.rotate(l, rotationConst); // rotate u around r by -rotationConst degree
			// l = makeUnitVector(l);
			break;
		case '6':
			r = r.rotate(l, -rotationConst); // rotate l around r by -rotationConst degree
			// r = makeUnitVector(r);
			u = u.rotate(l, -rotationConst); // rotate u around r by -rotationConst degree
			// l = makeUnitVector(l);
			break;
		case 'q':
			incrementAngle(gunAngle, 45);
			break;
		case 'w':
			decrementAngle(gunAngle, 45);
			break;
		case 'e':
			incrementAngle(gunSphereAngle, 45);
			break;
		case 'r':
			decrementAngle(gunSphereAngle, 45);
			break;
		case 'a':
			incrementAngle(gunBarrelAngle, 45);
			break;
		case 's':
			decrementAngle(gunBarrelAngle, 45);
			break;
		case 'd':
			incrementAngle(gunBarrelRotateAngle, 45);
			break;
		case 'f':
			decrementAngle(gunBarrelRotateAngle, 45);
			break;
		default:
			break;
	}
}


void specialKeyListener(int key, int x,int y){
	switch(key){
		case GLUT_KEY_DOWN:		//down arrow key
			cameraPos -= l * moveConst;
			break;
		case GLUT_KEY_UP:		// up arrow key
			cameraPos += l * moveConst;
			break;

		case GLUT_KEY_RIGHT:
			cameraPos += r * moveConst;
			break;
		case GLUT_KEY_LEFT:
			cameraPos -= r * moveConst;
			break;

		case GLUT_KEY_PAGE_UP:
			cameraPos += u * moveConst;
			break;
		case GLUT_KEY_PAGE_DOWN:
			cameraPos -= u * moveConst;
			break;

		case GLUT_KEY_INSERT:
			break;

		case GLUT_KEY_HOME:
			break;
		case GLUT_KEY_END:
			break;

		default:
			break;
	}
}

bool withinLimit(double angle, double limit) {
	return fabs(angle) < limit;
}


void mouseListener(int button, int state, int x, int y){	//x, y is the x-y of the screen (2D)
	switch(button){
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_DOWN){		// 2 times?? in ONE click? -- solution is checking DOWN or UP
				double  angleLimit = atan2 (whiteboardLength, whiteboardDistance) * 180 / pi;
				if ( withinLimit(gunAngle, angleLimit) && 
				withinLimit(gunSphereAngle, angleLimit) &&
				withinLimit(gunBarrelAngle, angleLimit)) {
					bullets[numberOfbullets] = bullet(gunAngle, gunSphereAngle, gunBarrelAngle, gunBarrelRotateAngle);
					numberOfbullets++;
				}
			}
			break;

		case GLUT_RIGHT_BUTTON:
		    if(state == GLUT_DOWN){		// 2 times?? in ONE click? -- solution is checking DOWN or UP
                drawgrid = 1 - drawgrid;
			}
			break;

		case GLUT_MIDDLE_BUTTON:
			if(state == GLUT_DOWN){		// 2 times?? in ONE click? -- solution is checking DOWN or UP
				drawaxes = 1 - drawaxes;
			}
			break;

		default:
			break;
	}
}



void display(){

	//clear the display
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0,0,0,0);	//color black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/********************
	/ set-up camera here
	********************/
	//load the correct matrix -- MODEL-VIEW matrix
	glMatrixMode(GL_MODELVIEW);

	//initialize the matrix
	glLoadIdentity();

	//now give three info
	//1. where is the camera (viewer)?
	//2. where is the camera looking?
	//3. Which direction is the camera's UP direction?

	// gluLookAt(100,100,100,	0,0,0,	0,0,1);
	//gluLookAt(200*cos(cameraAngle), 200*sin(cameraAngle), cameraHeight,		0,0,0,		0,0,1);
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,	cameraPos.x + l.x, cameraPos.y + l.y, cameraPos.z + l.z,	u.x, u.y, u.z);


	//again select MODEL-VIEW
	glMatrixMode(GL_MODELVIEW);


	/****************************
	/ Add your objects from here
	****************************/
	//add objects

	drawAxes();
	drawGrid();

    //glColor3f(1,0,0);
    //drawSquare(10);

	solve();

    //drawCircle(30,24);

    //drawCone(20,50,24);

	//drawSphere(30,24,20);




	//ADD this line in the end --- if you use double buffer (i.e. GL_DOUBLE)
	glutSwapBuffers();
}


void animate(){
	angle+=0.05;
	//codes for any changes in Models, Camera
	glutPostRedisplay();
}

void init(){
	//codes for initialization
	drawgrid=1;
	drawaxes=1;
	cameraHeight=150.0;
	cameraAngle=1.0;
	angle=0;

	//clear the screen
	glClearColor(0,0,0,0);

	/************************
	/ set-up projection here
	************************/
	//load the PROJECTION matrix
	glMatrixMode(GL_PROJECTION);

	//initialize the matrix
	glLoadIdentity();

	//give PERSPECTIVE parameters
	gluPerspective(80,	1,	1,	1000.0);
	//field of view in the Y (vertically)
	//aspect ratio that determines the field of view in the X direction (horizontally)
	//near distance
	//far distance
}

int main(int argc, char **argv){
    point p1(2, 4, 6);
    point p2(1, 2, 3);
    if ( p1 * 2.0 == p2) {
        printf("Equal");
    }
	glutInit(&argc,argv);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);	//Depth, Double buffer, RGB color

	glutCreateWindow("My OpenGL Program");

	init();

	glEnable(GL_DEPTH_TEST);	//enable Depth Testing

	glutDisplayFunc(display);	//display callback function
	glutIdleFunc(animate);		//what you want to do in the idle time (when no drawing is occuring)

	glutKeyboardFunc(keyboardListener);
	glutSpecialFunc(specialKeyListener);
	glutMouseFunc(mouseListener);

	glutMainLoop();		//The main loop of OpenGL

	return 0;
}
