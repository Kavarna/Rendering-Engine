#include "BVH.h"

using namespace Common;
using namespace Components;
using namespace Jnrlib;
using namespace Accelerators;
using namespace BVH;

struct BVHPrimitiveInfo
{
    BVHPrimitiveInfo(size_t primitiveIndex, BoundingBox const& bounds) :
        index(primitiveIndex),
        bounds(bounds),
        centroid(.5f * bounds.pMin + .5f * bounds.pMax)
    { }
    size_t index;
    BoundingBox bounds;
    Position centroid;
};

struct BVHBuildNode
{
    void InitAsLeaf(uint32_t first, uint32_t n, BoundingBox const& bounds_)
    {
        bounds = bounds_;
        numberOfPrimitives = n;
        firstPrimitiveOffset = first;
        children[0] = children[1] = nullptr;
    }

    void InitAsInterior(Axis splitAxis_, std::shared_ptr<BVHBuildNode> c0, std::shared_ptr<BVHBuildNode> c1)
    {
        splitAxis = splitAxis_;
        children[0] = c0;
        children[1] = c1;
        numberOfPrimitives = 0;
        bounds = Union(c0->bounds, c1->bounds);
    }

    BoundingBox bounds;
    std::shared_ptr<BVHBuildNode> children[2];
    Axis splitAxis;
    uint32_t firstPrimitiveOffset;
    uint32_t numberOfPrimitives;
};

struct Context
{
    /* Input */
    const Input& input;
    std::vector<BVHPrimitiveInfo> primitives;

    /* Output */
    uint32_t totalNodes = 0;
    std::vector<size_t> orderedPrimitives;
};

struct Bucket
{
    uint32_t count = 0;
    BoundingBox boundingBox;
};
constexpr const uint32_t NUMBER_OF_BUCKETS = 12;

