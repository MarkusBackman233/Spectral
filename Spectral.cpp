#include "Spectral.h"
#include <list>
#include <thread>
#include <future>

Spectral::Spectral(SDL_Renderer* renderer, SDL_Window* window, SDL_Event& event) 
    : m_viewOffset(1, 1, 0)
    , m_hasClicked(false)
{
    m_renderer = renderer;
    
    m_ObjectHandler = new ObjectHandler();

    m_directionLighting = Math::Vector3(1.0f, 10.0f, 5.0f);

    m_directionLighting = m_directionLighting.GetNormal();

    m_depthBuffer = new float[SCREEN_WIDTH * SCREEN_HEIGHT];

    GameObject* terrain= new GameObject();
    terrain->SetName("terrain");
    terrain->SetMesh("C:/Users/Markus/Desktop/Game/Level/LevelTerrain.obj");
    terrain->SetTexture("C:/Users/Markus/Desktop/Game/clay_snow1.jpg");
    terrain->SetNormal("C:/Users/Markus/Desktop/Game/clay_snow1_normal.png");
    terrain->CreateActor(GameObject::physicsType::dynamicActor, true);
    terrain->CreateShape(GameObject::physicsShape::triangleMesh);
    m_ObjectHandler->Instantiate(*terrain);       
    
    GameObject* trees= new GameObject();
    trees->SetName("trees");
    trees->SetMesh("C:/Users/Markus/Desktop/Game/Level/LevelTrees.obj");
    trees->SetTexture("C:/Users/Markus/Desktop/Game/trees/Dead_Tree_Bark1.png");
    trees->SetNormal("C:/Users/Markus/Desktop/Game/trees/Dead_Tree_Bark_Normal.png");
    trees->CreateActor(GameObject::physicsType::dynamicActor, true);
    trees->CreateShape(GameObject::physicsShape::triangleMesh);
    m_ObjectHandler->Instantiate(*trees);    
    
    GameObject* roads = new GameObject();
    roads->SetName("roads");
    roads->SetMesh("C:/Users/Markus/Desktop/Game/Level/LevelRoads.obj");
    roads->SetTexture("C:/Users/Markus/Desktop/Game/road_with_roadside_and_lanes_24_04_diffuse.png");
    roads->SetNormal("C:/Users/Markus/Desktop/Game/trees/Dead_Tree_Bark_Normal.png");
    roads->CreateActor(GameObject::physicsType::dynamicActor, true);
    roads->CreateShape(GameObject::physicsShape::triangleMesh);
    m_ObjectHandler->Instantiate(*roads);    
    
    GameObject* concrete = new GameObject();
    concrete->SetName("concrete");
    concrete->SetMesh("C:/Users/Markus/Desktop/Game/Level/LevelConcrete.obj");
    concrete->SetTexture("C:/Users/Markus/Desktop/Game/ooba.jpg");
    concrete->SetNormal("C:/Users/Markus/Desktop/Game/trees/Dead_Tree_Bark_Normal.png");
    concrete->CreateActor(GameObject::physicsType::dynamicActor, true);
    concrete->CreateShape(GameObject::physicsShape::triangleMesh);
    m_ObjectHandler->Instantiate(*concrete);    
    
    
    GameObject* cars = new GameObject();
    cars->SetName("cars");
    cars->SetMesh("C:/Users/Markus/Desktop/Game/Level/LevelCars.obj");
    cars->SetTexture("C:/Users/Markus/Desktop/Game/Level/van_03.png");
    cars->SetNormal("C:/Users/Markus/Desktop/Game/trees/Dead_Tree_Bark_Normal.png");
    cars->CreateActor(GameObject::physicsType::dynamicActor, true);
    cars->CreateShape(GameObject::physicsShape::triangleMesh);
    m_ObjectHandler->Instantiate(*cars);
    
    GameObject* box= new GameObject(Math::Vector3(10.0f,3.6f,-2.0f));
    box->SetName("Box");
    box->SetMesh("C:/Users/Markus/Desktop/Game/Cube.obj");
    box->SetTexture("C:/Users/Markus/Desktop/Game/Box_texture.jpg");
    box->SetNormal("C:/Users/Markus/Desktop/Game/Box_Normal.png");    
    box->CreateActor(GameObject::physicsType::dynamicActor, false);
    box->CreateShape(GameObject::physicsShape::box);
    m_ObjectHandler->Instantiate(*box);    
    //
    //
    //GameObject* tree2= new GameObject(Math::Vector3(5.0f,1.05f,3.0f));
    //tree2->SetName("DeadTree01");
    //tree2->SetMesh("C:/Users/Markus/Desktop/Game/Cube.obj");
    //tree2->SetTexture("C:/Users/Markus/Desktop/Game/trees/Dead_Tree_Bark1.png");
    //tree2->SetNormal("C:/Users/Markus/Desktop/Game/trees/Dead_Tree_Bark_Normal.png");    
    //m_ObjectHandler->Instantiate(*tree2);      
    //
    //GameObject* tree3= new GameObject(Math::Vector3(20.0f,50.05f,-70.0f));
    //tree3->SetName("DeadTree01");
    //tree3->SetMesh("C:/Users/Markus/Desktop/Game/Cube.obj");
    //tree3->SetTexture("C:/Users/Markus/Desktop/Game/trees/Dead_Tree_Bark1.png");
    //tree3->SetNormal("C:/Users/Markus/Desktop/Game/trees/Dead_Tree_Bark_Normal.png");    
    //m_ObjectHandler->Instantiate(*tree3);    
    
    m_player = new Player(*terrain->GetMesh(), window, event);

	float nearPlane = 0.1f;
	float farPlane = 1000.0f;  
    
	float fieldOfView = 90.0f;
	float aspectRatio = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH;
        
    
	float fovRad = 1.0f / tanf(fieldOfView * 0.5f / 180.0f * 3.14159f);

    m_projectionMatrix.m_matrix[0][0] = aspectRatio * fovRad;
    m_projectionMatrix.m_matrix[1][1] = fovRad;
    m_projectionMatrix.m_matrix[2][2] = farPlane / (farPlane - nearPlane);
    m_projectionMatrix.m_matrix[3][2] = (-farPlane * nearPlane) / (farPlane - nearPlane);
    m_projectionMatrix.m_matrix[2][3] = 1.0f;
    m_projectionMatrix.m_matrix[3][3] = 0.0f;

    float invAspectRatioFov = 1.0f / (aspectRatio * fovRad);
    float invFov = 1.0f / fovRad;
    float invNear = 1.0f / nearPlane;
    float invFar = 1.0f / farPlane;

    m_invProjectionMatrix.m_matrix[0][0] = invAspectRatioFov;
    m_invProjectionMatrix.m_matrix[0][1] = 0.0f;
    m_invProjectionMatrix.m_matrix[0][2] = 0.0f;
    m_invProjectionMatrix.m_matrix[0][3] = 0.0f;

    m_invProjectionMatrix.m_matrix[1][0] = 0.0f;
    m_invProjectionMatrix.m_matrix[1][1] = invFov;
    m_invProjectionMatrix.m_matrix[1][2] = 0.0f;
    m_invProjectionMatrix.m_matrix[1][3] = 0.0f;

    m_invProjectionMatrix.m_matrix[2][0] = 0.0f;
    m_invProjectionMatrix.m_matrix[2][1] = 0.0f;
    m_invProjectionMatrix.m_matrix[2][2] = 0.0f;
    m_invProjectionMatrix.m_matrix[2][3] = (nearPlane - farPlane) / (farPlane * nearPlane);

    m_invProjectionMatrix.m_matrix[3][0] = 0.0f;
    m_invProjectionMatrix.m_matrix[3][1] = 0.0f;
    m_invProjectionMatrix.m_matrix[3][2] = -invFar;
    m_invProjectionMatrix.m_matrix[3][3] = 1.0f / (farPlane / (farPlane - nearPlane));






    


    for (int i = 0; i < 3; i++)
    {
        Light light;
        light.position = Math::Vector3(-(rand() % 50 + 1), (rand() % 30 + 1), (rand() % 50 + 1));
        light.color = Math::Vector3((rand() % 255 + 1), (rand() % 255 + 1), (rand() % 255 + 1));
        m_ligts.push_back(light);
    }





}


