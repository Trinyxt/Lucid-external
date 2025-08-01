#pragma once
#include <cmath>

struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
};

struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    bool operator==(const Vector3& other) const {
        const float epsilon = 0.0001f;
        return fabsf(x - other.x) < epsilon &&
            fabsf(y - other.y) < epsilon &&
            fabsf(z - other.z) < epsilon;
    }
    bool operator!=(const Vector3& other) const {
        return !(*this == other);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    float Dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    float Magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3 Normalized() const {
        float mag = Magnitude();
        if (mag == 0.0f)
            return Vector3(0, 0, 0);
        return Vector3(x / mag, y / mag, z / mag);
    }
};

inline Vector3 VectorAdd(const Vector3& a, const Vector3& b) {
    return Vector3{ a.x + b.x, a.y + b.y, a.z + b.z };
}

inline Vector3 VectorSub(const Vector3& a, const Vector3& b) {
    return Vector3{ a.x - b.x, a.y - b.y, a.z - b.z };
}


struct Vector4 {
    float x, y, z, w;
};

struct Matrix4 {
    float data[16];
};


Vector2 WorldToScreen(Vector3 world, Vector2 dimensions, Matrix4 viewmatrix) {
    Vector4 vec4;

    vec4.x = (world.x * viewmatrix.data[0]) + (world.y * viewmatrix.data[1]) + (world.z * viewmatrix.data[2]) + viewmatrix.data[3];
    vec4.y = (world.x * viewmatrix.data[4]) + (world.y * viewmatrix.data[5]) + (world.z * viewmatrix.data[6]) + viewmatrix.data[7];
    vec4.z = (world.x * viewmatrix.data[8]) + (world.y * viewmatrix.data[9]) + (world.z * viewmatrix.data[10]) + viewmatrix.data[11];
    vec4.w = (world.x * viewmatrix.data[12]) + (world.y * viewmatrix.data[13]) + (world.z * viewmatrix.data[14]) + viewmatrix.data[15];

    if (vec4.w < 0.1f)
        vec4.w = 0.1f;

    Vector3 ndc{};
    ndc.x = vec4.x / vec4.w;
    ndc.y = vec4.y / vec4.w;
    ndc.z = vec4.z / vec4.w;

    Vector2 screenPos = {
        (dimensions.x / 2 * ndc.x) + (dimensions.x / 2),
        -(dimensions.y / 2 * ndc.y) + (dimensions.y / 2)
    };

    if (gui::ScreenBounds) {
        if (screenPos.x < 0 || screenPos.x > dimensions.x || screenPos.y < 0 || screenPos.y > dimensions.y)
            return { -1, -1 };
    }

    return screenPos;
}

struct ScreenSize {
    int Width = 0;
    int Height = 0;
};