static std::shared_ptr<BVHBuildNode> RecursiveBuild(Context& ctx, uint32_t start, uint32_t end)
{
    CHECK(start < end) << "Recursive build with invalid range";

    /* TODO: Replace with smart memory allocator */
    auto node = std::make_shared<BVHBuildNode>();

    ctx.totalNodes++;

    /* Create a bounding box for all the primitives that go in this node */
    BoundingBox boundingBox;
    for (uint32_t i = start; i < end; ++i)
    {
        /* Compute bounding box for current primitive */
        uint32_t index0 = ctx.input.indices[ctx.primitives[i].index * 3 + 0];
        uint32_t index1 = ctx.input.indices[ctx.primitives[i].index * 3 + 1];
        uint32_t index2 = ctx.input.indices[ctx.primitives[i].index * 3 + 2];

        BoundingBox box(ctx.input.vertices[index0].position);
        box = Union(box, ctx.input.vertices[index1].position);
        box = Union(box, ctx.input.vertices[index2].position);

        /* Merge the bounding box */
        boundingBox = Union(box, boundingBox);
    }

    CHECK(boundingBox.Diagonal().length() != 0) << "The length of the bounding box must be greater than 0";

    uint32_t numberOfPrimitives = end - start;
    if (numberOfPrimitives <= ctx.input.maxPrimsInNode)
    {
        /* Simple case => build a leaf */
        uint32_t firstPrimitive = (uint32_t)ctx.orderedPrimitives.size();
        for (uint32_t i = start; i < end; ++i)
        {
            ctx.orderedPrimitives.push_back(ctx.primitives[i].index);
        }
        node->InitAsLeaf(firstPrimitive, numberOfPrimitives, boundingBox);
        return node;
    }


    BoundingBox centroidBounds{};
    for (uint32_t i = start; i < end; ++i)
    {
        centroidBounds = Union(centroidBounds, ctx.primitives[i].centroid);
    }
    Axis maximumAxis = centroidBounds.MaximumExtent();
    auto axisIndex = (uint32_t)maximumAxis;

    if (centroidBounds.pMin[axisIndex] == centroidBounds.pMax[axisIndex])
    {
        /* Same centroid for all primitives => Make this a leaf */
        uint32_t firstPrimitive = (uint32_t)ctx.orderedPrimitives.size();
        for (uint32_t i = start; i < end; ++i)
        {
            ctx.orderedPrimitives.push_back(ctx.primitives[i].index);
        }
        node->InitAsLeaf(firstPrimitive, numberOfPrimitives, boundingBox);
        return node;
    }

    uint32_t mid = std::midpoint(start, end);
    /* We have a valid bounding box including all the centroids of the current primitives */
    switch (ctx.input.splitType)
    {
        case SplitType::Middle:
        {
            Float middle = (centroidBounds.pMin[axisIndex] - centroidBounds.pMax[axisIndex]) * Half;
            BVHPrimitiveInfo const* middlePtr = std::partition(
                &ctx.primitives[start], &ctx.primitives[end - 1] + 1,
                [&](BVHPrimitiveInfo const& pi)
            {
                return pi.centroid[axisIndex] < middle;
            });
            mid = (uint32_t)(middlePtr - &ctx.primitives[0]);
            if (mid != end && mid != start) break;
            [[fallthrough]];
        }
        case SplitType::EqualCount:
            mid = std::midpoint(start, end);
            std::nth_element(
                &ctx.primitives[start], &ctx.primitives[mid], &ctx.primitives[end - 1] + 1,
                [&](BVHPrimitiveInfo const& lhs, BVHPrimitiveInfo const& rhs)
            {
                return lhs.centroid[axisIndex] < rhs.centroid[axisIndex];
            });
            break;
        case SplitType::SAH:
        {
            if (numberOfPrimitives <= 4)
            {
                /* Too few primitives for the SAH to be worth it, just do Equal Count instead */
                mid = std::midpoint(start, end);
                std::nth_element(
                    &ctx.primitives[start], &ctx.primitives[mid], &ctx.primitives[end - 1] + 1,
                    [&](BVHPrimitiveInfo const& lhs, BVHPrimitiveInfo const& rhs)
                {
                    return lhs.centroid[axisIndex] < rhs.centroid[axisIndex];
                });
            }
            else
            {
                std::array<Bucket, NUMBER_OF_BUCKETS> buckets;
                for (uint32_t i = start; i < end; ++i)
                {
                    auto const& currentPrimitive = ctx.primitives[i];
                    int bucketIndex = (int)((Float)NUMBER_OF_BUCKETS * GetElementByAxis(boundingBox.Offset(currentPrimitive.centroid), maximumAxis));

                    if (bucketIndex == NUMBER_OF_BUCKETS)
                    {
                        bucketIndex -= 1;
                    }

                    CHECK_GE(bucketIndex, 0);
                    CHECK_LT(bucketIndex, (int)NUMBER_OF_BUCKETS);

                    buckets[bucketIndex].count++;
                    buckets[bucketIndex].boundingBox = Union(buckets[bucketIndex].boundingBox, currentPrimitive.bounds);
                }

                std::array<float, NUMBER_OF_BUCKETS -1> cost;
                float minimumCost = FLT_MAX;
                int minimumBucket = -1;
                for (int i = 0; i < NUMBER_OF_BUCKETS - 1; ++i)
                {
                    BoundingBox b0;
                    BoundingBox b1;
                    uint32_t count0 = 0;
                    uint32_t count1 = 0;

                    for (int j = 0; j <= i; ++j)
                    {
                        b0 = Union(b0, buckets[j].boundingBox);
                        count0 += buckets[j].count;
                    }

                    for (int j = i + 1; j < NUMBER_OF_BUCKETS; ++j)
                    {
                        b1 = Union(b1, buckets[j].boundingBox);
                        count1 += buckets[j].count;
                    }

                    Float b0SurfaceArea = b0.SurfaceArea();
                    Float b1SurfaceArea = b1.SurfaceArea();

                    cost[i] = 1 + (count0 * b0SurfaceArea + count1 * b1SurfaceArea) / boundingBox.SurfaceArea();
                    if (cost[i] < minimumCost)
                    {
                        minimumCost = cost[i];
                        minimumBucket = i;
                    }
                }

                float leafCost = (float)numberOfPrimitives;
                if (numberOfPrimitives > ctx.input.maxPrimsInNode || minimumCost < leafCost)
                {
                    /* Split */
                    BVHPrimitiveInfo const* middlePtr = std::partition(
                        &ctx.primitives[start], &ctx.primitives[end - 1] + 1,
                        [&](BVHPrimitiveInfo const& pi)
                    {
                        int bucketIndex = (int)((Float)NUMBER_OF_BUCKETS * GetElementByAxis(boundingBox.Offset(pi.centroid), maximumAxis));

                        if (bucketIndex == NUMBER_OF_BUCKETS)
                        {
                            bucketIndex -= 1;
                        }

                        CHECK_GE(bucketIndex, 0);
                        CHECK_LT(bucketIndex, (int)NUMBER_OF_BUCKETS);

                        return bucketIndex <= minimumBucket;

                    });
                    mid = (uint32_t)(middlePtr - &ctx.primitives[0]);
                }
                else
                {
                    /* Create leaf */
                    uint32_t firstPrimitive = (uint32_t)ctx.orderedPrimitives.size();
                    for (uint32_t i = start; i < end; ++i)
                    {
                        ctx.orderedPrimitives.push_back(ctx.primitives[i].index);
                    }
                    node->InitAsLeaf(firstPrimitive, numberOfPrimitives, boundingBox);
                    return node;
                }
            }
            break;
        }
        default:
            break;
    }

    node->InitAsInterior(maximumAxis,
                            RecursiveBuild(ctx, start, (uint32_t)mid),
                            RecursiveBuild(ctx, (uint32_t)mid, end));
    

    return node;
}

