#include <CookEngine.h>

int main(){
    CookEngine::EngineApi engineApi;
    engineApi.Init();
    engineApi.Deinit();
    return 0;
}