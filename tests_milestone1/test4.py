class Graph:
    def __init__(self, vertices):
        self.V = vertices
        self.graph = []

    def add_edge(self, u, v, w):
        self.graph.append((u, v, w))

    def bellman_ford(self, src):
        dist = [float('inf')] * self.V
        dist[src] = 0

        for _ in range(self.V - 1):
            for u, v, w in self.graph:
                if dist[u] != float('inf') and dist[u] + w < dist[v]:
                    dist[v] = dist[u] + w

        for u, v, w in self.graph:
            if dist[u] != float('inf') and dist[u] + w < dist[v]:
                print("Graph contains negative weight cycle")
                return

        return dist

    def print_distances(self, dist):
        print("Shortest distances between every pair of vertices:")
        for i in range(self.V):
            for j in range(self.V):
                print(dist[i * self.V + j], end=" ")
            print()

    def count_paths(self, dist):
        paths = [0] * (self.V * self.V)

        for u, v, w in self.graph:
            paths[u * self.V + v] = 1

        for k in range(self.V):
            for i in range(self.V):
                for j in range(self.V):
                    if dist[i * self.V + k] != float('inf') and dist[k * self.V + j] != float('inf'):
                        if dist[i * self.V + j] == dist[i * self.V + k] + dist[k * self.V + j]:
                            paths[i * self.V + j] += paths[i * self.V + k] * paths[k * self.V + j]

        print("\nNumber of paths between every pair of vertices:")
        for i in range(self.V):
            for j in range(self.V):
                print(paths[i * self.V + j], end=" ")
            print()


# Test case
g = Graph(4)
g.add_edge(0, 1, 5)
g.add_edge(0, 2, 4)
g.add_edge(1, 3, 3)
g.add_edge(2, 1, 6)
g.add_edge(3, 2, 2)

src = 0
distances = g.bellman_ford(src)
if distances:
    g.print_distances(distances)
    g.count_paths(distances)
