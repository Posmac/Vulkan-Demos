#include "vkApp.h"

int main()
{
    vk::VkApp app;
    app.Run(600, 800);

    //loop
    // while(app.IsRunning())
    // {
    //     app.Update();
    // }

    app.Destroy();
}
