class KMeans:
    def __init__(self, n_clusters, max_iter=300):
        self.n_clusters = n_clusters
        self.max_iter = max_iter

    def fit(self, X):
        n_samples = len(X) // 2
        n_features = len(X) // n_samples

        # Initialize centroids
        self.centroids = X[0:n_features * self.n_clusters]

        # Main loop
        for _ in range(self.max_iter):
            # Assign samples to nearest centroid
            labels = self._assign_clusters(X, n_samples, n_features)

            # Update centroids
            new_centroids = [self._calculate_centroid(X, labels, k, n_samples, n_features) for k in range(self.n_clusters)]

            # Check convergence
            if self.centroids == new_centroids:
                break

            self.centroids = new_centroids

    def _assign_clusters(self, X, n_samples, n_features):
        labels = []
        for i in range(n_samples):
            sample = X[i * n_features: (i + 1) * n_features]
            min_distance = float('inf')
            closest_centroid = None
            for idx in range(self.n_clusters):
                centroid = self.centroids[idx * n_features: (idx + 1) * n_features]
                distance = self._euclidean_distance(sample, centroid)
                if distance < min_distance:
                    min_distance = distance
                    closest_centroid = idx
            labels.append(closest_centroid)
        return labels

    def _calculate_centroid(self, X, labels, k, n_samples, n_features):
        cluster_points = [X[i * n_features: (i + 1) * n_features] for i, label in enumerate(labels) if label == k]
        if len(cluster_points) == 0:
            return self.centroids[k * n_features: (k + 1) * n_features]  # If no points in cluster, keep the centroid unchanged
        centroid = [0] * n_features
        for point in cluster_points:
            for i in range(n_features):
                centroid[i] += point[i]
        centroid = [dim / len(cluster_points) for dim in centroid]
        return centroid

    def _euclidean_distance(self, p1, p2):
        return sum((a - b) ** 2 for a, b in zip(p1, p2)) ** 0.5

    def predict(self, X):
        n_samples = len(X) // 2
        n_features = len(X) // n_samples
        return self._assign_clusters(X, n_samples, n_features)


if __name__ == "__main__":
    # Sample data
    X = [1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16]

    kmeans = KMeans(n_clusters=2)
    kmeans.fit(X)
    labels = kmeans.predict(X)
    print("Cluster labels:", labels)
