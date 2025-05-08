#include "Ball.hpp"
#include <cmath>

Ball::Ball(sf::Vector2f pos) {
    shape.setRadius(10.f);
    shape.setFillColor(sf::Color::Red);
    shape.setOrigin(10.f, 10.f);
    shape.setPosition(pos);
    velocity = {0.f, 0.f};
}

void Ball::update(float dt) {
    // Always apply gravity
    velocity.y += 980.f * dt;

    // If grounded, apply tangential slope gravity
    if (onGround) {
        sf::Vector2f gravity(0.f, 980.f);
        float gTangent = gravity.x * slopeTangent.x + gravity.y * slopeTangent.y;
        velocity += slopeTangent * gTangent * dt;

        velocity *= 0.99f;
    }

    shape.move(velocity * dt);

    // Wall collision logic
    float radius = shape.getRadius();
    sf::Vector2f pos = shape.getPosition();

    const float screenWidth = 1920.f;

    if (pos.x - radius < 0.f) {
        shape.setPosition(radius, pos.y);
        velocity.x *= -0.8f;
    } else if (pos.x + radius > screenWidth) {
        shape.setPosition(screenWidth - radius, pos.y);
        velocity.x *= -0.8f;
    }

    onGround = false;
}

void Ball::collideWith(Ball& other) {
    sf::Vector2f posA = shape.getPosition();
    sf::Vector2f posB = other.shape.getPosition();

    sf::Vector2f delta = posB - posA;
    float dist2 = delta.x * delta.x + delta.y * delta.y;
    float radius = shape.getRadius();
    float combinedRadius = 2 * radius;

    if (dist2 > combinedRadius * combinedRadius || dist2 == 0.f)
        return;

    float dist = std::sqrt(dist2);
    sf::Vector2f normal = delta / dist;

    // Minimum translation distance to separate balls
    float overlap = 0.5f * (combinedRadius - dist);
    shape.move(-normal * overlap);
    other.shape.move(normal * overlap);

    // Simple elastic collision
    sf::Vector2f vA = velocity;
    sf::Vector2f vB = other.velocity;

    float dotA = vA.x * normal.x + vA.y * normal.y;
    float dotB = vB.x * normal.x + vB.y * normal.y;

    float mass = 0.8f;
    float restitution = 0.9f; // Slight energy loss

    float impulse = (dotB - dotA) * restitution;

    velocity += impulse * normal;
    other.velocity -= impulse * normal;
}

bool Ball::checkCollision(const Line& line) {
    const float groundTolerance = 2.f;
    sf::Vector2f p = shape.getPosition();
    sf::Vector2f a = line.getStart();
    sf::Vector2f b = line.getEnd();

    sf::Vector2f ab = b - a;
    sf::Vector2f ap = p - a;

    float ab_len2 = ab.x * ab.x + ab.y * ab.y;
    if (ab_len2 < 1e-6f) return false;

    // Project point onto segment
    float t = std::max(0.f, std::min(1.f, (ap.x * ab.x + ap.y * ab.y) / ab_len2));
    sf::Vector2f closest = a + t * ab;

    sf::Vector2f diff = p - closest;
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float radius = shape.getRadius();

    if (dist < radius + groundTolerance) {
        // Compute slope tangent
        slopeTangent = {-diff.y, diff.x};
        float len = std::sqrt(slopeTangent.x * slopeTangent.x + slopeTangent.y * slopeTangent.y);
        if (len > 0.f) slopeTangent /= len;

        if (dist < radius && dist > 0.f) {
            sf::Vector2f normal = diff / dist;
            float dot = velocity.x * normal.x + velocity.y * normal.y;

            if (dot < 0.f) {
                // Proper bounce: reflect and dampen
                velocity -= (1.8f * dot) * normal;
            }

            // Push out of collision
            float penetration = radius - dist;
            shape.move(normal * penetration);
        }

        // Now we say it's grounded
        if (dist < radius + groundTolerance)
            onGround = true;

        return true;
    }

    return false;
}

sf::Vector2f Ball::getPosition() const {
    return shape.getPosition();
}

sf::Vector2f Ball::getVelocity() const {
    return velocity;
}

void Ball::reset() {
    shape.setPosition({960.f, 100.f});
    velocity = {0.f, 0.f};
}

void Ball::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(shape, states);
}
