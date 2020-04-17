

#include "tests.hpp"

int main(int argc, char const *argv[]) {
	testIndexQueuePushMultiThreaded(argc, argv);
	testIndexQueuePopMultiThreaded(argc, argv);
    return 0;
}
