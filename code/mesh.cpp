#include "mesh.h"

void mesh::
UpdateColor(const vec3 NewCol)
{
    for(vertex& Vertex : Vertices)
    {
        Vertex.Col = NewCol;
    }
}

mesh::mesh(vec3 NewScale, vec3 NewTranslate, vec3 NewRotate)
{
    mat4 Result = Identity();
    mat4 Trans = Translate(NewTranslate);
    mat4 Scal  = Scale(NewScale);
    mat4 RotX  = RotateX(NewRotate.x);
    mat4 RotY  = RotateY(NewRotate.y);
    mat4 RotZ  = RotateZ(NewRotate.z);
    Result = Result * Scal;
    Result = Result * RotX;
    Result = Result * RotY;
    Result = Result * RotZ;
    Result = Result * Trans;
    Model = Result;
}

void mesh::SetNewTransform(vec3 NewScale, vec3 NewTranslate, vec3 NewRotate)
{
    mat4 Result = Identity();
    mat4 Trans = Translate(NewTranslate);
    mat4 Scal  = Scale(NewScale);
    mat4 RotX  = RotateX(NewRotate.x);
    mat4 RotY  = RotateY(NewRotate.y);
    mat4 RotZ  = RotateZ(NewRotate.z);
    Result = Result * Scal;
    Result = Result * RotZ;
    Result = Result * RotY;
    Result = Result * RotX;
    Result = Result * Trans;
    Model = Result;

    Position = {};
    vec3 OldPos = Position;
    vec4 PosTemp = (Model * vec4(OldPos, 1.0));
    Position = vec3(PosTemp.x, PosTemp.y, PosTemp.z);
}

void mesh::SetNewScale(vec3 NewScale)
{
    mat4 Result = Model;
    mat4 Scal  = Scale(NewScale);
    Result = Result * Scal;
    Model = Result;
}

void mesh::SetNewTranslate(vec3 NewTranslate)
{
    mat4 Result = Model;
    mat4 Trans = Translate(NewTranslate);
    Result = Result * Trans;
    Model = Result;
}

void mesh::SetNewRotate(vec3 NewRotate)
{
    mat4 Result = Model;
    mat4 RotX  = RotateX(NewRotate.x);
    mat4 RotY  = RotateY(NewRotate.y);
    mat4 RotZ  = RotateZ(NewRotate.z);
    Result = Result * RotZ;
    Result = Result * RotY;
    Result = Result * RotX;
    Model = Result;
}

aabb mesh::
GetAABB()
{
    aabb Result;
    float MinX = std::numeric_limits<float>::max(), MinY = std::numeric_limits<float>::max(), MinZ = std::numeric_limits<float>::max();
    float MaxX = std::numeric_limits<float>::min(), MaxY = std::numeric_limits<float>::min(), MaxZ = std::numeric_limits<float>::min();
    for(const vec3& Vert : Positions)
    {
        if(Vert.x < MinX) MinX = Vert.x;
        if(Vert.y < MinY) MinY = Vert.y;
        if(Vert.z < MinZ) MinZ = Vert.z;
        if(Vert.x > MaxX) MaxX = Vert.x;
        if(Vert.y > MaxY) MaxY = Vert.y;
        if(Vert.z > MaxZ) MaxZ = Vert.z;
    }
    Result.Min = {MinX, MinY, MinZ};
    Result.Max = {MaxX, MaxY, MaxZ};

    Result.Min = (Model * vec4(Result.Min, 1.0)).xyz;
    Result.Max = (Model * vec4(Result.Max, 1.0)).xyz;

    return Result;
}

std::vector<vec3> GetUnitCirclePositions(int SectorCount)
{
    float sectorStep = 2 * Pi<float> / float(SectorCount);
    float sectorAngle;

    std::vector<vec3> unitCirclePositions;
    for(int i = 0; i <= SectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        unitCirclePositions.push_back(vec3(cos(sectorAngle), 0, sin(sectorAngle)));
    }
    return unitCirclePositions;
}

