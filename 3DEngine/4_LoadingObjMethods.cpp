#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>

using namespace std;

//3 numbers defines a point in space
struct vec3D {
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;
};

//3 points defines a triangle
struct triangle {
    vec3D p[3];
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

        vector<vec3D> verts; //locally stores the triangles
        while (!f.eof()) {
            char junk; //temporarily stores the first char of each line 
            char line[128];//buffer for line
            strstream s; //string to later fill with line char array
            f.getline(line, 128); //put line into array
            s << line; //transforms char array into string

            if (line[0] == 'v') {
                //vertice string from .obj file
                // v X Y Z format
                vec3D v;
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
    vec3D vCamera;   // Location of camera in world space
    vec3D vLookDir;	// Direction vector along the direction camera points
    float fYaw;		// FPS Camera rotation in XZ plane
    float fTheta;

    vec3D Matrix_MultiplyVector(mat4x4& m, vec3D& i) {
        vec3D v;
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

    vec3D Vector_Add(vec3D& v1, vec3D& v2) {
        return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }

    vec3D Vector_Sub(vec3D& v1, vec3D& v2) {
        return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    }

    vec3D Vector_Mul(vec3D& v1, float k) {
        return { v1.x * k, v1.y * k, v1.z * k };
    }

    vec3D Vector_Div(vec3D& v1, float k) {
        return { v1.x / k, v1.y / k, v1.z / k };
    }

    float Vector_DotProduct(vec3D& v1, vec3D& v2) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    float Vector_Length(vec3D& v) {
        return sqrtf(Vector_DotProduct(v, v)); //pythagoras, vec lenght equal to hypothenuse
    }

    vec3D Vector_Normalise(vec3D& v) {
        float l = Vector_Length(v);
        return { v.x / l, v.y / l, v.z / l };
    }

    vec3D Vector_CrossProduct(vec3D& v1, vec3D& v2) {
        vec3D v;
        v.x = v1.y * v2.z - v1.z * v2.y;
        v.y = v1.z * v2.x - v1.x * v2.z;
        v.z = v1.x * v2.y - v1.y * v2.x;
        return v;
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

          meshCube.LoadFromObjectFile("VideoShip.obj");
          //project matrix
          matProj = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

          return true;
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      bool OnUserUpdate(float fElapsedTime) override {

          //CLEAR SCREEN
          Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

          mat4x4 matTrans, matWorld, matRotX, matRotZ;

          //angle changes as function of time
          fTheta += 1.0f * fElapsedTime;

          matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
          matRotX = Matrix_MakeRotationX(fTheta);
          matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 16.0f);
          matWorld = Matrix_MakeIdentity();
          matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
          matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);


          vector<triangle> vecTrianglesToRaster; //buffers triangles so the fardest is draw first

          //operate in triangles before draw. Scale, multiply, offset and cast to 2d
          for (auto tri : meshCube.tris) {

              //creates vectors to provide information regarding normal vector information
              vec3D normal, line1, line2, vCameraRay;

              //triangle projected in screen info, triangle offset from origin info, triangle rotate in z info, triangle rotated by x after z info
              triangle triProjected, triTransformed;

              //rotate in X and Z and offset
              triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
              triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
              triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);



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
                  //create light
                  vec3D vOffsetView = { 1,1,0 };
                  vec3D light_direction = { 0.0f, 1.0f, -1.0f };
                  light_direction = Vector_Normalise(light_direction);
                  float dp = max(0.1f, Vector_DotProduct(light_direction, normal));


                  CHAR_INFO c = GetColour(dp);
                  triTransformed.col = c.Attributes;
                  triTransformed.sym = c.Char.UnicodeChar;


                  //project 3d into 2d
                  triProjected.p[0] = Matrix_MultiplyVector(matProj, triTransformed.p[0]);
                  triProjected.p[1] = Matrix_MultiplyVector(matProj, triTransformed.p[1]);
                  triProjected.p[2] = Matrix_MultiplyVector(matProj, triTransformed.p[2]);
                  triProjected.col = triTransformed.col;
                  triProjected.sym = triTransformed.sym;
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

                  vecTrianglesToRaster.push_back(triProjected);
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

              //DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
              //    triProjected.p[1].x, triProjected.p[1].y,
              //    triProjected.p[2].x, triProjected.p[2].y,
              //    PIXEL_SOLID, FG_BLACK);
          }

          return true;
      }
};

int main()
{
    OlcEngine3D demo;
    if (demo.ConstructConsole(256, 240, 2, 2))
        demo.Start();
    return 0;
}

