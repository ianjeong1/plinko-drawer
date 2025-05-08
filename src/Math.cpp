#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>

void assignLabels(std::vector<int>& counts, std::vector<float>& labels, float desired_mean, float desired_stddev) {
    const int n = counts.size();
    if (n == 0) return;

    // Total number of balls
    const int total_count = std::accumulate(counts.begin(), counts.end(), 0);
    if (total_count == 0) {
        std::fill(labels.begin(), labels.end(), desired_mean);
        return;
    }

    // Sort buckets by count
    std::vector<size_t> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&counts](size_t a, size_t b) {
        return counts[a] > counts[b];
    });

    // Compute cumulative distribution
    std::vector<float> cum_weights(n);
    cum_weights[0] = counts[indices[0]];
    for (int i = 1; i < n; ++i) {
        cum_weights[i] = cum_weights[i - 1] + counts[indices[i]];
    }

    // Assign labels based on quantiles of a normal distribution
    const float variance = desired_stddev * desired_stddev;
    for (int i = 0; i < n; ++i) {
        const float quantile = (cum_weights[i] - counts[indices[i]] / 2.0f) / total_count;
        // Inverse CDF of normal distribution (mean=0, stddev=1)
        float z_score = 0.0f;
        if (quantile > 0.0f && quantile < 1.0f) {
            // Approximate inverse CDF of normal distribution
            const float t = (quantile < 0.5f) ? std::sqrt(-2.0f * std::log(quantile)) : std::sqrt(-2.0f * std::log(1.0f - quantile));
            z_score = (quantile < 0.5f) ? -t : t;
        }
        labels[indices[i]] = desired_mean + z_score * desired_stddev;
    }

    // Adjust to match desired mean and variance (due to approximation errors)
    float current_mean = 0.0f, current_var = 0.0f;
    for (int i = 0; i < n; ++i) {
        current_mean += counts[i] * labels[i];
    }
    current_mean /= total_count;

    for (int i = 0; i < n; ++i) {
        current_var += counts[i] * (labels[i] - current_mean) * (labels[i] - current_mean);
    }
    current_var /= total_count;

    if (current_var > 0.0f) {
        const float scale = std::sqrt(variance / current_var);
        for (float& label : labels) {
            label = desired_mean + (label - current_mean) * scale;
        }
    }
}