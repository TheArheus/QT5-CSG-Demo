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
