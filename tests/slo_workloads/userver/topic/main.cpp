#include <tests/slo_workloads/topic/topic.h>

int main(int argc, char** argv) {
    return DoMain(argc, argv, DoCreate, DoRun, DoCleanup, true);
}
