#include "bvh.h"
#include "util/log.h"

#define BVH_LIMIT 0.0001f

void ResizeBVH(std::vector<NodeBVH>& bvh, size_t index) {
    if (bvh[index].config == BranchBVH::BOTH) {
        ResizeBVH(bvh, bvh[index].left);
        ResizeBVH(bvh, bvh[index].right);
        bvh[index].min = glm::min(bvh[bvh[index].left].min, bvh[index].min);
        bvh[index].max = glm::max(bvh[bvh[index].left].max, bvh[index].max);
        bvh[index].min = glm::min(bvh[bvh[index].right].min, bvh[index].min);
        bvh[index].max = glm::max(bvh[bvh[index].right].max, bvh[index].max);
    } else if (bvh[index].config == BranchBVH::LEFT) {
        ResizeBVH(bvh, bvh[index].left);
        bvh[index].min = glm::min(bvh[bvh[index].left].min, bvh[index].min);
        bvh[index].max = glm::max(bvh[bvh[index].left].max, bvh[index].max);
    } else if (bvh[index].config == BranchBVH::RIGHT) {
        ResizeBVH(bvh, bvh[index].right);
        bvh[index].min = glm::min(bvh[bvh[index].right].min, bvh[index].min);
        bvh[index].max = glm::max(bvh[bvh[index].right].max, bvh[index].max);
    } else {
        return;
    }
}

void SplitBVH(std::vector<NodeBVH>& bvh, size_t index, const std::vector<Primitive>& primitives, std::vector<size_t>& children, const std::vector<AABB> aabbs) {
    #define CBVH bvh[index]
    #define BVHMIN bvh[index].min
    #define BVHMAX bvh[index].max
    
    // split boxes
    float xyz_dist[3] = {
        BVHMAX.x - BVHMIN.x,
        BVHMAX.y - BVHMIN.y,
        BVHMAX.z - BVHMIN.z
    };
    int dist_ind = xyz_dist[0] > xyz_dist[1] ?
        (xyz_dist[0] > xyz_dist[2] ? 0 : 2) :
        (xyz_dist[1] > xyz_dist[2] ? 1 : 2);
    if (xyz_dist[dist_ind] < BVH_LIMIT) {
        size_t stream_index = index;
        for (size_t i = 0; i < children.size(); i++) {
            NodeBVH child = {
                aabbs[children[i]].min,
                aabbs[children[i]].max,
                BranchBVH::LEAF, children[i], 0
            };
            bvh.push_back(child);
            bvh[stream_index].left = bvh.size() - 1;

            if (i + 1 >= children.size()) {
                bvh[stream_index].config = BranchBVH::LEFT;
            } else {
                bvh[stream_index].config = BranchBVH::BOTH;
                NodeBVH next = { BVHMIN, BVHMAX, BranchBVH::LEAF, 0, 0 };
                bvh.push_back(next);
                bvh[stream_index].right = bvh.size() - 1;
                stream_index = bvh.size() - 1;
            }
        }
        ResizeBVH(bvh, index);
        return;
    }
    float mid_dist = xyz_dist[dist_ind] / 2.0f;
    NodeBVH left = { BVHMIN, BVHMAX, BranchBVH::LEAF, 0, 0 };
    NodeBVH right = { BVHMIN, BVHMAX, BranchBVH::LEAF, 0, 0 };
    left.max[dist_ind] -= mid_dist;
    right.min[dist_ind] += mid_dist;

    // subchildren
    std::vector<size_t> left_children;
    std::vector<size_t> right_children;
    for (size_t i = 0; i < children.size(); i++) {
        if (aabbs[children[i]].centroid[dist_ind] < left.max[dist_ind])
            left_children.push_back(children[i]);
        else
            right_children.push_back(children[i]);
    }
    if (left_children.size() > 0 && right_children.size() > 0) {
        CBVH.config = BranchBVH::BOTH;
    } else if (left_children.size() > 0) {
        CBVH.config = BranchBVH::LEFT;
    } else if (right_children.size() > 0) {
        CBVH.config = BranchBVH::RIGHT;
    } else {
        FATAL("This bvh logic should never happen");
        CBVH.config = BranchBVH::LEAF;
    }
    if (left_children.size() > 1) {
        bvh.push_back(left);
        CBVH.left = bvh.size() - 1;
        SplitBVH(bvh, CBVH.left, primitives, left_children, aabbs);
    } else if (left_children.size() == 1) {
        left.config = BranchBVH::LEAF;
        left.left = left_children[0];
        left.min = aabbs[left.left].min;
        left.max = aabbs[left.left].max;
        bvh.push_back(left);
        CBVH.left = bvh.size() - 1;
    }
    if (right_children.size() > 1) {
        bvh.push_back(right);
        CBVH.right = bvh.size() - 1;
        SplitBVH(bvh, CBVH.right, primitives, right_children, aabbs);
    } else if (right_children.size() == 1) {
        right.config = BranchBVH::LEAF;
        right.left = right_children[0];
        right.min = aabbs[right.left].min;
        right.max = aabbs[right.left].max;
        bvh.push_back(right);
        CBVH.right = bvh.size() - 1;
    }

    // resize
    if (bvh[index].config == BranchBVH::BOTH || bvh[index].config == BranchBVH::LEFT) {
        bvh[index].min = glm::min(bvh[bvh[index].left].min, bvh[index].min);
        bvh[index].max = glm::max(bvh[bvh[index].left].max, bvh[index].max);
    }
    if (bvh[index].config == BranchBVH::BOTH || bvh[index].config == BranchBVH::RIGHT) {
        bvh[index].min = glm::min(bvh[bvh[index].right].min, bvh[index].min);
        bvh[index].max = glm::max(bvh[bvh[index].right].max, bvh[index].max);
    }

    #undef CBVH
    #undef BVHMIN
    #undef BVHMAX
}

