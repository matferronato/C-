#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>

using namespace std;

//3 numbers defines a point in space
struct vec3d {
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;
};

//3 points defines a triangle
struct triangle {
    vec3d p[3];
    wchar_t sym; //symbol
    short col; //color
};

//n trisngles defines an object
struct mesh {
    vector<triangle> tris;
    bool LoadFromObjectFile(string sFilename) {
        ifstream f(sFilename);//cast file to variable
        if (!f.is_open())
            return false;

        vector<vec3d> verts; //locally stores the triangles
        while (!f.eof()) {
            char junk; //temporarily stores the first char of each line 
            char line[128];//buffer for line
            strstream s; //string to later fill with line char array
            f.getline(line, 128); //put line into array
            s << line; //transforms char array into string

            if (line[0] == 'v') {
                //vertice string from .obj file
                // v X Y Z format
                vec3d v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }
            if (line[0] == 'f') {
                //face string from .obj file
                // f line for vert1 line for vert2 line for vert3 format
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back({ verts[f[0] - 1],verts[f[1] - 1],verts[f[2] - 1] });
                //fill the mesh information from object (the shape of the object)
                //it adds to tris mesh the 3 vertex info related to each point that makes a triangle
                //every point is stored in the verts vector, and indexed using the face temporary array
                //one is subtracted from each index, because the obj file format starts counting at one
            }




        }
        return true;
    }
};

//matrix 4x4 used in multiplication 
struct mat4x4 {
    float m[4][4] = { 0 };
};

class OlcEngine3D : public olcConsoleGameEngine {
private:
    mesh meshCube;
    mat4x4 matProj;
    vec3d vCamera;   // Location of camera in world space
    vec3d vLookDir;	// Direction vector along the direction camera points
    float fYaw;		// FPS Camera rotation in XZ plane
    float fTheta;

    vec3d Matrix_MultiplyVector(mat4x4& m, vec3d& i) {
        vec3d v;
        v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
        v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
        v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
        v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
        return v;
    } //i input o output m matrix

