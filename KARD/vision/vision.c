//
//  vision.c
//============================================================
// INCLUDES
//============================================================
#include "vision.h"
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <ardrone_control.h>
#include <ardrone_tool/UI/ardrone_input.h>
#include <ardrone_tool/Control/ardrone_control.h>

#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <GLUT/glut.h>
    #include <OpenGL/glext.h>
#elif __linux
    #include <GL/gl.h>
    #include <GL/glut.h>
    #include <GL/glext.h>
#endif

//============================================================
// GLOBALS
//============================================================
// NI Contexts/Production Nodes
//------------------------------------------------------------
XnContext *         kvCONTEXT_PTR;
XnNodeHandle        kvUSER_NODE_HANDLE;
XnNodeHandle        kvDEPTH_NODE_HANDLE;
XnDepthMetaData *   kvDEPTH_MD_PTR;
//------------------------------------------------------------
// State Flags
//------------------------------------------------------------
XnBool              kvDRAW_BONES = TRUE;        // Bone Lines
XnBool              kvDRAW_BOUNDARIES = TRUE;   // Scene Bounds
XnBool              kvIS_FLYING = FALSE;
//------------------------------------------------------------
// Registration Points
//------------------------------------------------------------
float               kvLIMIT_HAND_UPPER = 180.0; // NEED TO SET THIS IN CALIBRATION
float               kvLIMIT_HAND_LOWER = -170.0; // SET THIS IN CALIBRATION
float               kvLIMIT_DEADZONE_UPPER_Y = 50;
float               kvLIMIT_DEADZONE_LOWER_Y = -50;
float               kvLIMIT_CENTER_Y = 0.0;
float               kvLIMIT_CENTER_X = 0.0;
float               kvLIMIT_TORSO_Y;
float               kvLIMIT_TORSO_X;
float               kvLIMIT_HEAD_UPPER_Y;
float               kvLIMIT_HEAD_LOWER_Y;
//------------------------------------------------------------
// Boundary Active Flags
//------------------------------------------------------------
float               kvHAND_ACTIVE = FALSE;
//------------------------------------------------------------
// Colors
//------------------------------------------------------------
// FLY
float               kvCOLOR_FLY_R = 173.0/255.0;
float               kvCOLOR_FLY_G = 255.0/255.0;
float               kvCOLOR_FLY_B = 47.0/255.0;
// LAND
float               kvCOLOR_LAND_R = 255.0/255.0;
float               kvCOLOR_LAND_G = 0.0/255.0;
float               kvCOLOR_LAND_B = 0.0/255.0;
// ACTIVE
float               kvCOLOR_ACTIVE_R = 255.0/255.0;
float               kvCOLOR_ACTIVE_G = 165.0/255.0;
float               kvCOLOR_ACTIVE_B = 0.0/255.0;

//============================================================
// ENUMS
//============================================================
enum KARD_WINDOW_ENUM {
    KARD_WINDOW_WIDTH   = 640,
    KARD_WINDOW_HEIGHT  = 480,
    KARD_WINDOW_X       = 300,
    KARD_WINDOW_Y       = 100
};

//============================================================
// DEFINES
//============================================================
#ifdef __APPLE__
    //#define SAMPLE_XML_PATH "SamplesConfig.xml"
    #define SAMPLE_XML_PATH "/Users/tyler/Projects/KARD/KARD/data/SamplesConfig.xml"
#elif __linux
    #define SAMPLE_XML_PATH "../data/SamplesConfig.xml"
#endif

#define CHECK_RC(rc, what)	\
    if (rc != XN_STATUS_OK) { \
    printf("%s failed: %s\n", what, xnGetStatusString(rc));	\
    return rc;	}

//------------------------------------------------------------
// KEYPRESS SIMULATIONS
//------------------------------------------------------------
// function: kvKeyPress()
// description: delegate for GLUT Keydown
void kvKeyPress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT :
            printf("VISION: KEYDOWN LEFT\n");
            break;
        case GLUT_KEY_RIGHT :
            printf("VISION: KEYDOWN RIGHT\n");
            break;
        case GLUT_KEY_UP :
            printf("VISION: KEYDOWN UP\n");
            break;
        case GLUT_KEY_DOWN :
            printf("VISION: KEYDOWN DOWN\n");
            break;
        case GLUT_KEY_F5:
            printf("VISION: TOGGLE BONES\n");
            kvDRAW_BONES = !kvDRAW_BONES;
            break;
        case GLUT_KEY_F6:
            printf("VISION: TOGGLE BOUNDARIES\n");
            kvDRAW_BOUNDARIES = !kvDRAW_BOUNDARIES;
            break;
    }
}

