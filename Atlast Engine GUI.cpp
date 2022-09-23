#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <vector>
#include <string>
#include <strstream>
#include <fstream>

class float2
{
public:
    float v[2];
    float& operator[](const int idx)
    {
        return v[idx];
    }
};

class float3
{
public:
    float v[3];
    float& operator[](const int idx)
    {
        return v[idx];
    }
    float3 operator+(float3 &ov)
    {
        return { v[0] + ov[0], v[1] + ov[1], v[2] + ov[2]};
    }
};
class char3
{
public:
    Uint8 v[3];
    Uint8& operator[](const int idx)
    {
        return v[idx];
    }
};
void RotX(float3 &v, float rad)
{
    float3 temp = v;
    float sinfrad = sinf(rad);
    float cosfrad = cosf(rad);
    temp[1] = v[1] * cosfrad + v[2] * sinfrad;
    temp[2] = v[1] * -sinfrad + v[2] * cosfrad;
    v = temp;
}
void RotY(float3 &v, float rad)
{
    float3 temp = v;
    float sinfrad = sinf(rad);
    float cosfrad = cosf(rad);
    temp[0] = v[0] * cosfrad + v[2] * -sinfrad;
    temp[2] = v[0] * sinfrad + v[2] * cosfrad;
    v = temp;
}
void RotZ(float3 &v, float rad)
{
    float3 temp = v;
    float sinfrad = sinf(rad);
    float cosfrad = cosf(rad);
    temp[0] = v[0] * cosfrad + v[1] * sinfrad;
    temp[1] = v[0] * -sinfrad + v[1] * cosfrad;
    v = temp;
}
void Rotate(float3 &v, float3 rot)
{
    RotX(v, rot[0]);
    RotZ(v, rot[2]);
    RotY(v, rot[1]);
}
float dot(float3 v1, float3 v2)
{
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float3 cross(float3 v1, float3 v2)
{
    float3 v;
    v[0] = v1[1] * v2[2] - v1[2] * v2[1];
    v[1] = v1[2] * v2[0] - v1[0] * v2[2];
    v[2] = v1[0] * v2[1] - v1[1] * v2[0];
    return v;
}

float3 add(float3 v1, float3 v2)
{
    return { v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2] };
}

float3 sub(float3 v1, float3 v2)
{
    return { v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2] };
}
float2 sub(float2 v1, float2 v2)
{
    return { v1[0] - v2[0], v1[1] - v2[1] };
}
float3 mul(float3 v1, float3 v2)
{
    return { v1[0] * v2[0], v1[1] * v2[1], v1[2] * v2[2] };
}

float3 div(float3 v1, float3 v2)
{
    return { v1[0] / v2[0], v1[1] / v2[1], v1[2] / v2[2] };
}

float3 mul(float3 v1, float k)
{
    return { v1[0] * k, v1[1] * k, v1[2] * k };
}

float3 div(float3 v1, float k)
{
    return { v1[0] / k, v1[1] / k, v1[2] / k };
}
float3 normalize(float3 v)
{
    float l = sqrtf(dot(v, v));
    return { v[0] / l, v[1] / l, v[2] / l };
}
float3 reflect(float3 x, float3 _NORMAL)
{
    return sub(x, mul(_NORMAL, dot(_NORMAL, x) * 2.0f));
}
int sign(float v)
{
    if (v < 0)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}