    mat4x4 Matrix_MakeIdentity() {
        mat4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_MakeRotationX(float fAngleRad) {
        mat4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = cosf(fAngleRad);
        matrix.m[1][2] = sinf(fAngleRad);
        matrix.m[2][1] = -sinf(fAngleRad);
        matrix.m[2][2] = cosf(fAngleRad);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_MakeRotationY(float fAngleRad) {
        mat4x4 matrix;
        matrix.m[0][0] = cosf(fAngleRad);
        matrix.m[0][2] = sinf(fAngleRad);
        matrix.m[2][0] = -sinf(fAngleRad);
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = cosf(fAngleRad);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_MakeRotationZ(float fAngleRad) {
        mat4x4 matrix;
        matrix.m[0][0] = cosf(fAngleRad);
        matrix.m[0][1] = sinf(fAngleRad);
        matrix.m[1][0] = -sinf(fAngleRad);
        matrix.m[1][1] = cosf(fAngleRad);
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_MakeTranslation(float x, float y, float z) {
        mat4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        matrix.m[3][0] = x;
        matrix.m[3][1] = y;
        matrix.m[3][2] = z;
        return matrix;
    }

    //projects 3d image into 2d screen
    mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar) {
        //field of view angle, height/width, nearest and fardest distance from screen to user
        float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f); //inverse tan operation
        mat4x4 matrix;
        matrix.m[0][0] = fAspectRatio * fFovRad; //(h/w)*1/(tan(ang/2))
        matrix.m[1][1] = fFovRad; //1/(tan(ang/2)
        matrix.m[2][2] = fFar / (fFar - fNear);//fFar and fNear are z variables
        matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
        matrix.m[2][3] = 1.0f;
        matrix.m[3][3] = 0.0f;
        return matrix;
    }

    mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2) {
        mat4x4 matrix;
        for (int c = 0; c < 4; c++)
            for (int r = 0; r < 4; r++)
                matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
        return matrix;
    }

    mat4x4 Matrix_PointAt(vec3d& pos, vec3d& target, vec3d& up) {
        //create a new axis
        //pos = where you are looking, target = where you want to look 
        //calculate the new direction of the forward arrow of the axis
        vec3d newForward = Vector_Sub(target, pos);
        newForward = Vector_Normalise(newForward);
        //calculate new up arrow, dot product represents how many of the vector B is cast at A axis
        vec3d aux = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
        vec3d newUp = Vector_Sub(up, aux);
        newUp = Vector_Normalise(newUp);
        vec3d newRight = Vector_CrossProduct(newUp, newForward);

        // Construct Dimensioning and Translation Matrix	
        mat4x4 matrix;
        matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
        matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
        matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
        matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
        return matrix;
    }


    mat4x4 Matrix_QuickInverse(mat4x4& m) {
        // Only for Rotation/Translation Matrices 1/matrix_pointat
        //A0 A1 A2 A3      A0 B0 C0 00
        //B0 B1 B2 B3  -   A1 B1 C1 00
        //C0 C1 C2 C3  -   A2 B2 C2 00
        //Tx Ty Tz 01      TA TB TC 01
        mat4x4 matrix;
        matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
        matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
        matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
        matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
        matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
        matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }


    //creates  shades of grey to better iluminate objects in scene
    CHAR_INFO GetColour(float lum) {
        short bg_col, fg_col;
        wchar_t sym;
        int pixel_bw = (int)(13.0f * lum);
        switch (pixel_bw)
        {
        case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;
        case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
        case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
        case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
        case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;
        case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
        case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
        case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
        case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;
        case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
        case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
        case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
        case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
        default:
            bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
        }
        CHAR_INFO c;
        c.Attributes = bg_col | fg_col;
        c.Char.UnicodeChar = sym;
        return c;
    }

    vec3d Vector_Add(vec3d& v1, vec3d& v2) {
        return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }

    vec3d Vector_Sub(vec3d& v1, vec3d& v2) {
        return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    }

    vec3d Vector_Mul(vec3d& v1, float k) {
        return { v1.x * k, v1.y * k, v1.z * k };
    }

    vec3d Vector_Div(vec3d& v1, float k) {
        return { v1.x / k, v1.y / k, v1.z / k };
    }

    float Vector_DotProduct(vec3d& v1, vec3d& v2) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    float Vector_Length(vec3d& v) {
        return sqrtf(Vector_DotProduct(v, v)); //pythagoras, vec lenght equal to hypothenuse
    }

    vec3d Vector_Normalise(vec3d& v) {
        float l = Vector_Length(v);
        return { v.x / l, v.y / l, v.z / l };
    }

    vec3d Vector_CrossProduct(vec3d& v1, vec3d& v2) {
        vec3d v;
        v.x = v1.y * v2.z - v1.z * v2.y;
        v.y = v1.z * v2.x - v1.x * v2.z;
        v.z = v1.x * v2.y - v1.y * v2.x;
        return v;
    }


    vec3d Vector_IntersectPlane(vec3d& plane_p, vec3d& plane_n, vec3d& lineStart, vec3d& lineEnd){
        //checks and returns where line intersects plane
        //provides point on a plane plan_p and normal to the plane plan_n, 
        //as well as start and end of line
        //run of the mill algorth to figure out if line crosses boundary 
        plane_n = Vector_Normalise(plane_n);
        float plan_d = -Vector_DotProduct(plane_p, plane_n);
        float ad = Vector_DotProduct(lineStart, plane_n);
        float bd = Vector_DotProduct(lineEnd, plane_n);
        float t = (-plan_d - ad) / (bd - ad);
        vec3d lineStartToEnd = Vector_Sub(lineEnd, lineStart);
        vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
        return Vector_Add(lineStart, lineToIntersect);
    }

    int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle& in_tri, triangle& out_tri1, triangle& out_tri2) {
        //receives a point in a plane, a normal, a input triangle, and two possible output triangles
        int nInsidePointsCount = 0; //number of points inside the plane
        int nOutsidePointsCount = 0; //number of points outside the plane
        float d0, d1, d2;//distance of points

        //normalise normal axis
        plane_n = Vector_Normalise(plane_n);

        //returned signed shortest distance between points and plane
        auto dist = [&](vec3d& p) {
            vec3d n = Vector_Normalise(p);
            return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
        };

        // Create two temporary storage arrays to classify points either side of plane
        // If distance sign is positive, point lies on "inside" of plane
        vec3d* inside_points[3];
        vec3d* outside_points[3];
        d0 = dist(in_tri.p[0]);
        d1 = dist(in_tri.p[1]);
        d2 = dist(in_tri.p[2]);
        
        //if distance is positive, than point is inside camera, so add point location into array index N and increment
        if (d0 >= 0) { inside_points[nInsidePointsCount++] = &in_tri.p[0]; }
        else { outside_points[nOutsidePointsCount++] = &in_tri.p[0]; }
        if (d1 >= 0) { inside_points[nInsidePointsCount++] = &in_tri.p[1]; }
        else { outside_points[nOutsidePointsCount++] = &in_tri.p[1]; }
        if (d2 >= 0) { inside_points[nInsidePointsCount++] = &in_tri.p[2]; }
        else {outside_points[nOutsidePointsCount++] = &in_tri.p[2];}

        //classify and break triangles
        if (nInsidePointsCount == 0) {
            //all points are outside plane
            return 0;
        }
        if (nInsidePointsCount == 1 && nOutsidePointsCount == 2) {
            //since there are two points outside the plane, there will
            //be only one new triangle

            //copy appearence of the new triangle
            out_tri1.col = in_tri.col;
            out_tri1.sym = in_tri.sym;
            //copy one of the points, the inside one is a valid point
            out_tri1.p[0] = *inside_points[0];
            //the other points are located where the line crosses the plane
            out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
            out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);
            return 1;
        }
        if (nInsidePointsCount == 2 && nOutsidePointsCount == 1) {
            //since there are one points outside the plane, there will
            //be two new triangle, to compensate for the square created

            //copy appearence of the new triangle
            out_tri1.col = in_tri.col;
            out_tri1.sym = in_tri.sym;
            out_tri2.col = in_tri.col;
            out_tri2.sym = in_tri.sym;
            // The first triangle consists of the two inside points and a new
            // point determined by the location where one side of the triangle
            // intersects with the plane
            out_tri1.p[0] = *inside_points[0];
            out_tri1.p[1] = *inside_points[1];
            out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
            // The second triangle is composed of one of he inside points, a
            // new point determined by the intersection of the other side of the 
            // triangle and the plane, and the newly created point above
            out_tri2.p[0] = *inside_points[1];
            out_tri2.p[1] = out_tri1.p[2]; //using the new created point to break the square
            out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

            return 2; // Return two newly formed triangles which form a quad
        }
        if (nInsidePointsCount == 3 ) {
            //all points are inside plane, do nothing
            out_tri1 = in_tri;
            return 1; //only one of the output triangles is valid
        }
    }


    //*************************************************************************************************************************
    //*************************************************************************************************************************
    //*************************************************************************************************************************

