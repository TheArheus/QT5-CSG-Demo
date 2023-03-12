#include "openglrenderwidget.h"

void APIENTRY OpenGLRenderWidget::GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                            GLenum severity, GLsizei length,
                            const GLchar *msg, const void *data)
{
    std::string _source;
    std::string _type;
    std::string _severity;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
        _source = "API";
        break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _source = "WINDOW SYSTEM";
        break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _source = "SHADER COMPILER";
        break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        _source = "THIRD PARTY";
        break;

        case GL_DEBUG_SOURCE_APPLICATION:
        _source = "APPLICATION";
        break;

        case GL_DEBUG_SOURCE_OTHER:
        _source = "UNKNOWN";
        break;

        default:
        _source = "UNKNOWN";
        break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        _type = "ERROR";
        break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = "DEPRECATED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = "UDEFINED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_PORTABILITY:
        _type = "PORTABILITY";
        break;

        case GL_DEBUG_TYPE_PERFORMANCE:
        _type = "PERFORMANCE";
        break;

        case GL_DEBUG_TYPE_OTHER:
        _type = "OTHER";
        break;

        case GL_DEBUG_TYPE_MARKER:
        _type = "MARKER";
        break;

        default:
        _type = "UNKNOWN";
        break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        _severity = "HIGH";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM:
        _severity = "MEDIUM";
        break;

        case GL_DEBUG_SEVERITY_LOW:
        _severity = "LOW";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
        _severity = "NOTIFICATION";
        break;

        default:
        _severity = "UNKNOWN";
        break;
    }

    printf("%d: %s of %s severity, raised from %s: %s\n",
            id, _type.c_str(), _severity.c_str(), _source.c_str(), msg);
    qDebug("%d: %s of %s severity, raised from %s: %s\n",
           id, _type.c_str(), _severity.c_str(), _source.c_str(), msg);
}

enum bsp_position
{
    POLYGON_COPLANAR_WITH_PLANE,
    POLYGON_IN_FRONT_OF_PLANE,
    POLYGON_BEHIND_PLANE,
    POLYGON_STRADDLING_PLANE,

    POINT_ON_PLANE,
    POINT_IN_FRONT_OF_PLANE,
    POINT_BEHIND_PLANE,
};

vec4
GetPlaneFromPolygon(const polygon &Polygon)
{
    vec4 Plane = {};

    vec4 v0 = Polygon.V[0].Pos;
    vec4 v1 = Polygon.V[1].Pos;
    vec4 v2 = Polygon.V[2].Pos;

    vec3 A = v0.xyz;
    vec3 B = v1.xyz;
    vec3 C = v2.xyz;

    vec3 AB = B - A;
    vec3 AC = C - A;
    vec3 Normal = Cross(AB, AC).Normalize();

    Plane.xyz = Normal;
    Plane.w = Normal.Dot(A);

    return Plane;
}

uint32_t
ClassifyPointToPlane(vec3 P, vec4 Plane)
{
    vec3 Normal = Plane.xyz;
    float Dist = Normal.Dot(P) - Plane.w;
    float PlaneThickness = std::numeric_limits<float>::epsilon();

    if(Dist >  PlaneThickness)
        return POINT_IN_FRONT_OF_PLANE;
    if(Dist < -PlaneThickness)
        return POINT_BEHIND_PLANE;
    return POINT_ON_PLANE;
}

uint32_t
ClassifyPolygonToPlane(const polygon &Polygon, vec4 Plane)
{
    uint32_t NumInFront = 0, NumBehind = 0;
    for(uint32_t PointIdx = 0;
        PointIdx < 3;
        ++PointIdx)
    {
        vec4 PolygonPos = Polygon[PointIdx].Pos;
        switch(ClassifyPointToPlane(PolygonPos.xyz, Plane))
        {
            case POINT_IN_FRONT_OF_PLANE:
            {
                NumInFront++;
            } break;
            case POINT_BEHIND_PLANE:
            {
                NumBehind++;
            } break;
        }
    }

    if(NumBehind  != 0 && NumInFront != 0) return POLYGON_STRADDLING_PLANE;
    if(NumInFront != 0) return POLYGON_IN_FRONT_OF_PLANE;
    if(NumBehind  != 0) return POLYGON_BEHIND_PLANE;
    return POLYGON_COPLANAR_WITH_PLANE;
}