void FillTriangleTop(float3 v0, float3 v1, float3 v2, int* buffer, float* dbuffer, int resX, int resY, int idx)
{
    if (v1[1] < 0) return;
    if (floorf(v1[1] - v0[1]) == 0) v1[1]++;
    if (floorf(v2[1] - v0[1]) == 0) v2[1]++;
    float invSlope1 = (v1[0] - v0[0]) / (v1[1] - v0[1]);
    float invSlope2 = (v2[0] - v0[0]) / (v2[1] - v0[1]);
    float invSlopez1 = (v1[2] - v0[2]) / (v1[1] - v0[1]);
    float invSlopez2 = (v2[2] - v0[2]) / (v2[1] - v0[1]);
    
    float x1 = v0[0];
    float x2 = v0[0];
    float z1 = v0[2];
    float z2 = v0[2];
    int step = 1;
    if (invSlope1 > invSlope2)
    {
        std::swap(invSlope1, invSlope2);
        std::swap(invSlopez1, invSlopez2);
    }
    int to = v1[1];
    int from = v0[1];
    if (to >= resY)
    {
        to = resY - 1;
    }
    if (from < 0)
    {
        int d = -v0[1];
        x1 += invSlope1 * d;
        x2 += invSlope2 * d;
        z1 += invSlopez1 * d;
        z2 += invSlopez2 * d;
        from = 0;
    }
    for (int y = from; y <= to; y++)
    {
        float invSlopez = (z2 - z1) / (x2 - x1);
        int scanX1 = x1;
        int scanX2 = std::min((int)x2, resX);
        float scanZ = z1;
        if (scanX1 < 0)
        {
            scanZ += invSlopez * -scanX1;
            scanX1 = 0;
        }
        
        int* bufAdv = buffer + y * resX + scanX1;
        float* dbufAdv = dbuffer + y * resX + scanX1;
        while (scanX1 <= scanX2)
        {
            if (*dbufAdv == 0.0f || *dbufAdv > scanZ)
            {
                *bufAdv = idx + 1;
                *dbufAdv = scanZ;
            }
            scanX1++;
            scanZ += invSlopez;
            bufAdv++;
            dbufAdv++;
        }
        x1 += invSlope1;
        x2 += invSlope2;
        z1 += invSlopez1;
        z2 += invSlopez2;
    }
}
void FillTriangleBottom(float3 v0, float3 v1, float3 v2, int* buffer, float* dbuffer, int resX, int resY, int idx)
{
    if (v2[1] < 0) return;
    if ((int)(v2[1] - v0[1]) == 0) v2[1]++;
    if ((int)(v2[1] - v1[1]) == 0) v2[1]++;
    float invSlope1 = -(v0[0] - v2[0]) / (v0[1] - v2[1]);
    float invSlope2 = -(v1[0] - v2[0]) / (v1[1] - v2[1]);
    float invSlopez1 = -(v0[2] - v2[2]) / (v0[1] - v2[1]);
    float invSlopez2 = -(v1[2] - v2[2]) / (v1[1] - v2[1]);
    float x1 = v2[0];
    float x2 = v2[0];
    float z1 = v2[2];
    float z2 = v2[2];
    int step = 1;
    if (invSlope1 > invSlope2)
    {
        std::swap(invSlope1, invSlope2);
        std::swap(invSlopez1, invSlopez2);
    }
    int to = v1[1];
    int from = v2[1];
    if (to < 0)
    {
        to = 0;
    }
    if (from >= resY)
    {
        int d = v2[1] - resY;
        x1 += invSlope1 * d;
        x2 += invSlope2 * d;
        z1 += invSlopez1 * d;
        z2 += invSlopez2 * d;
        from = resY - 1;
    }
    for (int y = from; y >= to; y--)
    {
        float invSlopez = (z2 - z1) / (x2 - x1);
        int scanX1 = x1;
        int scanX2 = std::min((int)x2, resX);
        float scanZ = z1;
        if (scanX1 < 0)
        {
            scanZ += invSlopez * -scanX1;
            scanX1 = 0;
        }

        int* bufAdv = buffer + y * resX + scanX1;
        float* dbufAdv = dbuffer + y * resX + scanX1;
        while (scanX1 <= scanX2)
        {
            if (*dbufAdv == 0.0f || *dbufAdv > scanZ)
            {
                *bufAdv = idx + 1;
                *dbufAdv = scanZ;
            }
            scanX1++;
            scanZ += invSlopez;
            bufAdv++;
            dbufAdv++;
        }
        x1 += invSlope1;
        x2 += invSlope2;
        z1 += invSlopez1;
        z2 += invSlopez2;
    }
}
void FillTriangle(float3 v0, float3 v1, float3 v2, int* buffer, float* dbuffer, int resX, int resY, int idx)
{
    if (v2[1] < v0[1])
    {
        std::swap(v2, v0);
    }
    if (v1[1] < v0[1])
    {
        std::swap(v1, v0);
    }
    if (v2[1] < v1[1])
    {
        std::swap(v2, v1);
    }
    FillTriangleTop(v0, v1, v2, buffer, dbuffer, resX, resY, idx);
    FillTriangleBottom(v0, v1, v2, buffer, dbuffer, resX, resY, idx);

}

