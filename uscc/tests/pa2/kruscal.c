#include <stdio.h>
#include <stdlib.h>

// Structure to represent an edge in the graph
struct Edge {
    int src, dest, weight;
};

// Structure to represent a subset for union-find
struct Subset {
    int parent, rank;
};

// Function to find the set of an element i
int find(struct Subset subsets[], int i) {
    if (subsets[i].parent != i)
        subsets[i].parent = find(subsets, subsets[i].parent);

    return subsets[i].parent;
}

// Function to perform union of two sets
void unionSets(struct Subset subsets[], int x, int y) {
    int xroot = find(subsets, x);
    int yroot = find(subsets, y);

    if (subsets[xroot].rank < subsets[yroot].rank)
        subsets[xroot].parent = yroot;
    else if (subsets[xroot].rank > subsets[yroot].rank)
        subsets[yroot].parent = xroot;
    else {
        subsets[yroot].parent = xroot;
        subsets[xroot].rank++;
    }
}

// Function to compare two edges based on their weight
int compareEdges(const void* a, const void* b) {
    return ((struct Edge*)a)->weight - ((struct Edge*)b)->weight;
}

// Function to perform Kruskal's algorithm
void kruskal(struct Edge edges[], int V, int E) {
    // Allocate memory for subsets
    struct Subset* subsets = (struct Subset*)malloc(V * sizeof(struct Subset));

    // Initialize each subset with its parent and rank
    for (int i = 0; i < V; i++) {
        subsets[i].parent = i;
        subsets[i].rank = 0;
    }

    // Sort edges in non-decreasing order by weight
    qsort(edges, E, sizeof(edges[0]), compareEdges);

    printf("Minimum Spanning Tree using Kruskal's Algorithm:\n");
    int i = 0;  // Index used to pick edges from sorted array
    int edgesCount = 0; // Count of edges included in the MST

    // Process each edge until the MST is formed
    while (edgesCount < V - 1 && i < E) {
        struct Edge nextEdge = edges[i++];

        int x = find(subsets, nextEdge.src);
        int y = find(subsets, nextEdge.dest);

        // If including this edge doesn't cause a cycle, include it in the MST
        if (x != y) {
            printf("(%d, %d) - %d\n", nextEdge.src, nextEdge.dest, nextEdge.weight);
            edgesCount++;
            unionSets(subsets, x, y);
        }
    }

    free(subsets);
}

int main() {
    // Example graph represented by its edges
    int V = 4;  // Number of vertices
    int E = 5;  // Number of edges

    struct Edge edges[] = {
        {0, 1, 10},
        {0, 2, 6},
        {0, 3, 5},
        {1, 3, 15},
        {2, 3, 4}
    };

    kruskal(edges, V, E);

    return 0;
}