vec4
PickSplitingPlane(const std::vector<polygon>& Polygons)
{
    vec4 BestPlane = {};
    float BestScore = std::numeric_limits<float>::max();
    const float BlendFactor = 0.8f;

    for(uint32_t i = 0;
        i < Polygons.size();
        i+=3)
    {
        uint32_t NumInFront = 0, NumBehind = 0, NumStraddling = 0;
        vec4 Plane = GetPlaneFromPolygon(Polygons[i]);

        for(uint32_t j = 0;
            j < Polygons.size();
            j+=3)
        {
            if(i == j) continue;

            switch(ClassifyPolygonToPlane(Polygons[j], Plane))
            {
                case POLYGON_COPLANAR_WITH_PLANE: // NOTE: Coplanar with the plane
                {
                    NumInFront++;
                } break;
                case POLYGON_IN_FRONT_OF_PLANE: // NOTE: In front of the plane
                {
                    NumInFront++;
                } break;
                case POLYGON_BEHIND_PLANE: // NOTE: Behind of the plane
                {
                    NumBehind++;
                } break;
                case POLYGON_STRADDLING_PLANE: // NOTE: Straddling plane
                {
                    NumStraddling++;
                } break;
            }

            float Score = BlendFactor * NumStraddling + (1.0 - BlendFactor) * fabs(NumInFront - NumBehind);
            if(Score < BestScore)
            {
                BestScore = Score;
                BestPlane = Plane;
            }
        }
    }

    return BestPlane;
}

vec3
EdgePlaneIntersection(vec3 A, vec3 B, vec4 Plane)
{
    vec3 Result = {};
    vec3 AB = B - A;
    vec3 Normal = Plane.xyz;
    float t = (Plane.w - Normal.Dot(A)) / Normal.Dot(AB);
    if(t > 1.0) return {};
    if(t < 0.0) return {};
    Result = Lerp(A, t, AB);

    return Result;
}

void
SplitPolygon(const polygon& Poly, vec4 SplitPlane, polygon* FrontPolygon, polygon* BackPolygon)
{
    uint32_t NumFront = 0, NumBack = 0;
    std::vector<vec3> FrontVerts;
    std::vector<vec3> BackVerts;

    vec4 TempA = Poly.V[0].Pos;
    vec3 A = TempA.xyz;
    uint32_t ASide = ClassifyPointToPlane(A, SplitPlane);
    for(int VertIdx = 1;
        VertIdx < 3;
        ++VertIdx)
    {
        vec4 TempB = Poly.V[VertIdx].Pos;
        vec3 B = TempB.xyz;
        uint32_t BSide = ClassifyPointToPlane(B, SplitPlane);
        if(BSide == POINT_IN_FRONT_OF_PLANE)
        {
            if(ASide == POINT_BEHIND_PLANE)
            {
                vec3 IntersectionP = EdgePlaneIntersection(B, A, SplitPlane);
                //assert(ClassifyPointToPlane(IntersectionP, SplitPlane) == POINT_ON_PLANE);
                FrontVerts.push_back(IntersectionP);
                BackVerts.push_back(IntersectionP);
            }
            FrontVerts.push_back(B);
        }
        else if(BSide == POINT_BEHIND_PLANE)
        {
            if(ASide == POINT_IN_FRONT_OF_PLANE)
            {
                vec3 IntersectionP = EdgePlaneIntersection(A, B, SplitPlane);
                //assert(ClassifyPointToPlane(IntersectionP, SplitPlane) == POINT_ON_PLANE);
                FrontVerts.push_back(IntersectionP);
                BackVerts.push_back(IntersectionP);
            }
            else if(ASide == POINT_ON_PLANE)
            {
                BackVerts.push_back(A);
            }
            BackVerts.push_back(B);
        }
        else
        {
            FrontVerts.push_back(B);
            if(ASide == POINT_BEHIND_PLANE)
            {
                BackVerts.push_back(B);
            }
            A = B;
            ASide = BSide;
        }
    }

    for(uint32_t i = 0;
        i < FrontVerts.size();
        ++i)
    {
        FrontPolygon->V[i].Pos = vec4(FrontVerts[i], 1);
    }

    for(uint32_t i = 0;
        i < BackVerts.size();
        ++i)
    {
        BackPolygon->V[i].Pos = vec4(BackVerts[i], 1);
    }
}