void Spectral::Update()
{

    PhysXManager& physXManager = PhysXManager::GetInstance();
    physXManager.TickSimulation(m_player->getDeltaTime());

    for (const auto& object : m_ObjectHandler->GetGameObjects())
    {
        if (object->m_isKinematic == false)
        {
            object->UpdatePhysicsPosition();
        }
    }

    m_player->Update();
    Render();
    


    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_M]) {

        if (!m_hasClicked) {
            GameObject* box = new GameObject(m_player->GetPosition());
            box->SetName("Box");
            box->SetMesh("C:/Users/Markus/Desktop/Game/Cube.obj");
            box->SetTexture("C:/Users/Markus/Desktop/Game/Box_texture.jpg");
            box->SetNormal("C:/Users/Markus/Desktop/Game/Box_Normal.png");
            box->CreateActor(GameObject::physicsType::dynamicActor, false);
            box->CreateShape(GameObject::physicsShape::box);

            Math::Vector3 rot = m_player->GetDirection().GetNormal() * 1000;

            box->m_physicsDynamicObject->addForce(physx::PxVec3(rot.x, rot.y, rot.z), physx::PxForceMode::eACCELERATION);
            m_ObjectHandler->Instantiate(*box);
            m_hasClicked = true;
        }
    }    
    else if (keystate[SDL_SCANCODE_L]) {

        if (!m_hasClicked) {
            GameObject* box = new GameObject(m_player->GetPosition());
            box->SetName("Box");
            box->SetMesh("C:/Users/Markus/Desktop/Game/Sphere.obj");
            box->SetTexture("C:/Users/Markus/Desktop/Game/Box_texture.jpg");
            box->SetNormal("C:/Users/Markus/Desktop/Game/Box_Normal.png");
            box->CreateActor(GameObject::physicsType::dynamicActor, false);
            box->CreateShape(GameObject::physicsShape::sphere);

            Math::Vector3 rot = m_player->GetDirection().GetNormal() * 1000;

            box->m_physicsDynamicObject->addForce(physx::PxVec3(rot.x, rot.y, rot.z), physx::PxForceMode::eACCELERATION);
            m_ObjectHandler->Instantiate(*box);
            m_hasClicked = true;
        }
    }    
    else if (keystate[SDL_SCANCODE_K]) {

        if (!m_hasClicked) {
            GameObject* box = new GameObject(m_player->GetPosition());
            box->SetMesh("C:/Users/Markus/Desktop/Game/AK47.obj");
            box->SetTexture("C:/Users/Markus/Desktop/Game/AK_103_Base_Color.png");
            box->SetNormal("C:/Users/Markus/Desktop/Game/AK_103_Normal.png");
            box->CreateActor(GameObject::physicsType::dynamicActor, false);
            box->CreateShape(GameObject::physicsShape::box);

            Math::Vector3 rot = m_player->GetDirection().GetNormal() * 1000;
            box->m_physicsDynamicObject->setMass((PxReal)4.3f);
            box->m_physicsDynamicObject->addForce(physx::PxVec3(rot.x, rot.y, rot.z), physx::PxForceMode::eACCELERATION);
            m_ObjectHandler->Instantiate(*box);
            m_hasClicked = true;
        }
    }    
    else if (keystate[SDL_SCANCODE_N]) {

        if (!m_hasClicked) {
            Light light;
            light.position = m_player->GetPosition();
            light.color = Math::Vector3((rand() % 255 + 1), (rand() % 255 + 1), (rand() % 255 + 1));
            m_ligts.push_back(light);
            m_hasClicked = true;
        }
    }
    else
    {
        m_hasClicked = false;
    }
}