void mesh::GenerateCylinder(int SectorCount, float Height, float Radius)
{
    std::vector<vec3> UnitPositions = GetUnitCirclePositions(SectorCount);

    vertex Vertex(vec3(0), vec3(0), vec3(0.24f, 0.7f, 0.36f));

    for (int i = 0; i < 2; ++i)
    {
        float TempHeight = -Height / 2.0f + i * Height;
        for(int j = 0; j <= SectorCount; j++)
        {
            vec3 NewCoord = vec3(UnitPositions[j].x * Radius, TempHeight, UnitPositions[j].z * Radius);
            vec3 NewNorm  = UnitPositions[j];
            Vertex.Pos = vec4(NewCoord, 1.0f);
            Vertex.Norm = NewNorm;
            Vertices.push_back(Vertex);
        }
    }

    int BaseCenterIndex = (int)Positions.size();
    int TopCenterIndex  = BaseCenterIndex + SectorCount + 1;

    for(int i = 0; i < 2; i++)
    {
        float TempHeight = -Height / 2.0f + i * Height;

        Vertex.Pos  = vec4(0, TempHeight, 0, 1.0);
        Vertex.Norm = vec3(0, (-1 + i * 2), 0);
        Vertices.push_back(Vertex);

        for (auto j = 0; j < SectorCount; j++)
        {
            Vertex.Pos = vec4(UnitPositions[j] * Radius) + vec4(0, TempHeight, 0, 1.0);
            Vertices.push_back(Vertex);
        }
    }

    int k1 = 0;                         // 1st vertex index at base
    int k2 = SectorCount + 1;           // 1st vertex index at top

    for(int i = 0; i < SectorCount; ++i, ++k1, ++k2)
    {
        // 2 triangles per sector
        // k1 => k1+1 => k2
        VertexIndices.push_back(k1);
        VertexIndices.push_back(k1 + 1);
        VertexIndices.push_back(k2);

        // k2 => k1+1 => k2+1
        VertexIndices.push_back(k2);
        VertexIndices.push_back(k1 + 1);
        VertexIndices.push_back(k2 + 1);
    }

    // indices for the base surface
    // NOTE: baseCenterIndex and topCenterIndices are pre-computed during vertex generation
    for(int i = 0, k = BaseCenterIndex + 1; i < SectorCount; ++i, ++k)
    {
        if(i < SectorCount - 1)
        {
            VertexIndices.push_back(BaseCenterIndex);
            VertexIndices.push_back(k + 1);
            VertexIndices.push_back(k);
        }
        else // last triangle
        {
            VertexIndices.push_back(BaseCenterIndex);
            VertexIndices.push_back(BaseCenterIndex + 1);
            VertexIndices.push_back(k);
        }
    }

    // indices for the top surface
    for(int i = 0, k = TopCenterIndex + 1; i < SectorCount; ++i, ++k)
    {
        if(i < SectorCount - 1)
        {
            VertexIndices.push_back(TopCenterIndex);
            VertexIndices.push_back(k);
            VertexIndices.push_back(k + 1);
        }
        else // last triangle
        {
            VertexIndices.push_back(TopCenterIndex);
            VertexIndices.push_back(k);
            VertexIndices.push_back(TopCenterIndex + 1);
        }
    }
}