// function: kvKeyRelease()
// description: delegate for GLUT Keyup
void kvKeyRelease(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT :
            printf("VISION: KEYUP LEFT\n");
            break;
        case GLUT_KEY_RIGHT :
            printf("VISION: KEYUP RIGHT\n");
            break;
        case GLUT_KEY_UP :
            printf("VISION: KEYUP UP\n");
            break;
        case GLUT_KEY_DOWN :
            printf("VISION: KEYUP DOWN\n");
            break;
        default:
            // no handle
            break;
    }
}

// function: kvSendKeyDown()
// TODO: CHECK IF THIS IS NEEDED
void kvSendKeyDown(char charCode) {
    //SendVKDown((unsigned short)VkKeyScanA(charCode));
    //printf("SendKeyDown: %c\n", charCode);
    
    
}

// function: kvSendKeyUp()
// TODO: CHECK IF THIS IS NEEDED
void kvSendKeyUp(char charCode) {
    //SendVKUp((unsigned short)VkKeyScanA(charCode));
    //printf("SendKeyUp: %c\n", charCode);
    

    switch(charCode) {
        case 'f':
            //printf("TAKE-OFF\n");
            //ardrone_tool_set_ui_pad_start(1);
            //ardrone_tool_set_ui_pad_select(1);
            break;
        case 'd':
            //printf("LAND\n");
            //ardrone_tool_set_ui_pad_start(0);
            break;
        default:
            //printf("EMERGENCY\n");
            break;
    }
}

//------------------------------------------------------------
// OPENGL
//------------------------------------------------------------
// function: kvInitScene()
// description: initializes the GL Scene
void kvInitScene() {
    glEnable(GL_DEPTH_TEST);
}

