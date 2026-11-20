#include "motionfx/motionfx_3d.h"
#include <cstring>
#include <algorithm>

namespace motionfx {

#ifndef M_PI
constexpr float M_PI = 3.14159265358979323846f;
#endif

Matrix4x4 Matrix4x4::identity() {
    Matrix4x4 res{};
    res.m[0][0] = 1.0f;
    res.m[1][1] = 1.0f;
    res.m[2][2] = 1.0f;
    res.m[3][3] = 1.0f;
    return res;
}

Matrix4x4 Matrix4x4::perspective(float fov_deg, float aspect, float z_near, float z_far) {
    Matrix4x4 res{};
    float rad = fov_deg * (M_PI / 180.0f);
    float tanHalf = std::tan(rad * 0.5f);

    res.m[0][0] = 1.0f / (aspect * tanHalf);
    res.m[1][1] = 1.0f / tanHalf;
    res.m[2][2] = (z_far + z_near) / (z_near - z_far);
    res.m[2][3] = (2.0f * z_far * z_near) / (z_near - z_far);
    res.m[3][2] = -1.0f;
    res.m[3][3] = 0.0f;
    return res;
}

Matrix4x4 Matrix4x4::translation(float tx, float ty, float tz) {
    Matrix4x4 res = Matrix4x4::identity();
    res.m[0][3] = tx;
    res.m[1][3] = ty;
    res.m[2][3] = tz;
    return res;
}

Matrix4x4 Matrix4x4::scaling(float sx, float sy, float sz) {
    Matrix4x4 res{};
    res.m[0][0] = sx;
    res.m[1][1] = sy;
    res.m[2][2] = sz;
    res.m[3][3] = 1.0f;
    return res;
}

Matrix4x4 Matrix4x4::rotation_x(float angle_deg) {
    Matrix4x4 res = Matrix4x4::identity();
    float rad = angle_deg * (M_PI / 180.0f);
    float c = std::cos(rad);
    float s = std::sin(rad);

    res.m[1][1] = c;
    res.m[1][2] = -s;
    res.m[2][1] = s;
    res.m[2][2] = c;
    return res;
}

Matrix4x4 Matrix4x4::rotation_y(float angle_deg) {
    Matrix4x4 res = Matrix4x4::identity();
    float rad = angle_deg * (M_PI / 180.0f);
    float c = std::cos(rad);
    float s = std::sin(rad);

    res.m[0][0] = c;
    res.m[0][2] = s;
    res.m[2][0] = -s;
    res.m[2][2] = c;
    return res;
}

Matrix4x4 Matrix4x4::rotation_z(float angle_deg) {
    Matrix4x4 res = Matrix4x4::identity();
    float rad = angle_deg * (M_PI / 180.0f);
    float c = std::cos(rad);
    float s = std::sin(rad);

    res.m[0][0] = c;
    res.m[0][1] = -s;
    res.m[1][0] = s;
    res.m[1][1] = c;
    return res;
}

Matrix4x4 Matrix4x4::rotation_euler(float rx_deg, float ry_deg, float rz_deg) {
    Matrix4x4 rx = rotation_x(rx_deg);
    Matrix4x4 ry = rotation_y(ry_deg);
    Matrix4x4 rz = rotation_z(rz_deg);
    // Standard Euler composition: Rz * Ry * Rx
    return rz * (ry * rx);
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& o) const {
    Matrix4x4 res{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += m[i][k] * o.m[k][j];
            }
            res.m[i][j] = sum;
        }
    }
    return res;
}

Vec4 Matrix4x4::transform(const Vec4& v) const {
    Vec4 res;
    res.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w;
    res.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w;
    res.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w;
    res.w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w;
    return res;
}

