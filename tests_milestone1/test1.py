class Graph:
    def __init__(self):
        self.nodes = set()
        self.edges = []

    def add_node(self, value):
        self.nodes.add(value)

    def add_edge(self, from_node, to_node, weight):
        self.edges.append((from_node, to_node, weight))

def dijkstra(graph, start):
    visited = [float('inf')] * len(graph.nodes)
    path = [-1] * len(graph.nodes)

    nodes = set(graph.nodes)

    while nodes:
        min_node = None
        min_dist = float('inf')
        for node in nodes:
            index = ord(node) - ord('A')
            if visited[index] < min_dist:
                min_node = node
                min_dist = visited[index]

        if min_node is None:
            break

        nodes.remove(min_node)
        index = ord(min_node) - ord('A')
        current_weight = visited[index]

        for edge in graph.edges:
            from_node, to_node, weight = edge
            if from_node == min_node:
                weight += current_weight
                to_index = ord(to_node) - ord('A')
                if weight < visited[to_index]:
                    visited[to_index] = weight
                    path[to_index] = min_node

    return visited, path

def shortest_path(graph, start, end):
    distances, paths = dijkstra(graph, start)
    path = []
    end_index = ord(end) - ord('A')
    while end_index != -1:
        path.append(chr(end_index + ord('A')))
        end_index = ord(paths[end_index]) - ord('A') if paths[end_index] != -1 else -1
    return path[::-1], distances[ord(end) - ord('A')]

def print_distances_between_pairs(graph)->int:
    nodes = sorted(list(graph.nodes))
    for i in range(len(nodes)):
        for j in range(i+1, len(nodes)):
            start_node = nodes[i]
            end_node = nodes[j]
            shortest_path_result, distance_result = shortest_path(graph, start_node, end_node)
            print(f"Distance from {start_node} to {end_node}: {distance_result}")

def main():
    
    graph = Graph()
    graph.add_node("A")
    graph.add_node("B")
    graph.add_node("C")
    graph.add_node("D")
    graph.add_node("E")
    graph.add_node("F")
    graph.add_edge("A", "B", 4)
    graph.add_edge("A", "C", 2)
    graph.add_edge("B", "C", 5)
    graph.add_edge("B", "D", 10)
    graph.add_edge("C", "D", 3)
    graph.add_edge("C", "E", 8)
    graph.add_edge("D", "E", 2)
    graph.add_edge("D", "F", 6)
    graph.add_edge("E", "F", 1)

    start_node = "A"
    end_node = "F"

    shortest_path_result, distance_result = shortest_path(graph, start_node, end_node)
    print("Shortest Path:", shortest_path_result)
    print("Shortest Distance:", distance_result)

    print("\nDistances between pairs of edges: are aoihoqrglkgouqrhglkj;qhgouqrg;kljqwnlkrguqwngjqrgqkgniqurg;qrkwg[qrgn;qrwkl]")
    print_distances_between_pairs(graph)

if __name__ == "__main__":
    main()