// function: kvRenderScene
// description: renders the GL context
void kvRenderScene() {
    static float angle = 0.0f, deltaAngle = 0.0f;
    float handOffsetY = 0.3f;
    
    angle += deltaAngle;
    kvOrientMe(angle);
    
    // clear the GL buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // set the domain lines for user actions
    glColor3f(0.9f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    
    if(kvDRAW_BOUNDARIES) {
        //---------------------
        // DRAW SCENE BOUNDARIES
        //---------------------
        // HORIZONTAL CROSSHAIR
        glBegin(GL_LINES);
            glVertex3f(-10.0f, kvLIMIT_CENTER_Y, -1.0f);
            glVertex3f( 10.0f, kvLIMIT_CENTER_Y, -1.0f);
        glEnd();
        // VERTICAL CROSSHAIR
        glBegin(GL_LINES);
            glVertex3f( kvLIMIT_CENTER_X, -10.0f, -1.0f);
            glVertex3f( kvLIMIT_CENTER_X, 10.0f, -1.0f);
        glEnd();
        
        // set color feedback for Drone state
        if (kvHAND_ACTIVE) {
            glColor3f( kvCOLOR_ACTIVE_R, kvCOLOR_ACTIVE_G, kvCOLOR_ACTIVE_B );
        } else if (kvIS_FLYING) {
            glColor3f( kvCOLOR_FLY_R, kvCOLOR_FLY_G, kvCOLOR_FLY_B );
        } else {
            glColor3f( kvCOLOR_LAND_R, kvCOLOR_LAND_G, kvCOLOR_LAND_B);
        }
        
        //---------------------
        // HAND BOUNDARIES
        //---------------------
        // UPPER LIMIT
        glBegin(GL_LINES);
            glVertex3f(-10.0f, (-kvLIMIT_CENTER_Y + handOffsetY), -1.0f);
            glVertex3f( 10.0f, (-kvLIMIT_CENTER_Y + handOffsetY), -1.0f);
        glEnd();
        // LOWER LIMIT
        glBegin(GL_LINES);
            glVertex3f(-10.0f, (-kvLIMIT_CENTER_Y - handOffsetY), -1.0f);
            glVertex3f( 10.0f, (-kvLIMIT_CENTER_Y - handOffsetY), -1.0f);
        glEnd();
        //---------------------
        // DEAD ZONE
        //---------------------
        glColor3f( 1.0, 1.0, 1.0);
        // UPPER LIMIT
        glBegin(GL_LINES);
        glVertex3f(-10.0f, 0.1, -1.0f);
        glVertex3f( 10.0f, 0.1, -1.0f);
        glEnd();
        // LOWER LIMIT
        glBegin(GL_LINES);
        glVertex3f(-10.0f, -0.1, -1.0f);
        glVertex3f( 10.0f, -0.1, -1.0f);
        glEnd();
    }
    
    // UPDATE THE NODES
    xnWaitAndUpdateAll(kvCONTEXT_PTR);
    kvDrawStickFigure(kvUSER_NODE_HANDLE, kvDEPTH_NODE_HANDLE, kvDEPTH_MD_PTR);
    
    glutSwapBuffers();
}

// function: kvOrientMe()
// description: corrects orientation of tracked user
void kvOrientMe(float theta) {
    static float x = 0.0f, y = 0.0f, z = 0.0f;
    static float lx = 0.0f, ly = 0.0f, lz = -1.0f; lx += sin(theta);
    //lz += -cos(ang);
    
    glLoadIdentity();
    
    gluLookAt(x, y, z,
              lx, ly, lz,
              0.0f, 1.0f, 0.0f);
}

//------------------------------------------------------------
// KARD VISION INITIALIZER
//------------------------------------------------------------
// function: kvInitVision()
// description: initializes the complete vision part of KARD
XnStatus kvInitVision() {
    XnStatus nRetVal = XN_STATUS_OK;
    XnNodeHandle hScriptNode;
    XnEnumerationErrors * pErrors = NULL;
    
    XnCallbackHandle hCalib;
    XnCallbackHandle hPose;
    
    XnCallbackHandle hCallBack;
    
    const char *fn = SAMPLE_XML_PATH;
    
    nRetVal = xnInitFromXmlFileEx(fn, &kvCONTEXT_PTR, pErrors, &hScriptNode);
    CHECK_RC(nRetVal, "InitFromXml");
    
    nRetVal = xnFindExistingRefNodeByType(kvCONTEXT_PTR, XN_NODE_TYPE_DEPTH, &kvDEPTH_NODE_HANDLE);
    CHECK_RC(nRetVal, "Find depth generator");
    
    kvDEPTH_MD_PTR = xnAllocateDepthMetaData();
    
    // try to get a user
    nRetVal = xnFindExistingRefNodeByType(kvCONTEXT_PTR, XN_NODE_TYPE_USER, &kvUSER_NODE_HANDLE);
    CHECK_RC(nRetVal, "Find user generator");
    
    // register callbacks
    nRetVal = xnRegisterUserCallbacks(kvUSER_NODE_HANDLE, kvNewUser, kvLostUser, NULL, &hCallBack);
    CHECK_RC(nRetVal, "Register User Callbacks");
    
    nRetVal = xnSetSkeletonProfile(kvUSER_NODE_HANDLE, XN_SKEL_PROFILE_ALL);
    CHECK_RC(nRetVal, "Setting Skeleton Profile");
    
    xnRegisterCalibrationCallbacks(kvUSER_NODE_HANDLE, kvCalibrationStart, kvCalibrationEnd, NULL, &hCalib);
    xnRegisterToPoseCallbacks(kvUSER_NODE_HANDLE, kvPoseDetected, NULL, NULL, &hPose);
    
    xnStartGeneratingAll(kvCONTEXT_PTR);
    
	// fake init params
	int argc = 1;
	char *argv[] = { "KARD Vision", NULL };

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(KARD_WINDOW_X, KARD_WINDOW_Y);
    glutInitWindowSize(KARD_WINDOW_WIDTH, KARD_WINDOW_HEIGHT);
    
    glutCreateWindow("KARD Skeleton Tracking");
    
    // Initialize the GL scene
    kvInitScene();
    
    glutIgnoreKeyRepeat(1);
    
    // set GLUT KEYPRESS delegates
    glutSpecialFunc(kvKeyPress);
    glutSpecialUpFunc(kvKeyRelease);
    
    glutDisplayFunc((void *)kvRenderScene);
    glutIdleFunc((void *)kvRenderScene);
    
    return nRetVal;
}

// function: kvStartVision()
// description: starts the OPENGL drawing loop
void kvStartVision() {
	glutMainLoop();
}

//------------------------------------------------------------
// OPENGL DRAWING
//------------------------------------------------------------
// function: kvDrawStickPoint()
// description: draws a stick 3D point given an XnPoint3D
void kvDrawStickPoint(XnNodeHandle hDepthNode, XnPoint3D point) {
    xnConvertRealWorldToProjective(hDepthNode, 1, &point, &point);
    
    float fX = ((point.X-320)/ 320);
    float fY = -((point.Y-240) /240);
    //float fU = fX + 0.5;
    //float fV = fY + 0.5;
    
    glVertex3f(fX, fY, 0.0f);
}

// function: kvDrawJoint
// description: draws the skeleton joint
void kvDrawJoint(XnNodeHandle hUserNode, XnNodeHandle hDepthNode, XnUserID user, XnSkeletonJoint joint) {
    static int i = 0;
    
    XnSkeletonJointPosition p;
    XnPoint3D point;
    
    float X,Y;
    
    glPushMatrix();
    xnGetSkeletonJointPosition(hUserNode, user, joint, &p);
    //printf("%s failed: %d\n",xnGetStatusString(rc),joint);
    
    point=p.position;
    xnConvertRealWorldToProjective(hDepthNode, 1, &point, &point);
    //printf("%f , %f\n",point.X,point.Y);
    
    X = (point.X - 320) / 320;
    Y = -1 * (point.Y - 240) / 240;
    
    glPushMatrix();
    glTranslatef(X, Y, 0);
    
    if (joint == 1){
        glutSolidSphere(0.05f,20,20);
        //printf("head x:%f y:%f ",X,Y);
        //printf("x:%f y:%f z:%f \n",point.X,point.Y,point.Z);
        //centerY(Y);
        //deltaAngle=X;
    }
    
    glutSolidSphere(.05f,20,20);
    glPopMatrix();
    
    i++;
}

// function: kvDrawSingleUser
// description: draws a user on the GL context
void kvDrawSingleUser(XnNodeHandle hUserNode, XnNodeHandle hDepthNode, XnUserID user) {
    int count=0;
    XnSkeletonJoint joint;
    
    for (count = 1; count < 25; count++) {
        
        glColor3f( (count / 24.0f), (count / 48.0f), 1.0f);
        joint = kvGetJoint(count);
        
        if(!xnIsJointAvailable(hUserNode, joint)) continue;
        kvDrawJoint(hUserNode, hDepthNode, user, joint);
    }
}

// function: kvDrawStickLine
// description: draws a stick line on the GL context between 2 joints
void kvDrawStickLine(XnNodeHandle hUserNode, XnNodeHandle hDepthNode, XnUserID user, XnSkeletonJoint joint1, XnSkeletonJoint joint2) {
    XnSkeletonJointPosition pos1, pos2;
    xnGetSkeletonJointPosition(hUserNode, user, joint1, &pos1);
    xnGetSkeletonJointPosition(hUserNode, user, joint2, &pos2);
    
    if (pos1.fConfidence == 1 && pos2.fConfidence == 1) {
        glColor3f(1.0f,0.5f,0.3f);
    } else {
        glColor3f(0.5,0.5,0.5);
        
        if ((pos1.position.X == 0 && pos1.position.Y == 0 && pos1.position.Z == 0) ||
            (pos2.position.X == 0 && pos2.position.Y == 0 && pos2.position.Z == 0)) {
            return;
        }
    }
    
    kvDrawStickPoint(hDepthNode, pos1.position);
    kvDrawStickPoint(hDepthNode, pos2.position);
}

// function: kvDrawBones
// description: draws the skeleton of a user
void kvDrawBones(XnNodeHandle hUserNode, XnNodeHandle hDepthNode, XnUserID user) {
    // load the matrix onto the stack
    glPushMatrix();
    // set the thickness of each line
    glLineWidth(3.0);
    
    // begin the line drawing context
    glBegin(GL_LINES);
    
    // Draw each joint connection
    // The XN_SKEL_LEFT_HAND is connected to the .... XN_SKEL_LEFT_ELBOW
    // The XN_SKEL_LEFT_ELBOW is connected to the ... XN_SKEL_LEFT_SHOULDER...
    
    // draw everything above the hip
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_LEFT_HAND, XN_SKEL_LEFT_ELBOW );
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_SHOULDER);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_LEFT_SHOULDER, XN_SKEL_RIGHT_SHOULDER);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_TORSO, XN_SKEL_RIGHT_SHOULDER);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_NECK, XN_SKEL_HEAD);
    // draw the hip and torso
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP);
    // draw the legs
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
    kvDrawStickLine(hUserNode, hDepthNode, user, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);
    
    // end the drawing process
    glEnd();
    
    // remove matrix off the stack
    glPopMatrix();
}