ProjectedQuad calculate_perspective_quad(float width, float height,
                                         const Transform3D& t3d,
                                         float anchor_x, float anchor_y) {
    ProjectedQuad quad{};
    if (width <= 0.0f || height <= 0.0f) return quad;

    float fov = (t3d.fov > 10.0f && t3d.fov < 170.0f) ? t3d.fov : 60.0f;
    float aspect = width / height;
    float fov_rad = fov * (M_PI / 180.0f);
    float eye_dist = (height * 0.5f) / std::tan(fov_rad * 0.5f);

    float z_near = 1.0f;
    float z_far = eye_dist * 20.0f;

    // View matrix: eye positioned at (0, 0, eye_dist) looking at (0, 0, 0)
    Matrix4x4 view = Matrix4x4::translation(0.0f, 0.0f, -eye_dist);

    // Projection matrix
    Matrix4x4 proj = Matrix4x4::perspective(fov, aspect, z_near, z_far);

    // Model matrix:
    // Center the item quad around anchor point in model space, rotate, then apply Z depth
    Matrix4x4 rot = Matrix4x4::rotation_euler(t3d.rot_x, t3d.rot_y, t3d.rot_z);
    Matrix4x4 trans_z = Matrix4x4::translation(0.0f, 0.0f, t3d.pos_z);
    Matrix4x4 model = trans_z * rot;

    // Full MVP matrix
    Matrix4x4 mvp = proj * (view * model);

    // 4 local corners centered around the anchor point:
    // Top-Left, Top-Right, Bottom-Right, Bottom-Left
    float left = -width * anchor_x;
    float right = width * (1.0f - anchor_x);
    float top = height * anchor_y;       // In 3D space, Y up is positive
    float bottom = -height * (1.0f - anchor_y);

    std::array<Vec4, 4> corners = {
        Vec4{left, top, 0.0f, 1.0f},       // 0: Top-Left
        Vec4{right, top, 0.0f, 1.0f},      // 1: Top-Right
        Vec4{right, bottom, 0.0f, 1.0f},   // 2: Bottom-Right
        Vec4{left, bottom, 0.0f, 1.0f}     // 3: Bottom-Left
    };

    quad.min_z = 1e9f;
    quad.max_z = -1e9f;

    for (size_t i = 0; i < 4; ++i) {
        Vec4 clip = mvp.transform(corners[i]);
        float inv_w = (std::abs(clip.w) > 1e-6f) ? (1.0f / clip.w) : 1.0f;

        // Normalized Device Coordinates (NDC: -1 to +1)
        float ndc_x = clip.x * inv_w;
        float ndc_y = clip.y * inv_w;

        // Convert NDC to screen pixel coordinates
        // NDC (-1, 1) -> (0, 0), NDC (1, -1) -> (width, height)
        quad.vertices[i].x = (ndc_x + 1.0f) * 0.5f * width;
        quad.vertices[i].y = (1.0f - ndc_y) * 0.5f * height;
        quad.vertices[i].z = clip.z;

        quad.min_z = std::min(quad.min_z, clip.z);
        quad.max_z = std::max(quad.max_z, clip.z);
    }

    // Determine backfacing via 2D cross product in screen space
    float edge1_x = quad.vertices[1].x - quad.vertices[0].x;
    float edge1_y = quad.vertices[1].y - quad.vertices[0].y;
    float edge2_x = quad.vertices[2].x - quad.vertices[1].x;
    float edge2_y = quad.vertices[2].y - quad.vertices[1].y;
    float cross_z = edge1_x * edge2_y - edge1_y * edge2_x;

    // In screen coordinates (where Y is down), clockwise winding gives cross_z > 0.
    // If cross_z < 0, the quad has flipped over to the back face.
    quad.is_backfacing = (cross_z < 0.0f);
    return quad;
}

// ============================================================================
// BezierMotionPath
// ============================================================================

BezierMotionPath::BezierMotionPath(MotionPathPoint p0, MotionPathPoint p1,
                                   MotionPathPoint p2, MotionPathPoint p3)
    : p0_(p0), p1_(p1), p2_(p2), p3_(p3) {}

void BezierMotionPath::set_control_points(MotionPathPoint p0, MotionPathPoint p1,
                                         MotionPathPoint p2, MotionPathPoint p3) {
    p0_ = p0;
    p1_ = p1;
    p2_ = p2;
    p3_ = p3;
}

MotionPathPoint BezierMotionPath::evaluate(double t) const {
    t = std::clamp(t, 0.0, 1.0);
    double u = 1.0 - t;
    double tt = t * t;
    double uu = u * u;
    double uuu = uu * u;
    double ttt = tt * t;

    MotionPathPoint res;
    res.x = static_cast<float>(uuu * p0_.x + 3.0 * uu * t * p1_.x + 3.0 * u * tt * p2_.x + ttt * p3_.x);
    res.y = static_cast<float>(uuu * p0_.y + 3.0 * uu * t * p1_.y + 3.0 * u * tt * p2_.y + ttt * p3_.y);
    return res;
}

float BezierMotionPath::evaluate_tangent_angle_deg(double t) const {
    t = std::clamp(t, 0.0, 1.0);
    double u = 1.0 - t;

    // First derivative B'(t) = 3(1-t)^2 (P1 - P0) + 6(1-t)t (P2 - P1) + 3t^2 (P3 - P2)
    double dx = 3.0 * u * u * (p1_.x - p0_.x) +
                6.0 * u * t * (p2_.x - p1_.x) +
                3.0 * t * t * (p3_.x - p2_.x);
    double dy = 3.0 * u * u * (p1_.y - p0_.y) +
                6.0 * u * t * (p2_.y - p1_.y) +
                3.0 * t * t * (p3_.y - p2_.y);

    return static_cast<float>(std::atan2(dy, dx) * (180.0 / M_PI));
}

} // namespace motionfx
