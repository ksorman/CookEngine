#include "ModelLoader.h"
#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/vector3.h"
#include "utils/Mesh.h"
#include <cassert>
#include <memory>
#include <string>


namespace CookEngine {

std::shared_ptr<Model> ModelLoader::LoadModel(std::string_view modelName)
{
    Assimp::Importer importer;

    const aiScene* pScene = importer.ReadFile(std::string(modelName).c_str(),
      aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices
        | aiProcess_MakeLeftHanded);

    if (pScene && pScene->mNumMeshes) {
        std::shared_ptr<Model> model = std::make_shared<Model>();
        model->meshes.push_back(Mesh());

        Mesh* pMesh = &model->meshes[0];
        const aiMesh* pAiMesh = pScene->mMeshes[0];
        pMesh->verteces.reserve(pAiMesh->mNumVertices);
        pMesh->indeces.reserve(pAiMesh->mNumFaces * 3);

        const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < pAiMesh->mNumVertices; ++i) {
            const aiVector3D* pPos = &(pAiMesh->mVertices[i]);
            const aiVector3D* pTexCoord = pAiMesh->HasTextureCoords(0) ? &(pAiMesh->mTextureCoords[0][i]) : &zero3D;

            Vertex vertex(glm::vec3(pPos->x, pPos->y, pPos->z), glm::vec2(pTexCoord->x, pTexCoord->y));
            pMesh->verteces.push_back(vertex);
        }

        for (int i = 0; i < pAiMesh->mNumFaces; ++i) {
            const aiFace& face = pAiMesh->mFaces[i];
            assert(face.mNumIndices == 3);

            pMesh->indeces.push_back(face.mIndices[0]);
            pMesh->indeces.push_back(face.mIndices[1]);
            pMesh->indeces.push_back(face.mIndices[2]);
        }

        return model;
    }

    return nullptr;
}

std::shared_ptr<Model> ModelLoader::GetModel(std::string_view modelName)
{
    if (m_modelsCache.contains(std::string(modelName)) && m_modelsCache[std::string(modelName)].expired() == false) {
        return m_modelsCache[std::string(modelName)].lock();
    }
    return LoadModel(modelName);
}
}// namespace CookEngine