// function: kvDrawStickFigure
// description: draws the stick figure of a user
void kvDrawStickFigure(XnNodeHandle hUserNode, XnNodeHandle hDepthNode, XnDepthMetaData * pDepthMD) {
    // store the depthMap
    xnGetDepthMetaData(hDepthNode, pDepthMD);
    //XnPoint3D corner = xnCreatePoint3D( dm.XRes(), dm.YRes(), dm.ZRes());
    XnFloat XRes, YRes, ZRes;
    
    // simple storage for using in the point function
    XRes = pDepthMD->pMap->Res.X;
    YRes = pDepthMD->pMap->Res.Y;
    ZRes = pDepthMD->nZRes;
    
    XnUserID users[10];
    XnUInt16 nUsers = 10;
    
    glPushMatrix();
    
    xnGetUsers(hUserNode, users, &nUsers);
    
    // loop through each user and draw if being tracked
    int i;
    for (i = 0; i < nUsers; ++i) {
        if (xnIsSkeletonTracking(hUserNode, users[i])) {
            kvDrawSingleUser(hUserNode, hDepthNode, users[i]);
            
            if(kvDRAW_BONES) kvDrawBones(hUserNode, hDepthNode, users[i]);
            kvHandsLocationLogic(hUserNode, hDepthNode, users[i]);
            
            /*mouse->Sendposition1(users[i],i,XN_SKEL_LEFT_SHOULDER,XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,g_DepthGenerator,g_UserGenerator);
             mouse->Sendposition1(users[i],i,XN_SKEL_RIGHT_SHOULDER,XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND,g_DepthGenerator,g_UserGenerator);
             mouse->Sendposition(users[i],i,XN_SKEL_RIGHT_HAND,"RH",g_DepthGenerator,g_UserGenerator);
             mouse->Sendposition(users[i],i,XN_SKEL_LEFT_HAND,"LH",g_DepthGenerator,g_UserGenerator);*/
        }
    }
    
    glPopMatrix();
}