void Spectral::Render()
{
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        m_albedoBuffer[i] = 0;
        m_depthBuffer[i] = 0;
    }
    if(m_render){
        for (const auto& object : m_ObjectHandler->GetGameObjects())
        {
            DrawCube(object);

            //for (const auto& triangle : object->m_triangles)
            //{
            //    DrawTriangle(triangle);
            //}
        }
    }
    if(m_renderPhysX)
    {
        PhysXManager& physXManager = PhysXManager::GetInstance();
        const PxRenderBuffer& rb = physXManager.GetScene()->getRenderBuffer();

        for (PxU32 i = 0; i < rb.getNbLines(); i++)
        {
            const PxDebugLine& line = rb.getLines()[i];
            Math::Vector3 p0(line.pos0.x, line.pos0.y, line.pos0.z);
            Math::Vector3 p1(line.pos1.x, line.pos1.y, line.pos1.z);

            ProjectLine(p0,p1);
        }
    }
    if (m_renderWireframe)
    {
        for (const auto& object : m_ObjectHandler->GetGameObjects())
        {
            for (const auto& triangle : object->m_triangles)
            {
                for (int i = 0; i < 3; i++)
                {
                    ProjectLine(triangle.m_vertex[i], triangle.m_vertex[((1 - i) % 3)]);
                }
            }
        }
    }

}