std::unique_ptr<bsp_node>
BuildBSPTree(const std::vector<polygon>& Polygons, uint32_t Depth = 0)
{
    if(Polygons.size() == 0) return nullptr;

    std::unique_ptr<bsp_node> NewNode(new bsp_node);

    std::vector<polygon> Front, Back;

    vec4 SplitPlane = {}; //PickSplitingPlane(Polygons);

    for(uint32_t i = 0;
        i < Polygons.size();
        i++)
    {
        polygon FrontPart = {};
        polygon BackPart  = {};
        if(i == 0)
        {
            NewNode->Polygons.push_back(Polygons[i]);
            SplitPlane = GetPlaneFromPolygon(Polygons[i]);
            continue;
        }

        switch(ClassifyPolygonToPlane(Polygons[i], SplitPlane))
        {
            case POLYGON_COPLANAR_WITH_PLANE:
            {
                NewNode->Polygons.push_back(Polygons[i]);
            } break;
            case POLYGON_IN_FRONT_OF_PLANE:
            {
                Front.push_back(Polygons[i]);
            } break;
            case POLYGON_BEHIND_PLANE:
            {
                Back.push_back(Polygons[i]);
            } break;
            case POLYGON_STRADDLING_PLANE:
            {
                SplitPolygon(Polygons[i], SplitPlane, &FrontPart, &BackPart);
                Front.push_back(FrontPart);
                Back.push_back(BackPart);
            } break;
        }
    }

    NewNode->Plane = SplitPlane;
    NewNode->Front = BuildBSPTree(Front, Depth + 1);
    NewNode->Back  = BuildBSPTree(Back, Depth + 1);

    return NewNode;
}

void BSPInsert(std::unique_ptr<bsp_node>& Tree, const polygon& Polygon, uint32_t Depth = 0)
{
    vec4 SplitPlane = {};
    polygon FrontPart = {};
    polygon BackPart  = {};

    switch(ClassifyPolygonToPlane(Polygon, SplitPlane))
    {
        case POLYGON_COPLANAR_WITH_PLANE:
#if 0
        {
            Tree->Polygons.push_back(Polygon);
        }break;
#endif
        case POLYGON_IN_FRONT_OF_PLANE:
        {
            if(!Tree->Front)
            {
                Tree->Front = std::make_unique<bsp_node>();
                Tree->Front->Polygons.push_back(Polygon);
                Tree->Front->Plane = GetPlaneFromPolygon(Polygon);
                return;
            }
            BSPInsert(Tree->Front, Polygon, Depth + 1);
        } break;
        case POLYGON_BEHIND_PLANE:
        {
            if(!Tree->Back)
            {
                Tree->Back = std::make_unique<bsp_node>();
                Tree->Back->Polygons.push_back(Polygon);
                Tree->Back->Plane = GetPlaneFromPolygon(Polygon);
                return;
            }
            BSPInsert(Tree->Back, Polygon, Depth + 1);
        } break;
        case POLYGON_STRADDLING_PLANE:
        {
            SplitPolygon(Polygon, SplitPlane, &FrontPart, &BackPart);
            if(!Tree->Front && !Tree->Back)
            {
                Tree->Front = std::make_unique<bsp_node>();
                Tree->Front->Polygons.push_back(Polygon);
                Tree->Front->Plane = GetPlaneFromPolygon(Polygon);

                Tree->Back = std::make_unique<bsp_node>();
                Tree->Back->Polygons.push_back(Polygon);
                Tree->Back->Plane = GetPlaneFromPolygon(Polygon);

                return;
            }
            else if(!Tree->Front)
            {
                Tree->Front = std::make_unique<bsp_node>();
                Tree->Front->Polygons.push_back(Polygon);
                Tree->Front->Plane = GetPlaneFromPolygon(Polygon);
                BSPInsert(Tree->Back, BackPart, Depth + 1);

                return;
            }
            else if(!Tree->Back)
            {
                Tree->Back = std::make_unique<bsp_node>();
                Tree->Back->Polygons.push_back(Polygon);
                Tree->Back->Plane = GetPlaneFromPolygon(Polygon);

                BSPInsert(Tree->Front, FrontPart, Depth + 1);

                return;
            }
            //BSPInsert(Tree->Back, BackPart, Depth + 1);
            //BSPInsert(Tree->Front, FrontPart, Depth + 1);
        } break;
    }
}