// function: kvDrawBoundaries
// description: draws the boundaries around the skeleton when a pose is detected
void kvDrawBoundaries (XnNodeHandle hDepthNode, XnPoint3D refL, XnPoint3D refR, XnPoint3D Left, XnPoint3D Right) {
    // draw boundary box
    float zLDiff, zRDiff;
    
    // convert points
    xnConvertRealWorldToProjective(hDepthNode, 1, &refL, &refL);
    xnConvertRealWorldToProjective(hDepthNode, 1, &refR, &refR);
    xnConvertRealWorldToProjective(hDepthNode, 1, &Left, &Left);
    xnConvertRealWorldToProjective(hDepthNode, 1, &Right, &Right);
    
    zLDiff = refL.Z - Left.Z;
    zRDiff = refR.Z - Right.Z;
    glLineWidth(3.0);
    
    //---------------------------------------------
    // LEFT HAND
    //---------------------------------------------
    // TOP
    glBegin(GL_LINES);
    glVertex3f( (refL.X-345)/320,-1*(refL.Y-270)/240, -1.0f);
    glVertex3f( (refL.X-295)/320,-1*(refL.Y-270)/240, -1.0f);
    glEnd();
    // RIGHT
    glBegin(GL_LINES);
    glVertex3f( (refL.X-295)/320,-1*(refL.Y-210)/240, -1.0f);
    glVertex3f( (refL.X-295)/320,-1*(refL.Y-270)/240, -1.0f);
    glEnd();
    // BOTTOM
    glBegin(GL_LINES);
    glVertex3f( (refL.X-345)/320,-1*(refL.Y-200)/240, -1.0f);
    glVertex3f( (refL.X-295)/320,-1*(refL.Y-200)/240, -1.0f);
    glEnd();
    // LEFT
    glBegin(GL_LINES);
    glVertex3f( (refL.X-345)/320,-1*(refL.Y-210)/240, -1.0f);
    glVertex3f( (refL.X-345)/320,-1*(refL.Y-270)/240, -1.0f);
    glEnd();
    
    //---------------------------------------------
    // RIGHT HAND
    //---------------------------------------------
    // TOP
    glBegin(GL_LINES);
    glVertex3f( (refR.X-345)/320,-1*(refR.Y-270)/240, -1.0f);
    glVertex3f( (refR.X-295)/320,-1*(refR.Y-270)/240, -1.0f);
    glEnd();
    // RIGHT
    glBegin(GL_LINES);
    glVertex3f( (refR.X-295)/320,-1*(refR.Y-210)/240, -1.0f);
    glVertex3f( (refR.X-295)/320,-1*(refR.Y-270)/240, -1.0f);
    glEnd();
    // BOTTOM
    glBegin(GL_LINES);
    glVertex3f( (refR.X-335)/320,-1*(refR.Y-210)/240, -1.0f);
    glVertex3f( (refR.X-305)/320,-1*(refR.Y-210)/240, -1.0f);
    glEnd();
    // LEFT
    glBegin(GL_LINES);
    glVertex3f( (refR.X-345)/320,-1*(refR.Y-210)/240, -1.0f);
    glVertex3f( (refR.X-345)/320,-1*(refR.Y-270)/240, -1.0f);
    glEnd();
    
    //---------------------------------------------
    // CROSSHAIR
    //---------------------------------------------
    // SPHERE
    glPushMatrix();
    glTranslatef(0, 0.8f,0);
    glutSolidSphere(.05f,40,100);
    glPopMatrix();
    // VERTICAL
    glBegin(GL_LINES);
    glVertex3f( 0,.9, -1.0f);
    glVertex3f( 0,.7, -1.0f);
    glEnd();
}