struct Triangle3D
{
    float3 verts[3];
};
class Object
{
public:
    std::string name;
    bool winOpen = false;
    float3 pos;
    float3 rot;
    std::string meshDirectory;
    std::string texDirectory;
    float3 color;
    std::vector<Triangle3D*> mesh;
    void loadMesh(std::string filename)
    {
        std::ifstream f(filename);

        std::vector<float3> verts;
        std::vector<float2> uv;
        while (!f.eof())
        {
            char line[128];
            f.getline(line, 128);

            std::strstream s;
            s << line;

            char junk;

            if (line[0] == 'v' && line[1] == ' ')
            {
                float3 v;
                s >> junk >> v[0] >> v[1] >> v[2];
                verts.push_back(v);
            }
            if (line[0] == 'f')
            {
                int f2;
                int f3;
                int f4;
                Triangle3D *tri = new Triangle3D;
                s >> junk >> f2 >> f3 >> f4;
                tri->verts[0] = verts[f2 - 1];
                tri->verts[1] = verts[f3 - 1];
                tri->verts[2] = verts[f4 - 1];
                mesh.push_back(tri);
            }
        }
    }
};
float3 Vector_IntersectPlane(float3& plane_p, float3& plane_n, float3& lineStart, float3& lineEnd, float& t)
{
    plane_n = normalize(plane_n);
    float plane_d = -dot(plane_n, plane_p);
    float ad = dot(lineStart, plane_n);
    float bd = dot(lineEnd, plane_n);
    t = (-plane_d - ad) / (bd - ad);
    float3 lineStartToEnd = sub(lineEnd, lineStart);
    float3 lineToIntersect = mul(lineStartToEnd, t);
    return add(lineStart, lineToIntersect);
}