void BSPInsert(std::unique_ptr<bsp_node>& Tree, const std::vector<polygon>& Polygons, uint32_t Depth = 0)
{
    if(Polygons.size() == 0) return;
    std::vector<polygon> Front, Back;

    for(uint32_t i = 0;
        i < Polygons.size();
        i++)
    {
        BSPInsert(Tree, Polygons[i], Depth + 1);
    }
}

bool BSPCollision(const std::unique_ptr<bsp_node>& Tree, const mesh& Mesh)
{
    bool Res;
    bool Col1 = true, Col2 = true;
    if(Tree->Front)
        Col1 = BSPCollision(Tree->Front, Mesh);
    if(Tree->Back)
        Col2 = BSPCollision(Tree->Back, Mesh);
    for(uint32_t Idx = 0;
        Idx < Mesh.Vertices.size();
        ++Idx)
    {
        vec3 Pos = (Mesh.Model * Mesh.Vertices[Idx].Pos).xyz;
        uint32_t CollisionCls = ClassifyPointToPlane(Pos, Tree->Plane);
        Res = CollisionCls == POINT_BEHIND_PLANE || CollisionCls == POINT_ON_PLANE;
        if(Res) break;
    }
    return Col1 && Col2 && Res;
}

std::unique_ptr<bsp_node> BSPTreeBoolean(const std::unique_ptr<bsp_node>& A, const std::unique_ptr<bsp_node>& B)
{
    return nullptr;
}

uint32_t BSPGetIndexCount(const std::unique_ptr<bsp_node>& Tree)
{
    uint32_t Result = Tree->Polygons.size() * 3;

    if(Tree->Front) Result += BSPGetIndexCount(Tree->Front);
    if(Tree->Back) Result  += BSPGetIndexCount(Tree->Back);

    return Result;
}

void BSPGenerateVertices(const std::unique_ptr<bsp_node>& Tree, mesh& Mesh, vec3 Color,
                         std::unordered_map<vertex, uint64_t>& UniqueVertices, std::vector<uint32_t>& Indices, uint32_t& VertexIndex)
{
    for(polygon Poly : Tree->Polygons)
    {
        for(vertex Vert : Poly.V)
        {
            vertex NewVert;
            NewVert.Pos  = Vert.Pos;
            NewVert.Norm = Tree->Plane.xyz;
            NewVert.Col  = Color;

            if(UniqueVertices.count(NewVert) == 0)
            {
                UniqueVertices[NewVert] = static_cast<uint32_t>(Mesh.Vertices.size());
                Mesh.Vertices.push_back(NewVert);
            }

            Indices[VertexIndex++] = UniqueVertices[NewVert];
        }
    }

    if(Tree->Front)
        BSPGenerateVertices(Tree->Front, Mesh, Color, UniqueVertices, Indices, VertexIndex);

    if(Tree->Back)
        BSPGenerateVertices(Tree->Back, Mesh, Color, UniqueVertices, Indices, VertexIndex);
}

void BSPGenerateVertices(const std::unique_ptr<bsp_node>& Tree, mesh& Mesh, vec3 Color)
{
    std::unordered_map<vertex, uint32_t> UniqueVertices;
    uint32_t IndexCount = BSPGetIndexCount(Tree);
    std::vector<uint32_t> Indices(IndexCount);

    std::queue<bsp_node*> Queue;
    Queue.push(Tree.get());

    uint32_t VertexIndex = 0;
    while(!Queue.empty())
    {
        bsp_node* Node = Queue.front();
        Queue.pop();
        for(polygon Poly : Node->Polygons)
        {
            vec4 PolyPlane = GetPlaneFromPolygon(Poly);
            vec3 Normal = Mesh.Model.GetMat3() * PolyPlane.xyz;
            for(vertex Vert : Poly.V)
            {
                vertex NewVert;
                NewVert.Pos  = Vert.Pos;
                NewVert.Norm = vec3(-Normal.x, -Normal.y, -Normal.z);
                NewVert.Col  = Color;

                if(UniqueVertices.count(NewVert) == 0)
                {
                    UniqueVertices[NewVert] = static_cast<uint32_t>(Mesh.Vertices.size());
                    Mesh.Vertices.push_back(NewVert);
                }

                Indices[VertexIndex++] = UniqueVertices[NewVert];
            }
        }

        if(Node->Front)
            Queue.push(Node->Front.get());

        if(Node->Back)
            Queue.push(Node->Back.get());
    }


    Mesh.VertexIndices = Indices;
}

