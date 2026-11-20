#include "motionfx/motionfx_3d.h"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    using namespace motionfx;

    std::cout << "[TEST] Testing MotionFX Phase 2: True 3D Perspective & Motion Paths..." << std::endl;

    const float width = 1920.0f;
    const float height = 1080.0f;

    // Test 1: Identity / Rest Quad
    Transform3D t_identity{};
    t_identity.rot_x = 0.0f;
    t_identity.rot_y = 0.0f;
    t_identity.rot_z = 0.0f;
    t_identity.pos_z = 0.0f;
    t_identity.fov = 60.0f;

    ProjectedQuad q_id = calculate_perspective_quad(width, height, t_identity);
    assert(!q_id.is_backfacing);
    assert(std::abs(q_id.vertices[0].x - 0.0f) < 1.0f);
    assert(std::abs(q_id.vertices[0].y - 0.0f) < 1.0f);
    assert(std::abs(q_id.vertices[1].x - width) < 1.0f);
    assert(std::abs(q_id.vertices[2].x - width) < 1.0f);
    assert(std::abs(q_id.vertices[2].y - height) < 1.0f);
    assert(std::abs(q_id.vertices[3].y - height) < 1.0f);
    std::cout << "  -> Identity 3D projection matches exact screen dimensions." << std::endl;

    // Test 2: True Perspective Foreshortening on Card Flip (Rot Y = 45 deg)
    Transform3D t_card_flip{};
    t_card_flip.rot_y = 45.0f;
    t_card_flip.fov = 60.0f;

    ProjectedQuad q_flip = calculate_perspective_quad(width, height, t_card_flip);
    assert(!q_flip.is_backfacing);

    // Left edge height: V3.y - V0.y
    float left_edge_h = q_flip.vertices[3].y - q_flip.vertices[0].y;
    // Right edge height: V2.y - V1.y
    float right_edge_h = q_flip.vertices[2].y - q_flip.vertices[1].y;

    std::cout << "  -> Card Flip (RotY=45 deg): Left edge height=" << left_edge_h
              << "px, Right edge height=" << right_edge_h << "px" << std::endl;
    // CRITICAL: True 3D perspective creates trapezoidal foreshortening!
    // In 2D fake rotation, left_edge_h would equal right_edge_h. In true 3D, one is significantly larger!
    assert(std::abs(left_edge_h - right_edge_h) > 50.0f);
    std::cout << "  -> True perspective foreshortening confirmed (height difference > 50px)!" << std::endl;

    // Test 3: Card Flip Backfacing Check (Rot Y = 135 deg)
    Transform3D t_backface{};
    t_backface.rot_y = 135.0f;
    ProjectedQuad q_back = calculate_perspective_quad(width, height, t_backface);
    assert(q_back.is_backfacing);
    std::cout << "  -> Backfacing detection at 135 deg verified." << std::endl;

    // Test 4: Z Depth Translation (Perspective Fly In / Depth)
    Transform3D t_deep{};
    t_deep.pos_z = -500.0f; // farther
    ProjectedQuad q_deep = calculate_perspective_quad(width, height, t_deep);
    float deep_w = q_deep.vertices[1].x - q_deep.vertices[0].x;

    Transform3D t_near{};
    t_near.pos_z = 200.0f; // closer
    ProjectedQuad q_near = calculate_perspective_quad(width, height, t_near);
    float near_w = q_near.vertices[1].x - q_near.vertices[0].x;

    assert(near_w > width);
    assert(deep_w < width);
    std::cout << "  -> Z-depth perspective verified: Deep width=" << deep_w
              << "px (< " << width << "), Near width=" << near_w << "px (> " << width << ")." << std::endl;

    // Test 5: Bézier Motion Path
    MotionPathPoint p0{0.0f, 0.0f};
    MotionPathPoint p1{300.0f, 600.0f};
    MotionPathPoint p2{700.0f, 600.0f};
    MotionPathPoint p3{1000.0f, 0.0f};
    BezierMotionPath path(p0, p1, p2, p3);

    MotionPathPoint start = path.evaluate(0.0);
    MotionPathPoint mid = path.evaluate(0.5);
    MotionPathPoint end = path.evaluate(1.0);

    assert(std::abs(start.x - 0.0f) < 1e-4 && std::abs(start.y - 0.0f) < 1e-4);
    assert(std::abs(end.x - 1000.0f) < 1e-4 && std::abs(end.y - 0.0f) < 1e-4);
    assert(mid.y > 400.0f); // Curved peak
    std::cout << "  -> Bézier Motion Path: Start=(0,0), Mid=(" << mid.x << "," << mid.y << "), End=(1000,0) passed." << std::endl;

    std::cout << "[SUCCESS] All Phase 2 tests passed successfully!" << std::endl;
    return 0;
}