void Spectral::DrawCube( GameObject* gameObject)
{
    std::list<Triangle> listTriangles;
    Triangle triangleProjected, triangleViewed;


    Triangle clippedTriangle[2];
    Triangle clippedTriangles2[2];

    for (const auto& triangle : gameObject->m_triangles)
    {
        if (triangle.m_normalDirection.Dot(triangle.m_vertex[0] - m_player->GetCameraPosition()) < 0.0f)
        {

            triangleViewed = triangle;
            triangleViewed.m_vertex[0] = m_player->GetCameraMatrix() * triangle.m_vertex[0];
            triangleViewed.m_vertex[1] = m_player->GetCameraMatrix() * triangle.m_vertex[1];
            triangleViewed.m_vertex[2] = m_player->GetCameraMatrix() * triangle.m_vertex[2];


            int clippedTriangles = 0;

            clippedTriangles = triangleViewed.ClipAgainstPlane(
                Math::Vector3(0.0f,0.0f,0.1f), 
                Math::Vector3(0.0f, 0.0f, 1.0f),
                clippedTriangle[0],
                clippedTriangle[1]
            );     


            for (int i = 0; i < clippedTriangles; i++)
            {
                triangleProjected = clippedTriangle[i];

                triangleProjected.m_vertex[0] = m_projectionMatrix * clippedTriangle[i].m_vertex[0];
                triangleProjected.m_vertex[1] = m_projectionMatrix * clippedTriangle[i].m_vertex[1];
                triangleProjected.m_vertex[2] = m_projectionMatrix * clippedTriangle[i].m_vertex[2];

                int vertexesInside = 0;

                // Clipping should occure here

                if (abs(triangleProjected.m_vertex[0].x) <= triangleProjected.m_vertex[0].w &&
                    abs(triangleProjected.m_vertex[0].y) <= triangleProjected.m_vertex[0].w &&
                    abs(triangleProjected.m_vertex[0].z) <= triangleProjected.m_vertex[0].w)
                {
                    vertexesInside++;
                }                
                if (abs(triangleProjected.m_vertex[1].x) <= triangleProjected.m_vertex[1].w &&
                    abs(triangleProjected.m_vertex[1].y) <= triangleProjected.m_vertex[1].w &&
                    abs(triangleProjected.m_vertex[1].z) <= triangleProjected.m_vertex[1].w)
                {
                    vertexesInside++;
                }                
                if (abs(triangleProjected.m_vertex[2].x) <= triangleProjected.m_vertex[2].w &&
                    abs(triangleProjected.m_vertex[2].y) <= triangleProjected.m_vertex[2].w &&
                    abs(triangleProjected.m_vertex[2].z) <= triangleProjected.m_vertex[2].w)
                {
                    vertexesInside++;
                }



                // end clipping

                triangleProjected.m_UV[0].x /= triangleProjected.m_vertex[0].w;
                triangleProjected.m_UV[1].x /= triangleProjected.m_vertex[1].w;
                triangleProjected.m_UV[2].x /= triangleProjected.m_vertex[2].w;

                triangleProjected.m_UV[0].y /= triangleProjected.m_vertex[0].w;
                triangleProjected.m_UV[1].y /= triangleProjected.m_vertex[1].w;
                triangleProjected.m_UV[2].y /= triangleProjected.m_vertex[2].w;

                triangleProjected.m_UV[0].w = 1.0f / triangleProjected.m_vertex[0].w;
                triangleProjected.m_UV[1].w = 1.0f / triangleProjected.m_vertex[1].w;
                triangleProjected.m_UV[2].w = 1.0f / triangleProjected.m_vertex[2].w;

                triangleProjected.m_vertex[0] = triangleProjected.m_vertex[0] / triangleProjected.m_vertex[0].w;
                triangleProjected.m_vertex[1] = triangleProjected.m_vertex[1] / triangleProjected.m_vertex[1].w;
                triangleProjected.m_vertex[2] = triangleProjected.m_vertex[2] / triangleProjected.m_vertex[2].w;

                triangleProjected.m_vertex[0] = triangleProjected.m_vertex[0] + m_viewOffset;
                triangleProjected.m_vertex[1] = triangleProjected.m_vertex[1] + m_viewOffset;
                triangleProjected.m_vertex[2] = triangleProjected.m_vertex[2] + m_viewOffset;

                triangleProjected.m_vertex[0].x *= 0.5f * (float)SCREEN_WIDTH;
                triangleProjected.m_vertex[1].x *= 0.5f * (float)SCREEN_WIDTH;
                triangleProjected.m_vertex[2].x *= 0.5f * (float)SCREEN_WIDTH;

                triangleProjected.m_vertex[0].y *= 0.5f * (float)SCREEN_HEIGHT;
                triangleProjected.m_vertex[1].y *= 0.5f * (float)SCREEN_HEIGHT;
                triangleProjected.m_vertex[2].y *= 0.5f * (float)SCREEN_HEIGHT;
                if(vertexesInside == 3)
                { 
                    //DrawLine(Math::Vector2(triangleProjected.m_vertex[0].x, triangleProjected.m_vertex[0].y), Math::Vector2(triangleProjected.m_vertex[1].x, triangleProjected.m_vertex[1].y), 0xff00ffff);
                    //DrawLine(Math::Vector2(triangleProjected.m_vertex[1].x, triangleProjected.m_vertex[1].y), Math::Vector2(triangleProjected.m_vertex[2].x, triangleProjected.m_vertex[2].y), 0xff00ffff);
                    //DrawLine(Math::Vector2(triangleProjected.m_vertex[2].x, triangleProjected.m_vertex[2].y), Math::Vector2(triangleProjected.m_vertex[0].x, triangleProjected.m_vertex[0].y), 0xff00ffff);
                    TexturedTriangle(gameObject, &triangleProjected, triangle);
                }
                else{
                    //triangleProjected.m_normalDirection = triangle.m_normalDirection;

                    listTriangles.clear();
                    
                    listTriangles.push_back(triangleProjected);
                    size_t nNewTriangles = 1;
                    int nTrisToAdd = 0;
                    
                    
                    //int outcode0 = ComputeOutCode(triangleProjected.m_vertex[0]);
                    //int outcode1 = ComputeOutCode(triangleProjected.m_vertex[1]);
                    //int outcode3 = ComputeOutCode(triangleProjected.m_vertex[2]);
                    
                    //if (!(outcode0 | outcode1 | outcode3))
                    //{
                    //    TexturedTriangle(gameObject, &triangleProjected);
                    //}
                    //else if (outcode0 | outcode1 | outcode3 )
                    //{
                    for (int p = 0; p < 4; p++)
                    {
                        while (nNewTriangles > 0)
                        {
                            nNewTriangles--;
                        
                            switch (p)
                            {
                            case 0:	
                                nTrisToAdd = listTriangles.front().ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, clippedTriangles2[0], clippedTriangles2[1]);
                                break;
                            case 1:	
                                nTrisToAdd = listTriangles.front().ClipAgainstPlane({ 0.0f, (float)SCREEN_HEIGHT - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, clippedTriangles2[0], clippedTriangles2[1]);
                                break;
                            case 2:	
                                nTrisToAdd = listTriangles.front().ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, clippedTriangles2[0], clippedTriangles2[1]);
                                break;
                            case 3:	
                                nTrisToAdd = listTriangles.front().ClipAgainstPlane({ (float)SCREEN_WIDTH - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, clippedTriangles2[0], clippedTriangles2[1]);
                                break;
                            }
                            
                            listTriangles.pop_front();
                            switch (nTrisToAdd)
                            {
                            case 1:
                                listTriangles.push_back(clippedTriangles2[0]);
                                break;
                            case 2:
                                listTriangles.push_back(clippedTriangles2[0]);
                                listTriangles.push_back(clippedTriangles2[1]);
                                break;
                            }
                    
                        }
                        nNewTriangles = listTriangles.size();
                    }
                    
                    for (auto& triangleToDraw : listTriangles)
                    {
                        //DrawLine(Math::Vector2(triangleToDraw.m_vertex[0].x, triangleToDraw.m_vertex[0].y), Math::Vector2(triangleToDraw.m_vertex[1].x, triangleToDraw.m_vertex[1].y),0xff00ffff);
                        //DrawLine(Math::Vector2(triangleToDraw.m_vertex[1].x, triangleToDraw.m_vertex[1].y), Math::Vector2(triangleToDraw.m_vertex[2].x, triangleToDraw.m_vertex[2].y),0xff00ffff);
                        //DrawLine(Math::Vector2(triangleToDraw.m_vertex[2].x, triangleToDraw.m_vertex[2].y), Math::Vector2(triangleToDraw.m_vertex[0].x, triangleToDraw.m_vertex[0].y),0xff00ffff);
                        TexturedTriangle( gameObject, &triangleToDraw,  triangle);
                    }
                }
            }
        }
    }
}