std::string LoadShaderSource(std::string Path)
{
    std::ifstream File;
    std::stringstream Buf;

    std::string Result = "";
    File.open(Path);

    if(File.is_open())
    {
        Buf << File.rdbuf();
        Result = Buf.str();
    }

    return Result;
}

OpenGLRenderWidget::
OpenGLRenderWidget(QWidget* parent) :
    QOpenGLWidget(parent)
{
    Cube.LoadMesh("..\\assets\\cube.obj");
    Cylinder.GenerateCylinder(20, 1.0f, 0.1f);

    Cube.SetNewTransform(vec3(0.5f, 0.2f, 0.5f), vec3(2, 0, 3.5f), vec3(0));
    Cylinder.SetNewTransform(vec3(1), vec3(-0.5, 0.5f, 1.5f), vec3(0));
}

void OpenGLRenderWidget::
initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugMessageCallback, NULL);

    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // Cube
    glGenVertexArrays(1, &CubeVertexObject);
    glCreateBuffers(1, &CubeVertexBuffer);
    glCreateBuffers(1, &CubeIndexBuffer);

    glBindVertexArray(CubeVertexObject);

    glBindBuffer(GL_ARRAY_BUFFER, CubeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, Cube.Vertices.size()*sizeof(vertex), Cube.Vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CubeIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Cube.VertexIndices.size()*sizeof(unsigned int), Cube.VertexIndices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, Pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, Norm));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, Col));

    glBindVertexArray(0);

    // Cylinder
    glGenVertexArrays(1, &CylinderVertexObject);
    glCreateBuffers(1, &CylinderVertexBuffer);
    glCreateBuffers(1, &CylinderIndexBuffer);

    glBindVertexArray(CylinderVertexObject);

    glBindBuffer(GL_ARRAY_BUFFER, CylinderVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, Cylinder.Vertices.size()*sizeof(vertex), Cylinder.Vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CylinderIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Cylinder.VertexIndices.size()*sizeof(unsigned int), Cylinder.VertexIndices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, Pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, Norm));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, Col));

    glBindVertexArray(0);

    Program = glCreateProgram();

    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string VertexShaderSrc = LoadShaderSource("../assets/mesh.vert.glsl");
    const char* VertexShaderCode = VertexShaderSrc.c_str();
    glShaderSource(VertexShader, 1, &VertexShaderCode, 0);
    glCompileShader(VertexShader);

    int Success;
    char InfoLog[512] = {};
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
    if(!Success)
    {
        glGetShaderInfoLog(VertexShader, 512, NULL, InfoLog);
        std::cout << "Compile Error: " << InfoLog << std::endl;
    }
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string FragmentShaderSrc = LoadShaderSource("../assets/mesh.frag.glsl");
    const char* FragmentShaderCode = FragmentShaderSrc.c_str();
    glShaderSource(FragmentShader, 1, &FragmentShaderCode, 0);
    glCompileShader(FragmentShader);

    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
    if(!Success)
    {
        glGetShaderInfoLog(Program, 512, NULL, InfoLog);
        std::cout << "Compile Error: " << InfoLog << std::endl;
    }

    glAttachShader(Program, VertexShader);
    glAttachShader(Program, FragmentShader);
    glLinkProgram(Program);

    glGetProgramiv(Program, GL_LINK_STATUS, &Success);
    if(!Success)
    {
        glGetProgramInfoLog(Program, 512, NULL, InfoLog);
        std::cout << "Linking Error: " << InfoLog << std::endl;
    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    glClearColor(0.25, 0.25, 0.25, 1);

    QTimer* Timer = new QTimer(this);
    connect(Timer, SIGNAL(timeout()), this, SLOT(update()));
    //Timer->start(DeltaTime*1000);
    Timer->start(1000/33);
}