int Triangle_ClipAgainstPlane(float3 plane_p, float3 plane_n, Triangle3D& in_tri, Triangle3D& out_tri1, Triangle3D& out_tri2)
{
    plane_n = normalize(plane_n);

    auto dist = [&](float3& p)
    {
        float3 n = normalize(p);
        return (plane_n[0] * p[0] + plane_n[1] * p[1] + plane_n[2] * p[2] - dot(plane_n, plane_p));
    };
    float3* inside_points[3];  int nInsidePointCount = 0;
    float3* outside_points[3]; int nOutsidePointCount = 0;
    float d0 = dist(in_tri.verts[0]);
    float d1 = dist(in_tri.verts[1]);
    float d2 = dist(in_tri.verts[2]);
    if (d0 >= 0)
    {
        inside_points[nInsidePointCount++] = &in_tri.verts[0];
    }
    else
    {
        outside_points[nOutsidePointCount++] = &in_tri.verts[0];
    }
    if (d1 >= 0)
    {
        inside_points[nInsidePointCount++] = &in_tri.verts[1];
    }
    else
    {
        outside_points[nOutsidePointCount++] = &in_tri.verts[1];
    }
    if (d2 >= 0)
    {
        inside_points[nInsidePointCount++] = &in_tri.verts[2];
    }
    else
    {
        outside_points[nOutsidePointCount++] = &in_tri.verts[2];
    }
    if (nInsidePointCount == 0) return 0;

    if (nInsidePointCount == 3)
    {
        out_tri1 = in_tri;
        return 1;
    }

    float u, v;

    if (nInsidePointCount == 1 && nOutsidePointCount == 2)
    {
        out_tri1.verts[0] = *inside_points[0];
        out_tri1.verts[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], u);
        out_tri1.verts[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], v);

        return 1;
    }

    if (nInsidePointCount == 2 && nOutsidePointCount == 1)
    {
        out_tri1.verts[0] = *inside_points[0];
        out_tri1.verts[1] = *inside_points[1];
        out_tri1.verts[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], u);
        out_tri2.verts[0] = *inside_points[1];
        out_tri2.verts[1] = out_tri1.verts[2];
        out_tri2.verts[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], v);

        return 2;
    }
}
class Scene
{
public:
    std::vector<Object*> data;

};
void FragmentShader(std::vector<Object*> &objects, float* dbuffer, int* buffer, Uint8* outBuffer, int resX, int resY)
{
    int* bufferAdv = buffer;
    float* dbufferAdv = dbuffer;
    Uint8* outBufferAdv = outBuffer;
    for (int y = 0; y < resY; y++)
    {
        for (int x = 0; x < resX; x++)
        {
            int idx = *bufferAdv;
            float depth = *dbufferAdv;
            if (idx != 0)
            {
                idx--;
                Object *obj = objects[idx];
                outBufferAdv[0] = (Uint8)std::min(std::max(255.0f / depth, 0.0f), 255.0f);
                outBufferAdv[1] = (Uint8)std::min(std::max(255.0f / depth, 0.0f), 255.0f);;
                outBufferAdv[2] = (Uint8)std::min(std::max(255.0f / depth, 0.0f), 255.0f);;

            }
            bufferAdv++;
            dbufferAdv++;
            outBufferAdv += 3;
        }
    }
}
void windowUpdate(Uint8* buffer, SDL_Renderer *renderer, int resX, int resY)
{
    
    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom((void*)buffer, resX, resY, 24, resX * 3, 0xFF0000, 0x00FF00, 0x0000FF, 0x000000);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_RenderCopy(renderer, tex, NULL, NULL);
    SDL_DestroyTexture(tex);
}
// Main code
int main(int, char**)
{
    Scene scene;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    // Setup SDL_Renderer instance
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return false;
    }
    //SDL_RendererInfo info;
    //SDL_GetRendererInfo(renderer, &info);
    //SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;

    int* drawBuffer = (int*)malloc(sizeof(int) * 1280 * 720);
    float* depthBuffer = (float*)malloc(sizeof(float) * 1280 * 720);
    Uint8* outBuffer = (Uint8*)malloc(1280 * 720 * 3);

    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Scene Manager");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("Objects");               // Display some text (you can use a format strings too)
            for (Object* obj : scene.data)
            {
                ImGui::Checkbox(obj->name.c_str(), &obj->winOpen);
            }
            if (ImGui::Button("Create New Object"))
            {
                Object *obj = new Object{ std::string("Object") + std::to_string(scene.data.size()) };
                scene.data.push_back(obj);
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        int i = -1;
        for (Object* obj : scene.data)
        {
            i++;
            if (obj->winOpen)
            {
                ImGui::Begin((std::string("Object ") + std::to_string(i)).c_str());
                ImGui::Text("Info");
                ImGui::InputText("Name", &obj->name);
                ImGui::Text("Transform");
                ImGui::DragFloat3("Position", obj->pos.v, 0.001f);
                ImGui::DragFloat3("Rotation", obj->rot.v, 0.001f);

                ImGui::Text("Mesh");
                ImGui::InputText("Mesh Directory", &obj->meshDirectory);
                if (ImGui::Button("Load Mesh"))
                {
                    obj->loadMesh(obj->meshDirectory);
                }

                ImGui::Text("Colors and Textures");
                ImGui::ColorEdit3("Base Color", obj->color.v);

                ImGui::InputText("Texture Directory", &obj->texDirectory);
                if (ImGui::Button("Load Texture"))
                {

                }

                ImGui::End();
            }
        }

        // Rendering
        ImGui::Render();
        SDL_RenderClear(renderer);
        memset(drawBuffer, 0, sizeof(int) * 1280 * 720);
        memset(depthBuffer, 0, sizeof(float) * 1280 * 720);
        memset(outBuffer, 0, 1280 * 720 * 3);
        for (int i = 0; i < scene.data.size(); i++)
        {
            for (Triangle3D *tri : scene.data[i]->mesh)
            {
                Triangle3D drTri = *tri;
                Rotate(drTri.verts[0], scene.data[i]->rot);
                Rotate(drTri.verts[1], scene.data[i]->rot);
                Rotate(drTri.verts[2], scene.data[i]->rot);
                drTri.verts[0] = drTri.verts[0] + scene.data[i]->pos;
                drTri.verts[1] = drTri.verts[1] + scene.data[i]->pos;
                drTri.verts[2] = drTri.verts[2] + scene.data[i]->pos;

                Triangle3D newTris[2];
                int n = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, drTri, newTris[0], newTris[1]);
                for (int j = 0; j < n; j++)
                {
                    drTri = newTris[j];
                    drTri.verts[0][0] /= drTri.verts[0][2];
                    drTri.verts[0][1] /= drTri.verts[0][2];
                    drTri.verts[1][0] /= drTri.verts[1][2];
                    drTri.verts[1][1] /= drTri.verts[1][2];
                    drTri.verts[2][0] /= drTri.verts[2][2];
                    drTri.verts[2][1] /= drTri.verts[2][2];
                    drTri.verts[0][0] += 1;
                    drTri.verts[0][1] += 1;
                    drTri.verts[1][0] += 1;
                    drTri.verts[1][1] += 1;
                    drTri.verts[2][0] += 1;
                    drTri.verts[2][1] += 1;
                    drTri.verts[0][0] *= 1280 >> 1;
                    drTri.verts[0][1] *= 720 >> 1;
                    drTri.verts[1][0] *= 1280 >> 1;
                    drTri.verts[1][1] *= 720 >> 1;
                    drTri.verts[2][0] *= 1280 >> 1;
                    drTri.verts[2][1] *= 720 >> 1;
                    drTri.verts[0][0] = roundf(drTri.verts[0][0]);
                    drTri.verts[0][1] = roundf(drTri.verts[0][1]);
                    drTri.verts[1][0] = roundf(drTri.verts[1][0]);
                    drTri.verts[1][1] = roundf(drTri.verts[1][1]);
                    drTri.verts[2][0] = roundf(drTri.verts[2][0]);
                    drTri.verts[2][1] = roundf(drTri.verts[2][1]);
                    FillTriangle(drTri.verts[0], drTri.verts[1], drTri.verts[2], drawBuffer, depthBuffer, 1280, 720, 0);
                }
            }
        }
        FragmentShader(scene.data, depthBuffer, drawBuffer, outBuffer, 1280, 720);
        windowUpdate(outBuffer, renderer, 1280, 720);
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
