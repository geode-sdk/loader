#include <HeaderBase.hpp>
#include <helper/RecursiveMenu.hpp>
#include <utils/convert.hpp>

USE_GEODE_NAMESPACE();

RecursiveMenu* RecursiveMenu::create() {
	return createWithArray(CCArray::create());
}
RecursiveMenu* RecursiveMenu::createWithArray(CCArray* arrayOfItems) {
	RecursiveMenu *ret = new RecursiveMenu();
    if (ret && ret->initWithArray(arrayOfItems)) {
        ret->autorelease();
    }
    else {
        CC_SAFE_DELETE(ret);
    }
    return ret;
}

bool RecursiveMenu::ccTouchBegan(CCTouch* touch, CCEvent* event) {
	if (CCMenu::ccTouchBegan(touch, event)) return true;
	m_pSelectedItem = itemForTouch(touch);
    if (m_pSelectedItem) {
    	std::cout << m_pSelectedItem << " selected" << std::endl;
        m_eState = kCCMenuStateTrackingTouch;
        m_pSelectedItem->selected();
        return true;
    }
    return false;
}

void RecursiveMenu::ccTouchMoved(CCTouch* touch, CCEvent* event) {
	CCMenu::ccTouchMoved(touch, event);
	CCMenuItem *currentItem = itemForTouch(touch);
    if (currentItem != m_pSelectedItem) {
        if (m_pSelectedItem) {
            m_pSelectedItem->unselected();
        }
        m_pSelectedItem = currentItem;
        if (m_pSelectedItem) {
            m_pSelectedItem->selected();
        }
    }
}

CCMenuItem* RecursiveMenu::itemForTouch(CCTouch* touch) {
	return itemForTouch(touch, this);
}

CCMenuItem* RecursiveMenu::itemForTouch(CCTouch* touch, CCNode* child) {
	CCPoint touchLocation = touch->getLocation();
	if (child->getChildren() && child->getChildren()->count() > 0) {
        for (auto& node : cocos::ccArrayToVector<CCNode*>(child->getChildren())) {
            CCMenuItem* child = dynamic_cast<CCMenuItem*>(node);
            if (child && child->isVisible() && child->isEnabled()) {
                CCPoint local = child->convertToNodeSpace(touchLocation);
                CCRect r = child->rect();
                r.origin = CCPointZero;
                if (r.containsPoint(local)) {
                    return child;
                }
            }
            child = itemForTouch(touch, node);
            if (child) return child;
        }
    }
    return nullptr;
}
