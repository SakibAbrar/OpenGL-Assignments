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
double squareLength = 120;
double bigCircleRadius = 65;
double smallCircleRadius = 10;
bool pause;
double speed, maxSpeed, minSpeed;

struct point
{
	double x,y,z;
	point() {x = y = z = 0.0;}
    point(double _x, double _y, double _z): x(_x), y(_y), z(_z) {}
    point(double _x, double _y): x(_x), y(_y), z(0.0) {}
    point(point from, point to){
		x = to.x - from.x;
		y = to.y - from.y;
		z = to.z - from.z;
	}

    bool operator == (point another) const{
        return fabs(x - another.x) < EPS && fabs(y - another.y) < EPS && fabs(z - another.z) < EPS;
    }

	point operator -() {
		return point(-x, -y, -z);
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

	double abs() {
		return sqrt(x * x + y * y + z * z);
	}

	double operator |(point another) {
		return x * another.x + y * another.y + z * another.z;
	}

	point unitVector() {
		double r = sqrt(x * x + y * y + z * z);
		return point(x/r, y/r, z/r);
	}

	point projection(point on) {
		point self(x, y, z);
		return on.unitVector() * ( (self|on)/on.abs() );
	}

	// 3D rotation
	point rotate(point axis, double degree) {
		point vect(x, y, z);
		return (vect * cos(degree * (pi/180)) + ( (axis * vect) * sin(degree* (pi/180))) ); // rotate r around u by rotationConst degree
	}

	// 2D rotation
	point rotate(double degree) {
		point ans;
		ans.x = cos(degree * (pi/180)) * x - sin(degree * (pi/180))* y;
    	ans.y = sin(degree * (pi/180)) * x + cos(degree * (pi/180))* y;
		return ans;
	}

	point perpendiculer2D() {
		return point(y, -x);
	}

};

#define numberOfCircle 5
bool insideBigCircle[numberOfCircle];
point circles[numberOfCircle], velocity[numberOfCircle];

void setSpeedOfCircles(double speed) {
	for (int idx = 0; idx < numberOfCircle; idx++) {
		velocity[idx].x *= speed;
		velocity[idx].y *= speed;
		velocity[idx].z *= speed;
	}
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
    glColor3f(1.0,0.0,0.0);
	glBegin(GL_LINES);
        glVertex2f(a,a);
        glVertex2f(a,-a);
    glEnd();

    glBegin(GL_LINES);
        glVertex2f(a,-a);
        glVertex2f(-a,-a);
    glEnd();

    glBegin(GL_LINES);
        glVertex2f(-a,-a);
        glVertex2f(-a,a);
    glEnd();

    glBegin(GL_LINES);
        glVertex2f(-a,a);
        glVertex2f(a,a);
    glEnd();
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

void drawSmallCircles(int num) {
	for(int idx = 0; idx < num; idx++) {
       	glPushMatrix();
        {
           glTranslatef(circles[idx].x, circles[idx].y, 0);
           drawCircle(smallCircleRadius, 30);
        }
       	glPopMatrix();
    }
}

void keyboardListener(unsigned char key, int x,int y){
	switch(key){

		case 'p':
			pause = !pause;
			break;
		default:
			break;
	}
}


void specialKeyListener(int key, int x,int y){
	switch(key){
		case GLUT_KEY_DOWN:		//down arrow key
			break;
		case GLUT_KEY_UP:		// up arrow key
			break;

		case GLUT_KEY_RIGHT:
			break;
		case GLUT_KEY_LEFT:
			break;

		case GLUT_KEY_PAGE_UP:
			break;
		case GLUT_KEY_PAGE_DOWN:
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


void mouseListener(int button, int state, int x, int y){	//x, y is the x-y of the screen (2D)
	switch(button){
		case GLUT_LEFT_BUTTON:
			break;

		case GLUT_RIGHT_BUTTON:
			break;

		case GLUT_MIDDLE_BUTTON:
			//........
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
	gluLookAt(0,0,200,	0,0,0,	0,1,0);


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

	drawSmallCircles(numberOfCircle);
    drawSquare(squareLength);
    drawCircle(bigCircleRadius, 50);
    //drawCircle(30,24);

    //drawCone(20,50,24);

	//drawSphere(30,24,20);




	//ADD this line in the end --- if you use double buffer (i.e. GL_DOUBLE)
	glutSwapBuffers();
}

void handleSideBarCollision(int idx) {
	if( squareLength - fabs(circles[idx].x) < smallCircleRadius ) {
        velocity[idx].x = -velocity[idx].x;
    }

    if( squareLength - fabs(circles[idx].y) < smallCircleRadius ) {
        velocity[idx].y = -velocity[idx].y;
    }
}

void handleInsideBigCircle(int idx) {
	 if( circles[idx].abs() < bigCircleRadius - smallCircleRadius )
        insideBigCircle[idx] = true;
}

void handleSmallCircleCollision(int idx) {
	
	for(int other = 0; other < numberOfCircle; other++) {
		if ( other == idx ) continue;
		point dist = circles[idx] - circles[other];
		point after1 = circles[idx] + velocity[idx] * 10;
		point after2 = circles[other] + velocity[other] * 10;
		point distAfter = after1 - after2;
		if( distAfter.abs() > 2 * smallCircleRadius ) return;
        if( 2 * smallCircleRadius - dist.abs() < 0.1 ) {
			// printf("bichite bichite dhakka!");
            point r1r2 = circles[idx] - circles[other];
            point r1r2Perp = r1r2.perpendiculer2D();
            point v1Hor = velocity[idx].projection(r1r2);
            point v1Ver = velocity[idx].projection(r1r2Perp);

			point r2r1 = circles[other] - circles[idx];
            point r2r1Perp = r2r1.perpendiculer2D();
            point v2Hor = velocity[other].projection(r2r1);
            point v2Ver = velocity[other].projection(r2r1Perp);


            velocity[idx] = v2Hor + v1Ver;
            velocity[other] = v1Hor + v2Ver;
        }
    }
}

void handleSmallBigCircleCollition(int idx) {
	if( fabs(bigCircleRadius - smallCircleRadius  - circles[idx].abs() ) < 0.1 ) {
		if( (circles[idx]|velocity[idx]) < 0 ) return;
		point posVector = circles[idx];
		point projOnPos = - velocity[idx].projection(posVector);
		point perpProjOnPos = velocity[idx].projection(posVector.perpendiculer2D());
		velocity[idx] = projOnPos + perpProjOnPos;
	}
}

void animate(){
	angle+=0.01;

	for(int idx = 0; idx < numberOfCircle; idx++) {

        handleSmallBigCircleCollition(idx);
        if(!insideBigCircle[idx]) {
            handleInsideBigCircle(idx);
            handleSideBarCollision(idx);
        }

        else {
            handleSmallCircleCollision(idx);
			handleSmallBigCircleCollition(idx);
        }

        if(!pause) {
            circles[idx] += velocity[idx];
        }

    }

	glutPostRedisplay();
}

void initSmallCircles() {
	point init(0.02, 0.01);
	for (int idx = 0; idx < numberOfCircle; idx ++) {
		circles[idx].x = circles[idx].y = -(squareLength - smallCircleRadius);
		velocity[idx] = init.rotate(10*idx);
		insideBigCircle[idx] = false;
	}
}

void init(){
	//codes for initialization
	drawgrid = 0;
	drawaxes = 0;
	cameraHeight = 150.0;
	cameraAngle = 1.0;
	angle = 0;
	pause = false;

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
	initSmallCircles();
}

int main(int argc, char **argv){
    point p1(2, 4, 6);
    point p2(1, 0, 0);
	point p = -p1;
    printf("%lf %lf %lf", p.x, p.y, p.z);
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