//------------------------------------------------------------
// LOOKUPS
//------------------------------------------------------------
// function: kvGetJoint
// description: looks up an XnSkeleton joint given an integer
XnSkeletonJoint kvGetJoint(int a) {
    switch (a){
        case 1 : return XN_SKEL_HEAD;
        case 2 : return XN_SKEL_NECK;
        case 3 : return XN_SKEL_TORSO;
        case 4 : return XN_SKEL_WAIST;
        case 5 : return XN_SKEL_LEFT_COLLAR;
        case 6 : return XN_SKEL_LEFT_SHOULDER;
        case 7 : return XN_SKEL_LEFT_ELBOW;
        case 8 : return XN_SKEL_LEFT_WRIST;
        case 9 : return XN_SKEL_LEFT_HAND;
        case 10: return XN_SKEL_LEFT_FINGERTIP;
        case 11: return XN_SKEL_RIGHT_COLLAR;
        case 12: return XN_SKEL_RIGHT_SHOULDER;
        case 13: return XN_SKEL_RIGHT_ELBOW;
        case 14: return XN_SKEL_RIGHT_WRIST;
        case 15: return XN_SKEL_RIGHT_HAND;
        case 16: return XN_SKEL_RIGHT_FINGERTIP;
        case 17: return XN_SKEL_LEFT_HIP;
        case 18: return XN_SKEL_LEFT_KNEE;
        case 19: return XN_SKEL_LEFT_ANKLE;
        case 20: return XN_SKEL_LEFT_FOOT;
        case 21: return XN_SKEL_RIGHT_HIP;
        case 22: return XN_SKEL_RIGHT_KNEE;
        case 23: return XN_SKEL_RIGHT_ANKLE;
        case 24: return XN_SKEL_RIGHT_FOOT;
        default: return XN_SKEL_HEAD; // default head case
    }
}

//------------------------------------------------------------
// DELEGATES/CALLBACKS
//------------------------------------------------------------
// function: kvNewUser
// description: delegate for handling when a NewUser is detected and begins pose detection
void XN_CALLBACK_TYPE kvNewUser(XnNodeHandle hUserNode, XnUserID user, void* pCookie) {
    printf("VISION: New user identified: %d\n", user);
    xnStartPoseDetection(hUserNode, "Psi", user);
}

// function: kvLostUser
// description: delegate for handling when a User is lost
void XN_CALLBACK_TYPE kvLostUser(XnNodeHandle hUserNode, XnUserID user, void* pCookie) {
    printf("VISION: User %d lost\n", user);
}

// function: kvCalibrationStart
// description: starts calibration of the new user after detection
void XN_CALLBACK_TYPE kvCalibrationStart(XnNodeHandle hUserNode, XnUserID user, void* pCookie) {
    printf("VISION: Calibration started for user %d\n", user);
}

// function: kvCalibrationEnd
// description: checks if previous calibration was successful and begins skeleton tracking, otherwise restart
void XN_CALLBACK_TYPE kvCalibrationEnd(XnNodeHandle hUserNode, XnUserID user, XnBool bSuccess, void* pCookie) {
    printf("VISION: Calibration completed for user %d: %s\n", user, (bSuccess) ? "Success" : "Failure");
    
    // check if calibration worked otherwise restart the pose detection
    if (bSuccess) {
        // set the points
        xnStartSkeletonTracking(hUserNode, user);
    } else {
        xnStartPoseDetection(hUserNode, "Psi", user);
    }
}