public: OlcEngine3D() {
    m_sAppName = L"3D Demo";
}
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      bool OnUserCreate() override {
          //define cube points

          meshCube.LoadFromObjectFile("axis.obj");
          //project matrix
          matProj = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

          return true;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      bool OnUserUpdate(float fElapsedTime) override {
          mat4x4 matTrans, matWorld, matRotX, matRotZ, matCamera, matView, matCameraRot;
          vec3d vUp, vTarget, vForward;

          vForward = Vector_Mul(vLookDir, 8.0f * fElapsedTime);
          //user input
          if(GetKey(VK_UP).bHeld){
                vCamera.y += 8.0f * fElapsedTime;
          }
          if (GetKey(VK_DOWN).bHeld) {
              vCamera.y -= 8.0f * fElapsedTime;
          }
          if (GetKey(VK_RIGHT).bHeld) {
              vCamera.x += 8.0f * fElapsedTime;
          }
          if (GetKey(VK_LEFT).bHeld) {
              vCamera.x -= 8.0f * fElapsedTime;
          }
        // Standard FPS Control scheme, but turn instead of strafe
          if (GetKey(L'W').bHeld)
              vCamera = Vector_Add(vCamera, vForward);

          if (GetKey(L'S').bHeld)
              vCamera = Vector_Sub(vCamera, vForward);

          if (GetKey(L'A').bHeld)
              fYaw -= 2.0f * fElapsedTime;

          if (GetKey(L'D').bHeld)
              fYaw += 2.0f * fElapsedTime;

          //clear screen
          Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

          //set direction of view for the camera
          vUp = { 0,1,0 };
          vTarget = { 0,0,1 }; //target fixes along z axis
          matCameraRot = Matrix_MakeRotationY(fYaw); //rotated by N rad to simulate left and right turns
          vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);  //creates a new forward vector
          vTarget = Vector_Add(vCamera, vLookDir); //added to camera to create a target to view

          //create camera view matrix transformtion (matView is the inverse of matCamera)
          matCamera = Matrix_PointAt(vCamera,vTarget,vUp);
          matView = Matrix_QuickInverse(matCamera);

          //angle changes as function of time
          //fTheta += 1.0f * fElapsedTime;

          matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
          matRotX = Matrix_MakeRotationX(fTheta);
          matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 8.0f);
          matWorld = Matrix_MakeIdentity();
          matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
          matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);


          vector<triangle> vecTrianglesToRaster; //buffers triangles so the fardest is draw first

          //operate in triangles before draw. Scale, multiply, offset and cast to 2d
          for (auto tri : meshCube.tris) {
              //creates vectors to provide information regarding normal vector information
              vec3d normal, line1, line2, vCameraRay;
              //triangle projected in screen info, triangle offset from origin info, triangle rotate in z info, triangle rotated by x after z info
              triangle triProjected, triTransformed, triViewed;

              ////////////////////////////////////PIPE LINE///////////////////////////////////////////
              //++++++++++++++++++++++++++FIRST ROTATE AND OFFSET TRIANGLES++++++++++++++++++++++++++
              //rotate in X and Z and offset
              triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
              triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
              triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);



              //++++++++++++++++++++++++++CHECKS IF SHOULD BE DRAWN++++++++++++++++++++++++++++++++++
              //calculate normal vector to figureout whats should be drawn
              //get each line of the triangle
              line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
              line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
              //cross multiply to get the normal
              normal = Vector_CrossProduct(line1, line2);
              normal = Vector_Normalise(normal);

              // Get Ray from triangle to camera
              vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);
              // If ray is aligned with normal, then triangle is visible
              if (Vector_DotProduct(normal, vCameraRay) < 0.0f) {

                  //++++++++++++++++++++++++++PROVIDES ILUMINATION FOR EACH TRIANGLE++++++++++++++++++
                  vec3d vOffsetView = { 1,1,0 };
                  vec3d light_direction = { 0.0f, 1.0f, -1.0f };
                  light_direction = Vector_Normalise(light_direction);
                  float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

                  CHAR_INFO c = GetColour(dp);
                  triTransformed.col = c.Attributes;
                  triTransformed.sym = c.Char.UnicodeChar;


                  //++++++++++++++++++++++++++ACCOUNTS FOR CAMERA POSITION++++++++++++++++++++++++++
                  // convert world space into viewed space  
                  triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]); 
                  triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
                  triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
                  triViewed.col = triTransformed.col;
                  triViewed.sym = triTransformed.sym;

                  //++++++++++++++++++++++++++ClIPPING+++++++++++++++++++++++++++++++++++++++++++++
                  //avoids drawing triangles that are to close of the field of view, this technique
                  //is called clipping
                  int nClippedTriangles = 0;
                  triangle clipped[2]; //buffer to receive possible clipped triangles
                  //provides point to be edges of screen, the current running triangle, and the two buffer positions
                  nClippedTriangles = Triangle_ClipAgainstPlane({0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]); 

                  //++++++++++++++++++++++++++2D PROJECTION OF 3D IMAGE+++++++++++++++++++++++++++++
                  for (int n = 0; n < nClippedTriangles; n++) {

                      //project 3d into 2d
                      triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
                      triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
                      triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
                      triProjected.col = clipped[n].col;
                      triProjected.sym = clipped[n].sym;
                      //scale into view
                      triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
                      triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
                      triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);
                      triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
                      triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
                      triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);


                      //RESCALE STUFF
                      triProjected.p[0].x = triProjected.p[0].x * 0.5f * (float)ScreenWidth();
                      triProjected.p[0].y = triProjected.p[0].y * 0.5f * (float)ScreenHeight();

                      triProjected.p[1].x = triProjected.p[1].x * 0.5f * (float)ScreenWidth();
                      triProjected.p[1].y = triProjected.p[1].y * 0.5f * (float)ScreenHeight();

                      triProjected.p[2].x = triProjected.p[2].x * 0.5f * (float)ScreenWidth();
                      triProjected.p[2].y = triProjected.p[2].y * 0.5f * (float)ScreenHeight();
                      //store triangle
                      vecTrianglesToRaster.push_back(triProjected);
                  }
              }
          }

          //sort triangles from back to front
          sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2) {
              float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
              float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
              return z1 > z2;
              });

          for (auto& triProjected : vecTrianglesToRaster) {
              //draw and fill triangles
              FillTriangle(triProjected.p[0].x, triProjected.p[0].y,
                  triProjected.p[1].x, triProjected.p[1].y,
                  triProjected.p[2].x, triProjected.p[2].y,
                  triProjected.sym, triProjected.col);

              DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
                  triProjected.p[1].x, triProjected.p[1].y,
                  triProjected.p[2].x, triProjected.p[2].y,
                  PIXEL_SOLID, FG_BLACK);
          }

          return true;
      }
};

int main()
{
    OlcEngine3D demo;
    if (demo.ConstructConsole(256, 240, 3, 3))
        demo.Start();
    return 0;
}