float Spectral::lerp(float a, float b, float f)
{
    return a * (1.0f - f) + (b * f);
}

int Spectral::clip3(const Math::Vector3 n, Math::Vector3& v0, Math::Vector3& v1, Math::Vector3& v2, Math::Vector3& v3) {
    // Distances to the plane ( this is an array parallel
    // to v [] , stored as a vec3 )
    Math::Vector3 dist(v0.Dot(n), v1.Dot(n), v2.Dot(n));
    const float clipEpsilon = 0.00001f, clipEpsilon2 = 0.01f;
    if(dist.x >= clipEpsilon2|| dist.y >= clipEpsilon2|| dist.z >= clipEpsilon2)
    { 
        // Case 1 ( all clipped )
        return 0;
    }
    if (dist.x >= clipEpsilon2 && dist.y >= clipEpsilon2 && dist.z >= clipEpsilon2)
    {
        // Case 2 ( none clipped )
        v3 = v0;
        return 3;
    }
    // There are either 1 or 2 vertices above the clipping plane .
    //bvec3 above = greaterThanEqual(dist, vec3(0.0));
    bool above[3] = { dist.x >= 0.0f , dist.y >= 0.0f , dist.z >= 0.0f };


    bool nextIsAbove;
    // Find the CCW - most vertex above the plane .
    if (above[1] && !above[0]) {
        // Cycle once CCW . Use v3 as a temp
        nextIsAbove = above[2];
        v3 = v0; v0 = v1; v1 = v2; v2 = v3;
        auto tmpDist = dist;

        //dist = dist.yzx;

        dist.x = tmpDist.y;
        dist.y = tmpDist.z;
        dist.z = tmpDist.x;

    }
    else if (above[2] && !above[1]) {
        // Cycle once CW . Use v3 as a temp .
        nextIsAbove = above[0];
        v3 = v2; v2 = v1; v1 = v0; v0 = v3;
        //dist = dist.zxy;



        auto tmpDist = dist;

        dist.x = tmpDist.z;
        dist.y = tmpDist.x;
        dist.z = tmpDist.y;
    }
    else nextIsAbove = above[1];
    // We always need to clip v2 - v0 .
    //v3 = mix(v0, v2, dist[0] / (dist[0] - dist[2]));
    //v3 = lerp(v0, v2, dist[0] / (dist[0] - dist[2]));
    v3 = v0 + (v2 - v0) * (dist.x / (dist.x - dist.y));
    if (nextIsAbove) {
        // Case 3
        //v2 = mix(v1, v2, dist[1] / (dist[1] - dist[2]));
        v2 = v1 + (v2 - v1) * (dist.y / (dist.y - dist.z));
        return 4;
    }
    else {
        // Case 4
        //v1 = mix(v0, v1, dist[0] / (dist[0] - dist[1]));
        v1 = v0 + (v1 - v0) * (dist.x / (dist.x - dist.y));
        v2 = v3; v3 = v0;
        return 3;
    }
}

void Spectral::DrawTriangle(const Triangle& triangle)
{


}


void Spectral::ProjectLine(const Math::Vector3& p0, const Math::Vector3& p1)
{
    Math::Vector3 points[2];

    points[0] = p0;
    points[1] = p1;

    for (int i = 0; i < 2; i++)
    {
        Math::Vector3 toPoint = points[i] - m_player->GetCameraPosition();

        Math::Vector3 cameraDir = m_player->GetDirection().GetNormal();

        float dot = toPoint.Dot(cameraDir);

        if (dot < 1.f)
            return;


        points[i] = m_player->GetCameraMatrix() * points[i];

        points[i] = m_projectionMatrix * points[i];

        points[i] = points[i] / points[i].w;

        points[i] = points[i] + m_viewOffset;

        points[i].x *= 0.5f * (float)SCREEN_WIDTH;
        points[i].y *= 0.5f * (float)SCREEN_HEIGHT;
    }
    Math::Vector2 v2p0(points[0].x, points[0].y);
    Math::Vector2 v2p1(points[1].x, points[1].y);
    if (CohenSutherlandLineClip(v2p0, v2p1))
        DrawLine(v2p0, v2p1, 0xff00ffff);
}


int Spectral::ComputeOutCode(Math::Vector2 point)
{
    int code = INSIDE;  // initialised as being inside of clip window

    if (point.x < xmin)           // to the left of clip window
        code |= LEFT;
    else if (point.x > xmax)      // to the right of clip window
        code |= RIGHT;
    if (point.y < ymin)           // below the clip window
        code |= BOTTOM;
    else if (point.y > ymax)      // above the clip window
        code |= TOP;

    return code;
}
int Spectral::ComputeOutCode(Math::Vector3 point)
{
    int code = INSIDE;  // initialised as being inside of clip window

    if (point.x < xmin)           // to the left of clip window
        code |= LEFT;
    else if (point.x > xmax)      // to the right of clip window
        code |= RIGHT;
    if (point.y < ymin)           // below the clip window
        code |= BOTTOM;
    else if (point.y > ymax)      // above the clip window
        code |= TOP;

    return code;
}



// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
// diagonal from (xmin, ymin) to (xmax, ymax).
bool Spectral::CohenSutherlandLineClip(Math::Vector2& p0, Math::Vector2& p1)
{

    float x0 = p0.x;
    float y0 = p0.y;    
    
    float x1 = p1.x;
    float y1 = p1.y;


    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    int outcode0 = ComputeOutCode(Math::Vector2(x0, y0));
    int outcode1 = ComputeOutCode(Math::Vector2(x1, y1));
    bool accept = false;

    while (true) {
        if (!(outcode0 | outcode1)) {
            // bitwise OR is 0: both points inside window; trivially accept and exit loop
            accept = true;
            break;
        }
        else if (outcode0 & outcode1) {
            // bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
            // or BOTTOM), so both must be outside window; exit loop (accept is false)
            break;
        }
        else {
            // failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            float x, y;

            // At least one endpoint is outside the clip rectangle; pick it.
            int outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

            // Now find the intersection point;
            // use formulas:
            //   slope = (y1 - y0) / (x1 - x0)
            //   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
            //   y = y0 + slope * (xm - x0), where xm is xmin or xmax
            // No need to worry about divide-by-zero because, in each case, the
            // outcode bit being tested guarantees the denominator is non-zero
            if (outcodeOut & TOP) {           // point is above the clip window
                x = x0 + (x1 - x0) * ((float)ymax - y0) / (y1 - y0);
                y = (float)ymax;
            }
            else if (outcodeOut & BOTTOM) { // point is below the clip window
                x = x0 + (x1 - x0) * ((float)ymin - y0) / (y1 - y0);
                y = (float)ymin;
            }
            else if (outcodeOut & RIGHT) {  // point is to the right of clip window
                y = y0 + (y1 - y0) * ((float)xmax - x0) / (x1 - x0);
                x = (float)xmax;
            }
            else if (outcodeOut & LEFT) {   // point is to the left of clip window
                y = y0 + (y1 - y0) * ((float)xmin - x0) / (x1 - x0);
                x = (float)xmin;
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcodeOut == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = ComputeOutCode(Math::Vector2(x0, y0));
                p0.x = x0;
                p0.y = y0;
            }
            else {
                x1 = x;
                y1 = y;
                outcode1 = ComputeOutCode(Math::Vector2(x1, y1));

                p1.x = x1;
                p1.y = y1;
            }
        }
    }
    return accept;
}



