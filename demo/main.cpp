#include <CookEngine.h>

int main(){
    CookEngine::EngineApi engineApi;
    engineApi.Init();
    engineApi.Loop();
    engineApi.Deinit();
    return 0;
}