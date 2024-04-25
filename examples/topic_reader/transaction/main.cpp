#include "application.h"
#include "options.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/client/driver/driver.h>
=======
#include <src/client/ydb_driver/driver.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/driver/driver.h>
>>>>>>> origin/main

#include <optional>

std::optional<TApplication> App;

void StopHandler(int)
{
    std::cout << "Stopping session" << std::endl;
    if (App) {
        App->Stop();
    } else {
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char* argv[])
{
    signal(SIGINT, &StopHandler);
    signal(SIGTERM, &StopHandler);

    TOptions options(argc, argv);

    App.emplace(options);
    std::cout << "Application initialized" << std::endl;

    App->Run();
    std::cout << "Event loop completed" << std::endl;

    App->Finalize();
}