void Spectral::TexturedTriangle(GameObject* gameObject, Triangle* triangle, const Triangle& originalTriangle)
{
    uint16_t texWidth = static_cast<uint16_t>(gameObject->GetTextureSize().x);
    uint16_t texHeight= static_cast<uint16_t>(gameObject->GetTextureSize().y);

    int x1 = (int)triangle->m_vertex[0].x;
    int y1 = (int)triangle->m_vertex[0].y;
    float u1 = triangle->m_UV[0].x;
    float v1 = triangle->m_UV[0].y;
    float w1 = triangle->m_UV[0].w;    
    
    int x2 = (int)triangle->m_vertex[1].x;
    int y2 = (int)triangle->m_vertex[1].y;
    float u2 = triangle->m_UV[1].x;
    float v2 = triangle->m_UV[1].y;
    float w2 = triangle->m_UV[1].w;    
    
    int x3 = (int)triangle->m_vertex[2].x;
    int y3 = (int)triangle->m_vertex[2].y;
    float u3 = triangle->m_UV[2].x;
    float v3 = triangle->m_UV[2].y;
    float w3 = triangle->m_UV[2].w;



    triangle->m_lightDot = triangle->m_normalDirection.Dot(m_directionLighting);



    
    Math::Vector3 specularColor(250, 205, 215);
    float diffusePower = 2.0f;
    float specularPower = 5.5f;
    float specularHardness = 5.25f;

    triangle->m_diffuse = Math::Vector3(0, 0, 0);
    triangle->m_specular = Math::Vector3(0, 0, 0);

    for(auto light : m_ligts)
    { 


        Math::Vector3 diffuseColor = light.color;

        Math::Vector3 lightPosition = light.position;
        Math::Vector3 Diffuse;
        Math::Vector3 Specular;

        Math::Vector3 lightDir = lightPosition - originalTriangle.m_vertex[0]; //3D position in space of the surface

        //float distance = lightDir.Length();
        float distance = sqrt(pow(lightPosition.x - originalTriangle.m_vertex[0].x, 2) + pow(lightPosition.y - originalTriangle.m_vertex[0].y, 2) + pow(lightPosition.z - originalTriangle.m_vertex[0].z, 2));
        lightDir = lightDir.GetNormal(); // = normalize(lightDir);
        //distance = distance * distance; //This line may be optimised using Inverse square root

        //Intensity of the diffuse light. Saturate to keep within the 0-1 range.
        float NdotL = triangle->m_normalDirection.Dot(lightDir);
        float intensity = SDL_clamp(NdotL, 0.0f, 1.0f);


        // Calculate the diffuse light factoring in light color, power and the attenuation
        Diffuse = (diffuseColor * intensity) * diffusePower / distance;

        //Calculate the half vector between the light vector and the view vector.
        //This is typically slower than calculating the actual reflection vector
        // due to the normalize function's reciprocal square root
        Math::Vector3 H = (lightDir + m_player->GetDirection());

        //Intensity of the specular light
        float NdotH = triangle->m_normalDirection.Dot(H);
        intensity = std::pow(SDL_clamp(NdotH, 0.0f, 1.0f), specularHardness);

        //Sum up the specular light factoring
        Specular = (specularColor * intensity) * specularPower / distance;
        triangle->m_diffuse = triangle->m_diffuse + Diffuse;
        triangle->m_specular = triangle->m_diffuse + Specular;
    }


    if (y2 < y1)
    {
        std::swap(y1, y2);
        std::swap(x1, x2);
        std::swap(u1, u2);
        std::swap(v1, v2);
        std::swap(w1, w2);
    }
    if (y3 < y1)
    {
        std::swap(y1, y3);
        std::swap(x1, x3);
        std::swap(u1, u3);
        std::swap(v1, v3);
        std::swap(w1, w3);
    }
    if (y3 < y2)
    {
        std::swap(y2, y3);
        std::swap(x2, x3);
        std::swap(u2, u3);
        std::swap(v2, v3);
        std::swap(w2, w3);
    }

    int dy1 = y2 - y1;
    int dx1 = x2 - x1;
    float dv1 = v2 - v1;
    float du1 = u2 - u1;
    float dw1 = w2 - w1;

    int dy2 = y3 - y1;
    int dx2 = x3 - x1;
    float dv2 = v3 - v1;
    float du2 = u3 - u1;
    float dw2 = w3 - w1;

    float tex_u, tex_v, tex_w;
    
    float dax_step = 0, dbx_step = 0,
        du1_step = 0, dv1_step = 0,
        du2_step = 0, dv2_step = 0,
        dw1_step = 0, dw2_step = 0;

    if (dy2)
    {
        dbx_step = dx2 / (float)abs(dy2);
        du2_step = du2 / (float)abs(dy2);
        dv2_step = dv2 / (float)abs(dy2);
        dw2_step = dw2 / (float)abs(dy2);
    }

    if (dy1)
    {
        dax_step = dx1 / (float)abs(dy1);
        du1_step = du1 / (float)abs(dy1);
        dv1_step = dv1 / (float)abs(dy1);
        dw1_step = dw1 / (float)abs(dy1);

        for (int i = y1; i <= y2; i++)
        {
            int ax = (int)(x1 + (float)(i - y1) * dax_step);
            int bx = (int)(x1 + (float)(i - y1) * dbx_step);
            
            float tex_su = u1 + (float)(i - y1) * du1_step;
            float tex_sv = v1 + (float)(i - y1) * dv1_step;
            float tex_sw = w1 + (float)(i - y1) * dw1_step;
            
            float tex_eu = u1 + (float)(i - y1) * du2_step;
            float tex_ev = v1 + (float)(i - y1) * dv2_step;
            float tex_ew = w1 + (float)(i - y1) * dw2_step;
            
            if (ax > bx)
            {
                std::swap(ax, bx);
                std::swap(tex_su, tex_eu);
                std::swap(tex_sv, tex_ev);
                std::swap(tex_sw, tex_ew);
            }
            
            tex_u = tex_su;
            tex_v = tex_sv;
            tex_w = tex_sw;
            
            float tstep = 1.0f / ((float)(bx - ax));
            float t = 0.0f;
            
            for (int j = ax; j < bx; j++)
            {
                tex_u = (1.0f - t) * tex_su + t * tex_eu;
                tex_v = (1.0f - t) * tex_sv + t * tex_ev;
                tex_w = (1.0f - t) * tex_sw + t * tex_ew;
                const int pixelPosition = i * SCREEN_WIDTH + j;
                if (tex_w > m_depthBuffer[pixelPosition] && tex_w < 1.0f)
                {
                    float x = (tex_v / tex_w);
                    float y = (tex_u / tex_w);

                    m_albedoBuffer[pixelPosition] = GetPixel(gameObject, triangle,originalTriangle, x, y, texWidth, texWidth);
                    m_depthBuffer[pixelPosition] = tex_w;
                }
                t += tstep;
            }
        }
    }

    dy1 = y3 - y2;

    if (dy1)
    {
        if (dy2)
            dbx_step = dx2 / (float)abs(dy2);
        dx1 = x3 - x2;
        dv1 = v3 - v2;
        du1 = u3 - u2;
        dw1 = w3 - w2;

        du1_step = 0, dv1_step = 0;
        dax_step = dx1 / (float)abs(dy1);
        du1_step = du1 / (float)abs(dy1);
        dv1_step = dv1 / (float)abs(dy1);
        dw1_step = dw1 / (float)abs(dy1);


        for (int i = y2; i <= y3; i++)
        {
            int ax = (int)(x2 + (float)(i - y2) * dax_step);
            int bx = (int)(x1 + (float)(i - y1) * dbx_step);
            
            float tex_su = u2 + (float)(i - y2) * du1_step;
            float tex_sv = v2 + (float)(i - y2) * dv1_step;
            float tex_sw = w2 + (float)(i - y2) * dw1_step;
            
            float tex_eu = u1 + (float)(i - y1) * du2_step;
            float tex_ev = v1 + (float)(i - y1) * dv2_step;
            float tex_ew = w1 + (float)(i - y1) * dw2_step;
            
            if (ax > bx)
            {
                std::swap(ax, bx);
                std::swap(tex_su, tex_eu);
                std::swap(tex_sv, tex_ev);
                std::swap(tex_sw, tex_ew);
            }
            
            tex_u = tex_su;
            tex_v = tex_sv;
            tex_w = tex_sw;
            
            float tstep = 1.0f / ((float)(bx - ax));
            float t = 0.0f;
            
            for (int j = ax; j < bx; j++)
            {
                tex_u = (1.0f - t) * tex_su + t * tex_eu;
                tex_v = (1.0f - t) * tex_sv + t * tex_ev;
                tex_w = (1.0f - t) * tex_sw + t * tex_ew;
                const int pixelPosition = i * SCREEN_WIDTH + j;
                if (tex_w > m_depthBuffer[pixelPosition] && tex_w < 1.0f) 
                {
                    float x = (tex_v / tex_w);
                    float y = (tex_u / tex_w);
            
                    m_albedoBuffer[pixelPosition] = GetPixel(gameObject,triangle, originalTriangle, x, y, texWidth, texWidth);
                    m_depthBuffer[pixelPosition] = tex_w;
                }
                t += tstep;
            }
        }
    }
}