void mesh::
LoadMesh(const std::string& Path)
{
    std::vector<vec3> Coords;
    std::vector<vec2> TextCoords;
    std::vector<vec3> Normals;

    std::vector<uint32_t> CoordIndices;
    std::vector<uint32_t> TextCoordIndices;
    std::vector<uint32_t> NormalIndices;

    std::ifstream File(Path);
    if(File.is_open())
    {
        std::string Content;
        vec3 Vertex3 = {};
        vec2 Vertex2 = {};
        while (std::getline(File, Content))
        {
            if (Content[0] == '#' || Content[0] == 'm' || Content[0] == 'o' || Content[0] == 'u' || Content[0] == 's') continue;
            if (Content.rfind("v ", 0) != std::string::npos)
            {
                sscanf(Content.c_str(), "v %f %f %f", &Vertex3[0], &Vertex3[1], &Vertex3[2]);
                Coords.push_back(Vertex3);
            }
            if (Content.rfind("vn", 0) != std::string::npos)
            {
                sscanf(Content.c_str(), "vn %f %f %f", &Vertex3[0], &Vertex3[1], &Vertex3[2]);
                Normals.push_back(Vertex3);
            }
            if (Content.rfind("vt", 0) != std::string::npos)
            {
                sscanf(Content.c_str(), "vt %f %f", &Vertex2[0], &Vertex2[1]);
                TextCoords.push_back(Vertex2);
            }
            if (Content[0] == 'f')
            {
                uint32_t Indices[3][3] = {};
                char* ToParse = const_cast<char*>(Content.c_str()) + 1;

                int Idx = 0;
                int Type = 0;
                while(*ToParse++)
                {
                    if(*ToParse != '/')
                    {
                        Indices[Type][Idx] = atoi(ToParse);
                    }

                    while ((*ToParse != ' ') && (*ToParse != '/') && (*ToParse))
                    {
                        ToParse++;
                    }

                    if(*ToParse == '/')
                    {
                        Type++;
                    }
                    if(*ToParse == ' ')
                    {
                        Type = 0;
                        Idx++;
                    }
                }

                CoordIndices.push_back(Indices[0][0]-1);
                CoordIndices.push_back(Indices[0][1]-1);
                CoordIndices.push_back(Indices[0][2]-1);
                if (Indices[1][0] != 0)
                {
                    TextCoordIndices.push_back(Indices[1][0]-1);
                    TextCoordIndices.push_back(Indices[1][1]-1);
                    TextCoordIndices.push_back(Indices[1][2]-1);
                }
                if (Indices[2][0] != 0)
                {
                    NormalIndices.push_back(Indices[2][0]-1);
                    NormalIndices.push_back(Indices[2][1]-1);
                    NormalIndices.push_back(Indices[2][2]-1);
                }
            }
        }
    }

    std::unordered_map<vertex, uint64_t> UniqueVertices;
    uint32_t IndexCount = CoordIndices.size();
    std::vector<uint32_t> Indices(IndexCount);

    for(uint32_t VertexIndex = 0;
        VertexIndex < IndexCount;
        ++VertexIndex)
    {
        vertex Vert(vec4(0), vec3(0), vec3(0.24, 0.7, 0.36));

        vec3 Pos = Coords[CoordIndices[VertexIndex]];
        Vert.Pos = vec4(Pos, 1.0);

        if(NormalIndices.size() != 0)
        {
            vec3 Norm = Normals[NormalIndices[VertexIndex]];
            Vert.Norm = Norm;
        }

        if(UniqueVertices.count(Vert) == 0)
        {
            UniqueVertices[Vert] = static_cast<uint32_t>(Vertices.size());
            Vertices.push_back(Vert);
        }

        Indices[VertexIndex] = UniqueVertices[Vert];
    }

    Positions.insert(Positions.begin(), Coords.begin(), Coords.end());
    VertexIndices.insert(VertexIndices.end(), Indices.begin(), Indices.end());
}

std::vector<polygon> mesh::
GeneratePolygons(std::vector<uint32_t> Indices)
{
    std::vector<polygon> Result(Indices.size()/3);

    vertex NewVertA = {};
    vertex NewVertB = {};
    vertex NewVertC = {};
    for(uint32_t Idx = 0;
        Idx < Indices.size() / 3;
        Idx++)
    {
        uint32_t VertIdx0 = Indices[Idx * 3 + 0];
        uint32_t VertIdx1 = Indices[Idx * 3 + 1];
        uint32_t VertIdx2 = Indices[Idx * 3 + 2];
        NewVertA = Vertices[VertIdx0];
        NewVertB = Vertices[VertIdx1];
        NewVertC = Vertices[VertIdx2];
        NewVertA.Pos = Model * NewVertA.Pos;
        NewVertB.Pos = Model * NewVertB.Pos;
        NewVertC.Pos = Model * NewVertC.Pos;
        NewVertA.Norm = Model.GetMat3() * NewVertA.Norm;
        NewVertB.Norm = Model.GetMat3() * NewVertB.Norm;
        NewVertC.Norm = Model.GetMat3() * NewVertC.Norm;
        Result[Idx].V[0] = NewVertA;
        Result[Idx].V[1] = NewVertB;
        Result[Idx].V[2] = NewVertC;
    }

    return Result;
}

std::vector<vec3> mesh::
GenerateShape(std::vector<uint32_t> Indices)
{
    std::unordered_set<vec3> Shape;
    vertex NewVertA = {};
    vertex NewVertB = {};
    vertex NewVertC = {};
    vec3 A, B, C;
    for(uint32_t Idx = 0;
        Idx < Indices.size() / 3;
        Idx++)
    {
        uint32_t VertIdx0 = Indices[Idx * 3 + 0];
        uint32_t VertIdx1 = Indices[Idx * 3 + 1];
        uint32_t VertIdx2 = Indices[Idx * 3 + 2];
        NewVertA = Vertices[VertIdx0];
        NewVertB = Vertices[VertIdx1];
        NewVertC = Vertices[VertIdx2];
        A = (Model * NewVertA.Pos).xyz;
        B = (Model * NewVertB.Pos).xyz;
        C = (Model * NewVertC.Pos).xyz;
        Shape.insert(A);
        Shape.insert(B);
        Shape.insert(C);
    }

    std::vector<vec3> Result(Shape.begin(), Shape.end());
    return Result;
}
