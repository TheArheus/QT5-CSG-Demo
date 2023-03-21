#ifndef MESH_H
#define MESH_H

#include "mat_h.hpp"

#include <unordered_map>
#include <fstream>
#include <string>

struct aabb
{
    vec3 Min;
    vec3 Max;
};

struct vertex
{
    v4<float> Pos;
    v3<float> Norm;
    v3<float> Col;

    bool operator==(const vertex& rhs) const
    {
        bool Res1 = this->Pos == rhs.Pos;
        bool Res3 = this->Norm == rhs.Norm;

        return Res1 && Res3;
    }

};

struct polygon
{
    vertex V[3];
    polygon() = default;
    polygon(vec3 A, vec3 B, vec3 C, vec3 Norm, vec3 Col)
    {
        V[0].Pos  = vec4(A, 1);
        V[0].Norm = Norm;
        V[0].Col  = Col;
        V[1].Pos  = vec4(B, 1);
        V[1].Norm = Norm;
        V[1].Col  = Col;
        V[2].Pos  = vec4(C, 1);
        V[2].Norm = Norm;
        V[2].Col  = Col;
    }

    polygon(vec3 A, vec3 B, vec3 C, vec3 Norm0, vec3 Norm1, vec3 Norm2, vec3 Col)
    {
        V[0].Pos  = vec4(A, 1);
        V[0].Norm = Norm0;
        V[0].Col  = Col;
        V[1].Pos  = vec4(B, 1);
        V[1].Norm = Norm1;
        V[1].Col  = Col;
        V[2].Pos  = vec4(C, 1);
        V[2].Norm = Norm2;
        V[2].Col  = Col;
    }

    vertex& operator[](uint32_t Idx)
    {
        return V[Idx];
    }

    vertex operator[](uint32_t Idx) const
    {
        return V[Idx];
    }
};

namespace std
{
template<>
struct hash<vertex>
{
    size_t operator()(const vertex& VertData) const
    {
        size_t res = 0;
        std::hash_combine(res, hash<decltype(VertData.Pos)>{}(VertData.Pos));
        std::hash_combine(res, hash<decltype(VertData.Norm)>{}(VertData.Norm));
        return res;
    }
};
}


class mesh
{
public:
    mesh() = default;
    mesh(vec3 NewScale, vec3 NewTranslate, vec3 NewRotate);
    void SetNewTransform(vec3 NewScale, vec3 NewTranslate, vec3 NewRotate);
    void SetNewScale(vec3 NewScale);
    void SetNewTranslate(vec3 NewTranslate);
    void SetNewRotate(vec3 NewRotate);

    void LoadMesh(const std::string& Path);
    void GenerateCylinder(int SectorCount, float Height, float Radius);
    std::vector<polygon> GeneratePolygons(std::vector<uint32_t> Indices);

    void UpdateColor(const vec3 NewCol);

    aabb GetAABB();

    std::vector<vec3> Positions;
    std::vector<unsigned int> Indices;

    std::vector<vertex> Vertices;
    std::vector<unsigned int> VertexIndices;

    vec3 Position = {};
    mat4 Model = {};
};

#endif // MESH_H