Uint32 Spectral::GetPixel(GameObject* gameObject, Triangle* triangle, const Triangle& originalTriangle, float u, float v, uint16_t width, uint16_t height)
{
    if (u > 1.0f) {
        u = u - floor(u);
    }
    else if (u < 0.0f) {
        u = u + ceil(-u);
    }
    if (v > 1.0f) {
        v = v - floor(v);
    }
    else if (v < 0.0f) {
        v = v + ceil(-v);
    }

    int x = (int)(u * width);
    int y = (int)(v * height);

    if (y > 0)
        --y;    
    if (x > 0)
        --x;

    int pos = y * width +x;
    uint32_t color = gameObject->m_texture.pixels[pos];



    //if (width == gameObject->GetNormalMapSize().x && width == gameObject->GetNormalMapSize().y)
    //{
    //    uint32_t normalColor = gameObject->m_normalMap.pixels[pos];
    //    uint8_t red = (normalColor >> 24) & 0xFF;
    //    uint8_t green = (normalColor >> 16) & 0xFF;
    //    uint8_t blue = (normalColor >> 8) & 0xFF;
    //
    //    Math::Vector3 normalDirection((float)red / 255.0f - 0.5f, (float)green / 255.0f - 0.5f, (float)blue / 255.0f - 0.5f);
    //    float ammount = normalDirection.Dot(m_directionLighting);
    //    ammount *= 6;
    //    ammount = PxClamp(ammount, 0.0f, 1.0f);
    //    ammount *= PxClamp(triangle->m_lightDot, 0.3f, 1.0f);
    //    
    //    //color = DarkenColor(color, ammount);
    //}
    //else {
    //    //color = DarkenColor(color, PxClamp(triangle->m_lightDot, 0.3f, 1.0f));
    //}
    auto Diffuse = (triangle->m_diffuse + triangle->m_specular);

    Diffuse = Diffuse + PxClamp(triangle->m_lightDot * 10, 0.015f * 255, 1.0f * 255);

    Diffuse = Math::Vector3(SDL_clamp(Diffuse.x, 0.0f, 255.0f), SDL_clamp(Diffuse.y, 0.0f, 255.0f), SDL_clamp(Diffuse.z, 0.0f, 255.0f));

    uint8_t red = (color >> 24) & 0xFF;
    uint8_t green = (color >> 16) & 0xFF;
    uint8_t blue = (color >> 8) & 0xFF;


    Math::Vector3 asd(red * (Diffuse.x / 255), green * (Diffuse.y / 255), blue * (Diffuse.z / 255));

    color = ((uint8_t)asd.x << 24) | ((uint8_t)asd.y << 16) | ((uint8_t)asd.z << 8) | 0xff;




    return color;
}





void Spectral::DrawLine(Math::Vector2 p0, Math::Vector2 p1, uint32_t color)
{
    int x0 = (int)p0.x;
    int y0 = (int)p0.y;
    int x1 = (int)p1.x;
    int y1 = (int)p1.y;

    bool steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int error = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; x++)
    {
        if (steep)
        {
            if (x >= 0 && x < SCREEN_HEIGHT && y >= 0 && y < SCREEN_WIDTH)
            {
                m_albedoBuffer[x * SCREEN_WIDTH + y] = color;
            }
        }
        else
        {
            if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
            {
                m_albedoBuffer[y * SCREEN_WIDTH + x] = color;
            }
        }

        error -= dy;

        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
    
}



uint32_t Spectral::DarkenColor(uint32_t color, float darknessLevel)
{
    uint8_t red = (color >> 24) & 0xFF;
    uint8_t green = (color >> 16) & 0xFF;
    uint8_t blue = (color >> 8) & 0xFF;
    uint8_t alpha = color & 0xFF;

    red =   (uint8_t)(red*darknessLevel);
    green = (uint8_t)(green *darknessLevel);
    blue =  (uint8_t)(blue*darknessLevel);

    return (red << 24) | (green << 16) | (blue << 8) | alpha;
}

