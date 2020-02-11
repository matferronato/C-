#include "olcConsoleGameEngine.h"
using namespace std;

//3 numbers defines a point in space
struct vec3D {
    float x, y, z;
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
};

//matrix 4x4 used in multiplication 
struct mat4x4 {
    float m[4][4] = { 0 };
};

class OlcEngine3D : public olcConsoleGameEngine {
private: 
    mesh meshCube;
    mat4x4 matProj;
    vec3D vCamera;
    float fTheta;

    void MultiplyMatrixVector(vec3D &i, vec3D &o, mat4x4 &m) {
        o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
        o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
        o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
        float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];
        //checks for division by zero
        if (w != 0.0f) {
            o.x /= w;
            o.y /= w;
            o.z /= w;
        }
    } //i input o output m matrix

    //creates  shades of grey to better iluminate objects in scene
    CHAR_INFO GetColour(float lum){
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

public: OlcEngine3D() {
         m_sAppName = L"3D Demo";
      }

      bool OnUserCreate() override { 
          //define cube points
          meshCube.tris = {
              //south
              {0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f},
              {0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f},

              //east
              {1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f},
              {1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f},

              //north
              {1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f},
              {1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f},

              //west
              {0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f},
              {0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f},

              //top
              {0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f},
              {0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 0.0f},

              //bottom
              {1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f},
              {1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f},

          };

          //projection matrix, defines data to fill mult matrix
          float fNear = 0.1f; //nearest distance from screen to user
          float fFar = 1000.0f; //fardest distance from screen to user
          float fFov = 90.0f; //field of view, angle
          float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
          float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f); //inverse tan operation

          matProj.m[0][0] = fAspectRatio * fFovRad; //(h/w)*1/(tan(ang/2))
          matProj.m[1][1] = fFovRad; //1/(tan(ang/2)
          matProj.m[2][2] = fFar / (fFar - fNear); //fFar and fNear are z variables
          matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
          matProj.m[2][3] = 1.0f;
          matProj.m[3][3] = 0.0f;

          return true; }


      bool OnUserUpdate(float fElapsedTime) override {

          //CLEAR SCREEN
          Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

          mat4x4 matRotZ, matRotX;
          
          //angle changes as function of time
          fTheta += 1.0f * fElapsedTime;

          //ROTATE Z
          matRotZ.m[0][0] = cosf(fTheta);
          matRotZ.m[0][1] = sinf(fTheta);
          matRotZ.m[1][0] = -sinf(fTheta);
          matRotZ.m[1][1] = cosf(fTheta);
          matRotZ.m[2][2] = 1;
          matRotZ.m[3][3] = 1;

          //ROTATE X
          matRotX.m[0][0] = 1;
          matRotX.m[1][1] = cosf(fTheta * 0.5f);
          matRotX.m[1][2] = sinf(fTheta * 0.5f);
          matRotX.m[2][1] = -sinf(fTheta * 0.5f);
          matRotX.m[2][2] = cosf(fTheta * 0.5f);
          matRotX.m[3][3] = 1;


          //DRAW TRIANGLES
          for (auto tri : meshCube.tris) {
              
              //creates vectors to provide information regarding normal vector information
              vec3D normal, line1, line2;
              
              //triangle projected in screen info, triangle offset from origin info, triangle rotate in z info, triangle rotated by x after z info
              triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

              //rotate Z
              MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
              MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
              MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

              //rotate X after Z
              MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
              MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
              MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

              //translate original triangle in the z axis far from screen (offset from origin);
              triTranslated = triRotatedZX;
              triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
              triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
              triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

              //calculate normal vector to figureout whats should be drawn
              line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
              line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
              line1.z = triTranslated.p[1].z - triTranslated.p[0].z;
              line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
              line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
              line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

              normal.x = line1.y * line2.z - line1.z * line2.y;
              normal.y = line1.z * line2.x - line1.x * line2.z;
              normal.z = line1.x * line2.y - line1.y * line2.x;

              //normalize normal using pythagoras
              float l = sqrt(normal.x* normal.x + normal.y* normal.y + normal.z*normal.z); //lenght
              normal.x /= l; normal.y /= l; normal.z /= l;


              //only draws triangles that are visible from the camera perspective
              //it knows which triangles to draw by calculating the dot product, function that
              //checks the similarity between 2 vectors. dot product = Ax*Bx + Ay*By + Az*Bz
              if(normal.x * (triTranslated.p[0].x - vCamera.x) + 
                 normal.y * (triTranslated.p[0].y - vCamera.y) +
                 normal.z * (triTranslated.p[0].z - vCamera.z) < 0){

                  //cast a light to a triangle and normalize it
                  vec3D light_direction = { 0.0f, 0.0f, -1.0f };
                  float l = sqrt(light_direction.x * light_direction.x + light_direction.y * light_direction.y + light_direction.z * light_direction.z); //lenght
                  light_direction.x /= l; light_direction.y /= l; light_direction.z /= l;
                  
                  //checks similarity between light and dranw triangle using dotproduct
                  float dp = normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z;
                  
                  CHAR_INFO c = GetColour(dp);
                  triTranslated.col = c.Attributes;
                  triTranslated.sym = c.Char.UnicodeChar;


                  //project 3d into 2d
                  MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
                  MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
                  MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);
                  triProjected.col = triTranslated.col;
                  triProjected.sym = triTranslated.sym;


                  //RESCALE STUFF
                  triProjected.p[0].x = triProjected.p[0].x + 1.0f;
                  triProjected.p[0].y = triProjected.p[0].y + 1.0f;
                  triProjected.p[0].x = triProjected.p[0].x * 0.5f * (float)ScreenWidth();
                  triProjected.p[0].y = triProjected.p[0].y * 0.5f * (float)ScreenHeight();

                  triProjected.p[1].x = triProjected.p[1].x + 1.0f;
                  triProjected.p[1].y = triProjected.p[1].y + 1.0f;
                  triProjected.p[1].x = triProjected.p[1].x * 0.5f * (float)ScreenWidth();
                  triProjected.p[1].y = triProjected.p[1].y * 0.5f * (float)ScreenHeight();

                  triProjected.p[2].x = triProjected.p[2].x + 1.0f;
                  triProjected.p[2].y = triProjected.p[2].y + 1.0f;
                  triProjected.p[2].x = triProjected.p[2].x * 0.5f * (float)ScreenWidth();
                  triProjected.p[2].y = triProjected.p[2].y * 0.5f * (float)ScreenHeight();

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
          }

          
          return true; }
};

int main()
{
    OlcEngine3D demo;
    if(demo.ConstructConsole(256, 240, 2, 2))
        demo.Start();
    return 0;
}