// function: kvPoseDetected
// description: delegate for a registered pose detected and requests skeleton calibration and stops pose detection
void XN_CALLBACK_TYPE kvPoseDetected(XnNodeHandle hUserNode, const XnChar* pose, XnUserID user, void* pCookie) {
    printf("VISION: Pose '%s' detected for user %d\n", pose, user);
    xnRequestSkeletonCalibration(hUserNode, user, FALSE);
    xnStopPoseDetection(hUserNode, user);
}

//------------------------------------------------------------
// RUNNING EVENTS
//------------------------------------------------------------
// function: kvHandsBodyMovementLogic
// description: handles the piloting logic
void kvHandsBodyMovementLogic(XnNodeHandle hDepthNode, XnPoint3D refL, XnPoint3D refR, XnPoint3D Left, XnPoint3D Right) {
    // the differences between points x, y, z
    float xLDiff, xRDiff, zLDiff, zRDiff, yLDiff, yRDiff;
    // hand points
    XnPoint3D cL, cR, cLR, cRR;
    
    xnConvertRealWorldToProjective(hDepthNode, 1, &refL, &cLR);
    xnConvertRealWorldToProjective(hDepthNode, 1, &refR, &cRR);
    xnConvertRealWorldToProjective(hDepthNode, 1, &Left, &cL);
    xnConvertRealWorldToProjective(hDepthNode, 1, &Right, &cR);
    //printf("RL: %f\tL: %f \nRR: %f\tR: %f\n",cLR.Z,cL.Z,cRR.Z,cR.Z);
    
    xLDiff = cLR.X - cL.X;
    xRDiff = cRR.X - cR.X;
    zLDiff = cLR.Z - cL.Z;
    zRDiff = cRR.Z - cR.Z;
    yLDiff = cLR.Y - cL.Y;
    yRDiff = cRR.Y - cR.Y;
    
    // Assumptions:
    // Since we are in this state, we don't have to assume UPPER/LOWER limits of the hands
    // We only need to check against Center X/Y and the DEAD ZONE
    
    // check that we are out of the DEADZONE
    if (!(Right.Y < kvLIMIT_DEADZONE_UPPER_Y && Right.Y > kvLIMIT_DEADZONE_LOWER_Y &&
          Left.Y < kvLIMIT_DEADZONE_UPPER_Y && Left.Y > kvLIMIT_DEADZONE_LOWER_Y)) {
        if(Right.Y < kvLIMIT_DEADZONE_UPPER_Y && Right.Y > kvLIMIT_DEADZONE_LOWER_Y &&
           Left.Y > kvLIMIT_DEADZONE_UPPER_Y) {
            printf("TURN LEFT\n");
            ardrone_tool_set_progressive_cmd(1, 0, 0.1, 0, 0, 0, 0);
        } else if(Left.Y < kvLIMIT_DEADZONE_UPPER_Y && Left.Y > kvLIMIT_DEADZONE_LOWER_Y &&
                  Right.Y > kvLIMIT_DEADZONE_UPPER_Y) {
            printf("TURN RIGHT\n");
        } else if(Right.Y > kvLIMIT_CENTER_Y &&
           Left.Y > kvLIMIT_CENTER_Y) {                 // ASCEND
            printf("ASCEND\n");
        } else if(Right.Y < kvLIMIT_CENTER_Y &&
                  Left.Y < kvLIMIT_CENTER_Y) {          // DESCEND
            printf("DESCEND\n");
        } else if(Right.Y > kvLIMIT_CENTER_Y &&
                  Left.Y < kvLIMIT_CENTER_Y) {          // TILT LEFT
            printf("TILT LEFT\n");
        } else if(Right.Y < kvLIMIT_CENTER_Y &&
                  Left.Y > kvLIMIT_CENTER_Y) {          // TILT RIGHT
            printf("TILT RIGHT\n");
        }
        
        // FORWARD BACK
        //if ( Right.)
    }
    /* OLD SECONDLIFE CODE */
    /*
    if (xLDiff > 20 && xRDiff < -20) {
        kvSendKeyDown('w');
        kvSendKeyUp('w');
    } else if (xLDiff < -20 && xRDiff > 20) {
        kvSendKeyDown('s');
        kvSendKeyUp('s');
    }
    
    if (zLDiff > 100 ) {
        kvSendKeyUp('d');
        kvSendKeyDown('a');
    } else if (zRDiff > 100 ) {
        kvSendKeyUp('a');
        kvSendKeyDown('d');
    } else {
        kvSendKeyUp('a');
        kvSendKeyUp('d');
    }
    
    if (yLDiff > 20 && yRDiff > 20 ) {
        //SendVKDown(VK_PRIOR);
        //SendVKUp(VK_NEXT);
        //printf("up\n");
    } else if (yRDiff < -20 && yLDiff < -20) {
        //printf("Down\n");
        //SendVKDown(VK_NEXT);
        //SendVKUp(VK_PRIOR);
    } else{
        //SendVKUp(VK_PRIOR);
        //SendVKUp(VK_NEXT);
    }*/
}

