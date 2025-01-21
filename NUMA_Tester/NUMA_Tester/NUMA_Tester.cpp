#include <iostream>
#include <conio.h>
#include "NodeManager.h"

int main() {
    // Define the number of nodes and cores per node
    // TODO - Query the machine architecture
    const size_t num_nodes = 2;  // Last is ALL NODE
    const size_t num_cores_per_node = 24; // Number of Cores per Node, this equates to threads per Pool for each Node

    // Define the affinity masks for each node
    // TODO - Dynamic calc the affinity masks based on Node and Cores/Node
    std::vector<DWORD_PTR> affinity_masks = {
        0xFFFFFF,          // Affinity mask for first 24 cores
        0xFFFFFF000000,    // Affinity mask for cores 24–47
    };

    // Run the tests and Deallocate the Memory
    {
        // Create the NodeManager
        NodeManager node_manager(num_nodes, num_cores_per_node, affinity_masks);

        // Run the tests
        // TODO : Make this Cmd Line Params
        size_t nLoops = 10;     // Test the whole pool n times
        bool withSleep = false; // sleep for 5 seconds between tests for Profiling to separate the blocks
        node_manager.run_tests(nLoops, withSleep);

        // Print the durations
        // TODO - Batch Mode Results?
        // node_manager.print_durations();
    }

    std::cout << "Done: Press any key to exit!" << std::endl;
    _getch();

    return 0;
}