static uint32_t FlattenBVHTree(std::shared_ptr<BVHBuildNode> node, uint32_t* offset, AccelerationStructure& structure)
{
    CHECK(node != nullptr) << "Cannot flatten a nullptr node";
    std::vector<LinearBVHNode>& nodes = structure.nodes;;
    LinearBVHNode* linearNode = &nodes[*offset];
    linearNode->bounds = node->bounds;
    int myOffset = (*offset)++;
    if (node->numberOfPrimitives > 0)
    {
        /* This node is a leaf => copy info */
        CHECK(node->children[0] == nullptr && node->children[1] == nullptr) << "When flattening a leaf node, there should be no children";

        linearNode->primitiveOffset = node->firstPrimitiveOffset;
        linearNode->primitiveCount = node->numberOfPrimitives;
    }
    else
    {
        linearNode->axis = node->splitAxis;
        linearNode->bounds = node->bounds;
        FlattenBVHTree(node->children[0], offset, structure);
        linearNode->secondChildOffset = FlattenBVHTree(node->children[1], offset, structure);
    }

    return myOffset;
}

static void ReorderPrimitives(Context& ctx, std::vector<uint32_t>& indices)
{
    indices.resize(ctx.input.indices.size());
    for (size_t i = 0; i < ctx.orderedPrimitives.size(); ++i)
    {
        auto currentPrimitive = ctx.orderedPrimitives[i];
        auto index0 = ctx.input.indices[currentPrimitive * 3 + 0];
        auto index1 = ctx.input.indices[currentPrimitive * 3 + 1];
        auto index2 = ctx.input.indices[currentPrimitive * 3 + 2];

        indices[i * 3 + 0] = index0;
        indices[i * 3 + 1] = index1;
        indices[i * 3 + 2] = index2;
    }
}

Output Common::Accelerators::BVH::Generate(Input const& input)
{
    if (input.indices.empty())
        return {}; // Empty Input => Empty Output

    CHECK(input.indices.size() % 3 == 0) << "Cannot generate BVH with non-triangle faces";

    Context ctx{.input = input};

    /* Create array of primitives */
    uint32_t totalPrimitives = (uint32_t)input.indices.size() / 3;
    ctx.primitives.reserve(totalPrimitives);
    for (uint32_t i = 0; i < totalPrimitives; ++i)
    {
        uint32_t index0 = input.indices[i * 3 + 0];
        uint32_t index1 = input.indices[i * 3 + 1];
        uint32_t index2 = input.indices[i * 3 + 2];

        BoundingBox box(input.vertices[index0].position);
        box = Union(box, input.vertices[index1].position);
        box = Union(box, input.vertices[index2].position);

        ctx.primitives.emplace_back(i, box);
    }

    auto root = RecursiveBuild(ctx, 0, totalPrimitives);
    
    /* Build the output */
    Output output{};
    ReorderPrimitives(ctx, output.new_indices);

    /* Flatten BVH tree to be used */
    output.accelerationStructure.nodes.resize(ctx.totalNodes);
    uint32_t offset = 0;
    FlattenBVHTree(root, &offset, output.accelerationStructure);
    CHECK(offset == ctx.totalNodes);

    return output;
}