// function: kvSetJointPoint
// description: sets XnPoint3D given the XnSkeletonJoint
void kvSetJointPoint(XnNodeHandle hUserNode, XnUserID user, XnSkeletonJoint joint, XnPoint3D * point) {
    XnSkeletonJointPosition skeletonPosition;
    xnGetSkeletonJointPosition(hUserNode, user, joint, &skeletonPosition);
    
    // set the inplace value
    *point = skeletonPosition.position;
}

// function: kvHandsLocationLogic
// description: handles the hand motions
void kvHandsLocationLogic(XnNodeHandle hUserNode, XnNodeHandle hDepthNode, XnUserID user) {
    //-----!!!!! One thing to think about is that if we move forward or back that we might want to reset the initHands values could change...
    //static int timer=0;
    //static float r = 1.0f, g = 0.0f, b = 0.0f;
    
    static XnPoint3D refLeftHand, refRightHand;
    XnPoint3D leftHandPoint, rightHandPoint, headPoint, torsoPoint;
    
    // some boundary limits for hands to cross
    kvLIMIT_HEAD_UPPER_Y = headPoint.Y + 20;
    kvLIMIT_HEAD_LOWER_Y = headPoint.Y + 10;
    kvLIMIT_TORSO_Y = torsoPoint.Y - 50;
    
    // timers
    //int frameTimer = 10;     // the time wait between a steady buffer
    
    kvSetJointPoint(hUserNode, user, XN_SKEL_LEFT_HAND, &leftHandPoint);
    kvSetJointPoint(hUserNode, user, XN_SKEL_RIGHT_HAND, &rightHandPoint);
    kvSetJointPoint(hUserNode, user, XN_SKEL_HEAD, &headPoint);
    kvSetJointPoint(hUserNode, user, XN_SKEL_TORSO, &torsoPoint);
    
    //printf("leftHandPoint.Y: %f\n", leftHandPoint.Y);
    //printf("rightHandPoint.Y: %f\n", rightHandPoint.Y);
    
    // check if hands are within application bounds
    if ( leftHandPoint.Y <= kvLIMIT_HAND_UPPER &&
         leftHandPoint.Y >= kvLIMIT_HAND_LOWER &&
         rightHandPoint.Y <= kvLIMIT_HAND_UPPER &&
        rightHandPoint.Y >= kvLIMIT_HAND_LOWER) {
        kvHAND_ACTIVE = TRUE;
        
        // provide user a little buffer time between the FLY/LAND commands
        if ( kvIS_FLYING ) {
            refLeftHand = leftHandPoint;
            refRightHand = rightHandPoint;
            
            kvHandsBodyMovementLogic(hDepthNode, refLeftHand, refRightHand, leftHandPoint, rightHandPoint);
        }
    } else {
        kvHAND_ACTIVE = FALSE;
        
        // it isn't so lets check for fly or land
        if (leftHandPoint.Y > kvLIMIT_HAND_UPPER &&
            rightHandPoint.Y > kvLIMIT_HAND_UPPER) {
            // check current state of the Drone
            if (!kvIS_FLYING) {
                // reset the check values
                //steady = FALSE;
                //timer = frameTimer;
                //initHands = FALSE;
                kvIS_FLYING = TRUE;
                
                printf("FLY\n");
                // send fly command to Drone
                ardrone_tool_set_ui_pad_start(1);
                //ardrone_tool_set_ui_pad_select(1);
            }
        } else if (leftHandPoint.Y < kvLIMIT_HAND_LOWER &&
                   rightHandPoint.Y < kvLIMIT_HAND_LOWER) {
            // check current state of the Drone and land it if it is flying
            if (kvIS_FLYING) {
                kvIS_FLYING = FALSE;
                
                printf("LAND\n");
                ardrone_tool_set_ui_pad_start(0);
                // send land command
            }
        }
    }
}