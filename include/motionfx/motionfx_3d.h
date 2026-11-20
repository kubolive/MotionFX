#pragma once

#include "motionfx/motionfx_core.h"
#include <array>
#include <cmath>

namespace motionfx {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct Vec4 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float w{1.0f};
};

struct Matrix4x4 {
    float m[4][4]{};

    static Matrix4x4 identity();
    static Matrix4x4 perspective(float fov_deg, float aspect, float z_near, float z_far);
    static Matrix4x4 translation(float tx, float ty, float tz);
    static Matrix4x4 scaling(float sx, float sy, float sz);
    static Matrix4x4 rotation_x(float angle_deg);
    static Matrix4x4 rotation_y(float angle_deg);
    static Matrix4x4 rotation_z(float angle_deg);
    static Matrix4x4 rotation_euler(float rx_deg, float ry_deg, float rz_deg);

    Matrix4x4 operator*(const Matrix4x4& o) const;
    Vec4 transform(const Vec4& v) const;
    Vec3 project_point(const Vec3& pt, float viewport_w, float viewport_h) const;
};

// Represents 4 vertices of a transformed 3D quad projected to 2D screen space
struct ProjectedQuad {
    std::array<Vec3, 4> vertices; // Top-Left, Top-Right, Bottom-Right, Bottom-Left
    bool is_backfacing{false};
    float min_z{0.0f};
    float max_z{0.0f};
};

// Computes true 3D perspective projection for an OBS Scene Item
ProjectedQuad calculate_perspective_quad(float width, float height,
                                         const Transform3D& t3d,
                                         float anchor_x = 0.5f, float anchor_y = 0.5f);

// 2D Cubic Bézier Motion Path
struct MotionPathPoint {
    float x{0.0f};
    float y{0.0f};
};

class BezierMotionPath {
public:
    BezierMotionPath() = default;
    BezierMotionPath(MotionPathPoint p0, MotionPathPoint p1, MotionPathPoint p2, MotionPathPoint p3);

    void set_control_points(MotionPathPoint p0, MotionPathPoint p1, MotionPathPoint p2, MotionPathPoint p3);
    MotionPathPoint evaluate(double t) const;
    float evaluate_tangent_angle_deg(double t) const;

private:
    MotionPathPoint p0_{};
    MotionPathPoint p1_{};
    MotionPathPoint p2_{};
    MotionPathPoint p3_{};
};

} // namespace motionfx