void OpenGLRenderWidget::
paintGL()
{
    mesh ModCube = {};
    ModCube.Model = Cube.Model;
    mesh ModCylinder = {};
    ModCylinder.Model = Cylinder.Model;
    std::unique_ptr<bsp_node> CubeTree = BuildBSPTree(Cube.GeneratePolygons(Cube.VertexIndices));
    std::unique_ptr<bsp_node> CylinderTree = BuildBSPTree(Cylinder.GeneratePolygons(Cylinder.VertexIndices));

    // NOTE: Maybe some optimizations on this check
    if(BSPCollision(CubeTree, Cylinder))
    {
        std::unique_ptr<bsp_node> ModCubeTree = BuildBSPTree(Cube.GeneratePolygons(Cube.VertexIndices));
        //BSPInsert(ModCubeTree, Cylinder.GeneratePolygons(Cylinder.VertexIndices));

        BSPGenerateVertices(CubeTree, ModCube, vec3(0.1, 0.3, 0.2));
        BSPGenerateVertices(CylinderTree, ModCylinder, vec3(0.7, 0.25, 0.35));

        glBindVertexArray(CubeVertexObject);
        glBufferData(GL_ARRAY_BUFFER, (int32_t)ModCube.Vertices.size() * sizeof(vertex), ModCube.Vertices.data(), GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int32_t)ModCube.VertexIndices.size() * sizeof(unsigned int), ModCube.VertexIndices.data(), GL_DYNAMIC_DRAW);
        glBindVertexArray(0);

        glBindVertexArray(CylinderVertexObject);
        glBufferData(GL_ARRAY_BUFFER, (int32_t)ModCylinder.Vertices.size() * sizeof(vertex), ModCylinder.Vertices.data(), GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int32_t)ModCylinder.VertexIndices.size() * sizeof(unsigned int), ModCylinder.VertexIndices.data(), GL_DYNAMIC_DRAW);
        glBindVertexArray(0);
    }
    else
    {
        BSPGenerateVertices(CubeTree, ModCube, vec3(0.7f));
        BSPGenerateVertices(CylinderTree, ModCylinder, vec3(0.7f));

        glBindVertexArray(CubeVertexObject);
        glBufferData(GL_ARRAY_BUFFER, (int32_t)ModCube.Vertices.size() * sizeof(vertex), ModCube.Vertices.data(), GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int32_t)ModCube.VertexIndices.size() * sizeof(unsigned int), ModCube.VertexIndices.data(), GL_DYNAMIC_DRAW);
        glBindVertexArray(0);

        glBindVertexArray(CylinderVertexObject);
        glBufferData(GL_ARRAY_BUFFER, (int32_t)ModCylinder.Vertices.size() * sizeof(vertex), ModCylinder.Vertices.data(), GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int32_t)ModCylinder.VertexIndices.size() * sizeof(unsigned int), ModCylinder.VertexIndices.data(), GL_DYNAMIC_DRAW);
        glBindVertexArray(0);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(Program);

    glUniform3fv(glGetUniformLocation(Program, "CamPos"), 1, (float*)&CameraPos.E);
    glUniformMatrix4fv(glGetUniformLocation(Program, "Proj"), 1, GL_TRUE, (float*)&ProjMat.E);
    glUniformMatrix4fv(glGetUniformLocation(Program, "View"), 1, GL_TRUE, (float*)&ViewMat.E);

    mat4 Ident = Identity();
    glBindVertexArray(CubeVertexObject);
    //glUniformMatrix4fv(glGetUniformLocation(Program, "Model"), 1, GL_TRUE, (float*)&Ident.E);
    glUniformMatrix4fv(glGetUniformLocation(Program, "Model"), 1, GL_TRUE, (float*)&Cube.Model.E);
    glDrawElements(GL_TRIANGLES, (int32_t)ModCube.VertexIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindVertexArray(CylinderVertexObject);
    glUniformMatrix4fv(glGetUniformLocation(Program, "Model"), 1, GL_TRUE, (float*)&Ident.E);
    glDrawElements(GL_TRIANGLES, (int32_t)Cylinder.VertexIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    A += DeltaTime;
}

void OpenGLRenderWidget::
resizeGL(int w, int h)
{
    ProjMat = Perspective(Pi<float> / 3.0f, w, h, 0.1f, 100);
    glViewport(0, 0, w, h);
}

void OpenGLRenderWidget::
MoveTo(vec3 NewPos, float V)
{
    V = V / 1000.0f;
    vec3 Dir = NewPos - Cylinder.Position;
    Dir /= Dir.Length();
    Dir *= V;

    vec3 PosToSet = Cylinder.Position + Dir * DeltaTime;
    Cylinder.SetNewTransform(vec3(1), PosToSet, vec3(0));
}
