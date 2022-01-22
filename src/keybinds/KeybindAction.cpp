#include <Geode>

USE_GEODE_NAMESPACE();

bool KeybindAction::operator==(KeybindAction const& other) const {
    return this->id == other.id;
}
KeybindAction::~KeybindAction() {}
KeybindAction* KeybindAction::copy() const {
    return new KeybindAction(*this);
}
bool KeybindAction::inCategory(keybind_category_id const& category) const {
    return vector_utils::contains(this->categories, category);
}

KeybindModifier::~KeybindModifier() {}

KeybindModifier::KeybindModifier(
    std::string         const& name,
    keybind_action_id   const& id,
    keybind_category_id const& category,
    std::string         const& subcategory,
    std::string         const& description
) {
    this->name = name;
    this->id = id;
    this->categories = { category };
    this->subcategory = subcategory;
    this->description = description;
}

KeybindModifier::KeybindModifier(
    std::string         const& name,
    keybind_action_id   const& id,
    decltype(KeybindAction::categories) const& categories,
    std::string         const& subcategory,
    std::string         const& description
) {
    this->name = name;
    this->id = id;
    this->categories = categories;
    this->subcategory = subcategory;
    this->description = description;
}

bool TriggerableAction::invoke(CCNode* node, bool down) const {
    if (this->action) {
        return this->action(node, down);
    }
    return false;
}
bool TriggerableAction::invoke(CCNode* node, keybind_category_id const& id, bool down) const {
    if (this->actionWithID) {
        return this->actionWithID(node, id, down);
    }
    if (this->action) {
        return this->action(node, down);
    }
    return false;
}
TriggerableAction::~TriggerableAction() {}
KeybindAction* TriggerableAction::copy() const {
    return new TriggerableAction(*this);
}

TriggerableAction::TriggerableAction(
    std::string         const& name,
    keybind_action_id   const& id,
    keybind_category_id const& category,
    decltype(TriggerableAction::action) action
) {
    this->name = name;
    this->id = id;
    this->categories = { category };
    this->action = action;
}

TriggerableAction::TriggerableAction(
    std::string         const& name,
    keybind_action_id   const& id,
    keybind_category_id const& category,
    decltype(TriggerableAction::action) action,
    std::string         const& description
) {
    this->name = name;
    this->id = id;
    this->categories = { category };
    this->description = description;
    this->action = action;
}

TriggerableAction::TriggerableAction(
    std::string         const& name,
    keybind_action_id   const& id,
    keybind_category_id const& category,
    decltype(TriggerableAction::actionWithID) action,
    std::string         const& description
) {
    this->name = name;
    this->id = id;
    this->categories = { category };
    this->description = description;
    this->actionWithID = action;
}

TriggerableAction::TriggerableAction(
    std::string         const& name,
    keybind_action_id   const& id,
    std::vector<keybind_category_id> const& categories,
    decltype(actionWithID)     action,
    std::string         const& description
) {
    this->name = name;
    this->id = id;
    this->categories = categories;
    this->description = description;
    this->actionWithID = action;
}

TriggerableAction::TriggerableAction(
    std::string         const& name,
    keybind_action_id   const& id,
    keybind_category_id const& category,
    std::string         const& subcategory,
    decltype(TriggerableAction::action) action,
    std::string         const& description
) {
    this->name = name;
    this->id = id;
    this->categories = { category };
    this->subcategory = subcategory;
    this->description = description;
    this->action = action;
}

TriggerableAction::TriggerableAction(
    std::string         const& name,
    keybind_action_id   const& id,
    keybind_category_id const& category,
    std::string         const& subcategory,
    decltype(TriggerableAction::actionWithID) action,
    std::string         const& description
) {
    this->name = name;
    this->id = id;
    this->categories = { category };
    this->subcategory = subcategory;
    this->description = description;
    this->actionWithID = action;
}

RepeatableAction::~RepeatableAction() {}

KeybindAction* RepeatableAction::copy() const {
    return new RepeatableAction(*this);
}

keybind_category_id::keybind_category_id() {
    m_value = "";
}

keybind_category_id::keybind_category_id(const char* val) {
    if (val) {
        m_value = val;
    } else {
        m_value = "";
    }
}

keybind_category_id::keybind_category_id(std::string const& val) {
    m_value = val;
}

keybind_category_id::~keybind_category_id() {}

const char* keybind_category_id::c_str() const {
    return m_value.c_str();
}

keybind_category_id::operator int() const {
    if (!m_value.size()) return 0;
    return hash(m_value.c_str());
}

size_t keybind_category_id::size() const {
    return m_value.size();
}

keybind_category_id::operator std::string() const {
    return m_value;
}

keybind_category_id keybind_category_id::operator=(std::string const& val) {
    m_value = val;
    return *this;
}

bool keybind_category_id::operator==(keybind_category_id const& other) const {
    return
        string_utils::toLower(m_value) ==
        string_utils::toLower(other.m_value);
}

std::ostream& geode::operator<<(std::ostream& stream, keybind_category_id const& id) {
    return stream << id.m_value;
}

std::size_t std::hash<keybind_category_id>::operator()(keybind_category_id const& category) const {
    return std::hash<decltype(category.m_value)>()(category.m_value);
}
