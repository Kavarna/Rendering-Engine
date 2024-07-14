#include "BVH.h"

#include <numeric>

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

struct MortonPrimitive
{
    uint32_t primitiveIndex;
    uint32_t mortonCode;
};

struct LBVHTreelet
{
    uint32_t startIndex;
    uint32_t primitiveCount;
    std::vector<std::shared_ptr<BVHBuildNode>> buildNodes;
    std::shared_ptr<BVHBuildNode> root;
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

static uint32_t LeftShift3(uint32_t x)
{
    if (x == (1 << 10))
        x--;

    x = (x | (x << 16)) & 0b0000'0011'0000'0000'0000'0000'1111'1111;
    // x =                  ---- --98 ---- ---- ---- ---- 7654 3210
    x = (x | (x << 8)) & 0b0000'0011'0000'0000'1111'0000'0000'1111;
    // x =                 ---- --98 ---- ---- 7654 ---- ---- 3210
    x = (x | (x << 4)) & 0b0000'0011'0000'1100'0011'0000'1100'0011;
    // x =                 ---- --98 ---- 76-- --54 ---- 32-- --10
    x = (x | (x << 2)) & 0b0000'1001'0010'0100'1001'0010'0100'1001;
    // x =                 ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0

    return x;
}

static uint32_t EncodeMorton3(Position const& v)
{
    CHECK_GE(v.x, 0);
    CHECK_GE(v.y, 0);
    CHECK_GE(v.z, 0);
    return (LeftShift3((uint32_t)v.z) << 2) | (LeftShift3((uint32_t)v.y) << 1) | LeftShift3((uint32_t)v.x);
}

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
        BoundingBox box = ctx.primitives[i].bounds;

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

static bool SafetyCheck(std::vector<MortonPrimitive> const& primitives, uint32_t size)
{
    if (primitives.size() != size)
        return false;

    bool good = true;
    std::unordered_set<uint32_t> primitiveIds;
    for (uint32_t i = 0; i < size; ++i)
    {
        if (primitiveIds.find(primitives[i].primitiveIndex) != primitiveIds.end())
        {
            LOG(WARNING) << "Duplicate primitive index found on index " << i << ": " << primitives[i].primitiveIndex;
            good = false;
        }
        else
        {
            primitiveIds.insert(primitives[i].primitiveIndex);
        }

    }

    return good;
}

static void RadixSort(std::vector<MortonPrimitive>* v)
{
    uint32_t initialSize = (uint32_t)v->size();
    std::vector<MortonPrimitive> tempVector(v->size());
    constexpr uint32_t bitsPerPass = 6;
    constexpr uint32_t numberOfBits = 30;
    static_assert((numberOfBits % bitsPerPass) == 0, "Radix sort bits per pass must evenly divide number of bit");
    constexpr uint32_t numberOfPasses = numberOfBits / bitsPerPass;

    for (uint32_t pass = 0; pass < numberOfPasses; ++pass)
    {
        uint32_t lowBit = pass * bitsPerPass;

        std::vector<MortonPrimitive> const& in = (pass & 1) ? tempVector : *v;
        std::vector<MortonPrimitive>& out = (pass & 1) ? *v : tempVector;

        constexpr uint32_t numberOfBuckets = 1 << bitsPerPass;
        constexpr uint32_t bitMask = (1 << bitsPerPass) - 1;
        uint32_t bucketCount[numberOfBuckets] = {0};
        for (MortonPrimitive const& mp : in)
        {
            uint32_t bucket = (mp.mortonCode >> lowBit) & bitMask;
            CHECK_GE(bucket, 0u);
            CHECK_LE(bucket, numberOfBuckets);
            bucketCount[bucket]++;
        }
        
        int outIndex[numberOfBuckets] = {0};
        outIndex[0] = 0;
        for (uint32_t i = 1; i < numberOfBuckets; ++i)
        {
            outIndex[i] = outIndex[i - 1] + bucketCount[i - 1];
        }

        /* Store sorted values in output array */
        for (MortonPrimitive const& mp : in)
        {
            uint32_t bucket = (mp.mortonCode >> lowBit) & bitMask;
            CHECK_GE(bucket, 0u);
            CHECK_LE(bucket, numberOfBuckets);
            out[outIndex[bucket]++] = mp;
        }
    }

    if (numberOfPasses & 1)
        std::swap(*v, tempVector);
}

static std::shared_ptr<BVHBuildNode> emitLBVH(Context& ctx, std::vector<std::shared_ptr<BVHBuildNode>>& buildNodes,
                                              MortonPrimitive const* mortonPrimitives, uint32_t primitiveCount, int bitIndex,
                                              uint32_t& totalNodes, std::atomic<uint32_t>& orderedPrimsOffset)
{
    if (bitIndex == -1 || primitiveCount < ctx.input.maxPrimsInNode)
    {
        // Leaf node => Create it and return it
        totalNodes++;
        auto& buildNode = buildNodes.emplace_back(std::make_shared<BVHBuildNode>());
        uint32_t firstPrimitive = orderedPrimsOffset.fetch_add(primitiveCount);
        BoundingBox bbox;
        
        for (uint32_t i = 0; i < primitiveCount; ++i)
        {
            uint32_t primitiveIndex = mortonPrimitives[i].primitiveIndex;
            ctx.orderedPrimitives[firstPrimitive + i] = ctx.primitives[primitiveIndex].index;
            bbox = Union(bbox, ctx.primitives[primitiveIndex].bounds);
        }
        buildNode->InitAsLeaf(firstPrimitive, primitiveCount, bbox);
        return buildNode;
    }
    else
    {
        int mask = 1 << bitIndex;
        if ((mortonPrimitives[0].mortonCode & mask) == (mortonPrimitives[primitiveCount - 1].mortonCode & mask))
            return emitLBVH(ctx, buildNodes, mortonPrimitives, primitiveCount,
                            bitIndex - 1, totalNodes, orderedPrimsOffset);

        // Find LVBH split point for this dimension
        uint32_t searchStart = 0;
        uint32_t searchEnd = primitiveCount - 1;
        while (searchStart + 1 != searchEnd)
        {
            uint32_t mid = std::midpoint(searchStart, searchEnd);
            if ((mortonPrimitives[searchStart].mortonCode & mask) == (mortonPrimitives[mid].mortonCode & mask))
            {
                searchStart = mid;
            }
            else
            {
                searchEnd = mid;
            }
        }

        uint32_t splitOffset = searchEnd;

        totalNodes++;

        std::shared_ptr<BVHBuildNode> lbvh[2] = {
            emitLBVH(ctx, buildNodes, mortonPrimitives, splitOffset, bitIndex - 1,
            totalNodes, orderedPrimsOffset),
            emitLBVH(ctx, buildNodes, &mortonPrimitives[splitOffset], primitiveCount - splitOffset,
            bitIndex - 1, totalNodes, orderedPrimsOffset),
        };
        int axis = bitIndex % 3;
        
        auto& buildNode = buildNodes.emplace_back(std::make_shared<BVHBuildNode>());
        buildNode->InitAsInterior(Axis(axis), lbvh[0], lbvh[1]);
        return buildNode;
    }
}

static std::shared_ptr<BVHBuildNode> BuildUpperSAH(std::vector<std::shared_ptr<BVHBuildNode>>& treeletRoots,
                                                   uint32_t start, uint32_t end, uint32_t& totalNodes)
{
    CHECK_LT(start, end);
    uint32_t numNodes = end - start;
    if (numNodes == 1)
        return treeletRoots[start];
    
    std::shared_ptr<BVHBuildNode> node = std::make_unique<BVHBuildNode>();
    totalNodes++;

    BoundingBox bbox;
    BoundingBox centroidBox;
    for (uint32_t i = start; i < end; ++i)
    {
        bbox = Union(bbox, treeletRoots[i]->bounds);
        auto centroid = (treeletRoots[i]->bounds.pMin + treeletRoots[i]->bounds.pMax) * 0.5f;

        centroidBox = Union(centroidBox, centroid);
    }

    uint32_t dim = (uint32_t)centroidBox.MaximumExtent();

    constexpr uint32_t numberOfBuckets = 12;
    Bucket buckets[numberOfBuckets];

    for (uint32_t i = start; i < end; ++i)
    {
        Float centroid = (treeletRoots[i]->bounds.pMin[dim] + treeletRoots[i]->bounds.pMax[dim]) * 0.5f;

        uint32_t bucket = uint32_t((Float)numberOfBuckets * (centroid - centroidBox.pMin[dim]) / (centroidBox.pMax[dim] - centroidBox.pMin[dim]));

        if (bucket == numberOfBuckets)
            bucket--;
        CHECK_GE(bucket, 0u);
        CHECK_LT(bucket, numberOfBuckets);
        
        buckets[bucket].count++;
        buckets[bucket].boundingBox = Union(buckets[bucket].boundingBox, treeletRoots[i]->bounds);
    }

    Float cost[numberOfBuckets - 1];
    float minimumCost = FLT_MAX;
    uint32_t minimumBucket = -1;
    for (uint32_t i = 0; i < numberOfBuckets - 1; ++i)
    {
        BoundingBox b0;
        BoundingBox b1;
        uint32_t count0 = 0;
        uint32_t count1 = 0;
        for (uint32_t j = 0; j <= i; ++j)
        {
            b0 = Union(b0, buckets[j].boundingBox);
            count0 += buckets[j].count;
        }
        for (uint32_t j = i + 1; j < numberOfBuckets; ++j)
        {
            b1 = Union(b1, buckets[j].boundingBox);
            count1 += buckets[j].count;
        }

        Float b0SurfaceArea = b0.SurfaceArea();
        Float b1SurfaceArea = b1.SurfaceArea();

        cost[i] = 1 + (count0 * b0SurfaceArea + count1 * b1SurfaceArea) / bbox.SurfaceArea();
        if (cost[i] < minimumCost)
        {
            minimumCost = cost[i];
            minimumBucket = i;
        }
    }

    /* Split */
    auto middlePtr = std::partition(
        &treeletRoots[start], &treeletRoots[end - 1] + 1,
        [&](std::shared_ptr<BVHBuildNode> const& pi)
        {
            Float centroid =
                (pi->bounds.pMin[dim] + pi->bounds.pMax[dim]) * 0.5f;

            uint32_t bucketIndex = uint32_t((Float)numberOfBuckets * (centroid - centroidBox.pMin[dim]) / (centroidBox.pMax[dim] - centroidBox.pMin[dim]));

            if (bucketIndex == NUMBER_OF_BUCKETS)
            {
                bucketIndex -= 1;
            }

            CHECK_GE(bucketIndex, 0u);
            CHECK_LT(bucketIndex, NUMBER_OF_BUCKETS);

            return bucketIndex <= minimumBucket;

        });
    uint32_t mid = (uint32_t)(middlePtr - &treeletRoots[0]);
    CHECK(mid >= start);
    CHECK(mid <= end);
    node->InitAsInterior((Axis)dim,
                         BuildUpperSAH(treeletRoots, start, mid, totalNodes),
                         BuildUpperSAH(treeletRoots, mid, end, totalNodes));
    return node;

}

static std::shared_ptr<BVHBuildNode> BuildHLBVH(Context& ctx)
{
    BoundingBox boundingBox;
    for (auto const& primitive : ctx.primitives)
    {
        /* Merge the bounding box */
        boundingBox = Union(boundingBox, primitive.centroid);
    }

    std::vector<MortonPrimitive> mortonPrimitives(ctx.primitives.size());

    /* Compute Morton indices */
    ThreadPool::Get()->ExecuteParallelForImmediate(
        [&](uint32_t i)
    {
        constexpr uint32_t mortonBits = 10;
        constexpr uint32_t mortonScale = 1 << mortonBits;
        mortonPrimitives[i].primitiveIndex = (uint32_t)ctx.primitives[i].index;
        Position centroidOffset = boundingBox.Offset(ctx.primitives[i].centroid);
        mortonPrimitives[i].mortonCode = EncodeMorton3(centroidOffset * (Float)mortonScale);
    }, (uint32_t)ctx.primitives.size(), 512);

    RadixSort(&mortonPrimitives);

    /* Build bottom level treelets */
    std::vector<LBVHTreelet> treelets;
    uint32_t mask = 0b00111111111111000000000000000000;
    /* Checking the last 12 bits => clustering into 2^12 grid cells => we will have at most 2^16 treelets */
    for (uint32_t start = 0, end = 1; end <= mortonPrimitives.size(); ++end)
    {
        if (end == mortonPrimitives.size() ||
            (mortonPrimitives[start].mortonCode & mask) != (mortonPrimitives[end].mortonCode & mask))
        {
            uint32_t count = end - start;
            uint32_t maxBVHNodes = 2 * count;
            std::vector<std::shared_ptr<BVHBuildNode>> buildNodes;
            buildNodes.reserve(maxBVHNodes);
            treelets.emplace_back(start, count, buildNodes);
            start = end;
        }
    }

    ctx.orderedPrimitives.resize(ctx.primitives.size());
    std::atomic<uint32_t> totalNodes = 0;
    std::atomic<uint32_t> orderedPrimsOffset = 0;
    ThreadPool::Get()->ExecuteParallelForImmediate(
        [&](uint32_t i)
        {
            uint32_t nodesCreated = 0;
            const int firstBitIndex = 29 - 12; // 29 is the first bit from which we subtract the mask

            treelets[i].root = emitLBVH(ctx, treelets[i].buildNodes, mortonPrimitives.data() + treelets[i].startIndex,
                                        treelets[i].primitiveCount, firstBitIndex, nodesCreated, orderedPrimsOffset);

            totalNodes += nodesCreated;
        }, (uint32_t)treelets.size(), 1);

    ctx.totalNodes = totalNodes.load();

    std::vector<std::shared_ptr<BVHBuildNode>> finishedTreelets;
    finishedTreelets.reserve(treelets.size());
    for (LBVHTreelet& treelet : treelets)
    {
        if (!treelet.buildNodes.empty())
        {
            finishedTreelets.push_back(treelet.root);
        }
    }

    return BuildUpperSAH(finishedTreelets, 0, (uint32_t)finishedTreelets.size(), ctx.totalNodes);
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

    std::shared_ptr<BVHBuildNode> root;
    if (input.splitType == SplitType::HLBVH)
    {
        root = BuildHLBVH(ctx);
    }
    else
    {
        root = RecursiveBuild(ctx, 0, totalPrimitives);
    }
    
    CHECK(root != nullptr) << "Could not build a BVH";

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
