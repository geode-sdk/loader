#include <utils/node.hpp>
#include <WackyGeodeMacros.hpp>

USE_GEODE_NAMESPACE();

CCRect geode::cocos::calculateNodeCoverage(std::vector<CCNode*> const& nodes) {
    CCRect coverage;
    for (auto child : nodes) {
        auto pos = child->getPosition() - child->getScaledContentSize() * child->getAnchorPoint();
        auto csize = child->getPosition() + child->getScaledContentSize() * (CCPoint { 1.f, 1.f } - child->getAnchorPoint());
        if (pos.x < coverage.origin.x) {
            coverage.origin.x = pos.x;
        }
        if (pos.y < coverage.origin.y) {
            coverage.origin.y = pos.y;
        }
        if (csize.x > coverage.size.width) {
            coverage.size.width = csize.x;
        }
        if (csize.y > coverage.size.height) {
            coverage.size.height = csize.y;
        }
    }
    return coverage;
}

CCRect geode::cocos::calculateNodeCoverage(CCArray* nodes) {
    CCRect coverage;
    CCARRAY_FOREACH_B_TYPE(nodes, child, CCNode) {
        auto pos = child->getPosition() - child->getScaledContentSize() * child->getAnchorPoint();
        auto csize = child->getPosition() + child->getScaledContentSize() * (CCPoint { 1.f, 1.f } - child->getAnchorPoint());
        if (pos.x < coverage.origin.x) {
            coverage.origin.x = pos.x;
        }
        if (pos.y < coverage.origin.y) {
            coverage.origin.y = pos.y;
        }
        if (csize.x > coverage.size.width) {
            coverage.size.width = csize.x;
        }
        if (csize.y > coverage.size.height) {
            coverage.size.height = csize.y;
        }
    }
    return coverage;
}

CCRect geode::cocos::calculateChildCoverage(CCNode* parent) {
    return calculateNodeCoverage(parent->getChildren());
}