std::vector<AABB> generateAABBs(const std::vector<Primitive>& primitives) {
    std::vector<AABB> aabbs;
    for (int i = 0; i < primitives.size(); i++) 
        aabbs.push_back(PrimitiveUtils::generateAABB(primitives[i]));
    return aabbs;
}

std::vector<NodeBVH> BVH::create(const std::vector<Primitive>& primitives) {
    std::vector<NodeBVH> bvh;
    std::vector<AABB> aabbs = generateAABBs(primitives);
    NodeBVH root = (NodeBVH){
        glm::vec3(std::numeric_limits<float>::max()),
        glm::vec3(-std::numeric_limits<float>::max()),
        BranchBVH::LEAF, 0, 0
    };
    std::vector<size_t> indices;
    for (size_t i = 0; i < primitives.size(); i++) {
        root.min = glm::min(aabbs[i].min, root.min);
        root.max = glm::max(aabbs[i].max, root.max);
        indices.push_back(i);
    }
    bvh.push_back(root);
    SplitBVH(bvh, 0, primitives, indices, aabbs);
    return bvh;
}

bool BVH::intersect(const Ray& ray, size_t ind, const std::vector<NodeBVH>& bvh) {
    glm::vec3 dfrac = glm::vec3(1.0f / ray.d.x, 1.0f / ray.d.y, 1.0f / ray.d.z);
    float t1 = (bvh[ind].min.x - ray.p.x)*dfrac.x;
    float t2 = (bvh[ind].max.x - ray.p.x)*dfrac.x;
    float t3 = (bvh[ind].min.y - ray.p.y)*dfrac.y;
    float t4 = (bvh[ind].max.y - ray.p.y)*dfrac.y;
    float t5 = (bvh[ind].min.z - ray.p.z)*dfrac.z;
    float t6 = (bvh[ind].max.z - ray.p.z)*dfrac.z;
    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));
    if (tmax < 0 || tmin > tmax) return false;
    return true;
}
