#include <Geode.hpp>
#include <Log.hpp>
#include <Mod.hpp>
#include <Loader.hpp>
#include <utils/general.hpp>
#include <utils/stream.hpp>
#include <utils/casts.hpp>
#include <Internal.hpp>
#include <iomanip>

USE_GEODE_NAMESPACE();

Mod* LogMod::getMod() const { return m_mod; }

cocos2d::CCObject* LogCCObject::getObject() const { return m_obj; }

log_clock::time_point LogMessage::getTime() const {
    return m_time;
}

std::string LogMessage::getTimeString() const {
    return timePointAsString(m_time);
}

Mod* LogMessage::getSender() const {
    return m_sender;
}

Severity LogMessage::getSeverity() const {
    return m_severity;
}

std::vector<Log*> const& LogMessage::getData() const {
    return m_data;
}

void LogMessage::add(Log* msg) {
    this->m_data.push_back(msg);
}

Log::~Log() {}

LogCCObject::LogCCObject(cocos2d::CCObject* obj) : m_obj(obj) {
    obj->retain();
}

LogCCObject::~LogCCObject() {
    if (this->m_obj) {
        this->m_obj->release();
    }
}

LogMessage::~LogMessage() {
    for (auto const& log : this->m_data) {
        delete log;
    }
}

std::string LogString::toString() const {
    return this->m_string;
}

std::string LogMod::toString() const {
    if (!this->m_mod) return "[ null ]";
    return "[ " + std::string(this->m_mod->getName()) + " ]";
}

std::string LogCCObject::toString() const {
    return "{ " + std::string(typeid(*this->m_obj).name() + 6) + " }";
}

std::string LogMessage::toString(bool logTime) const {
    std::stringstream res;

    if (logTime) {
        const auto t = std::chrono::system_clock::to_time_t(this->m_time);
        tm obj;
        #ifdef _MSC_VER
        localtime_s(&obj, &t);
        #else
        obj = *std::localtime(&t);
        #endif
        res << '[' << std::put_time(&obj, "%H:%M:%S") << "] ";
    }
    res << '[';
    if (this->m_sender) {
        res << this->m_sender->getName();
    } else {
        res << '?';
    }
    res << "]:";
    for (auto const& log : this->m_data) {
        res << ' ' << log->toString();
    }

    return res.str();
}

void LogStream::init() {
    if (!this->m_log) {
        this->m_log = new LogMessage;
    }
}

void LogStream::save() {
    if (this->m_log && this->m_stream.str().size()) {
        this->m_log->add(new LogString(this->m_stream.str()));
        this->m_stream.str(std::string());
    }
}

void LogStream::log() {
    this->init();
    this->save();

    Loader::get()->log(this->m_log);

    #ifdef GEODE_PLATFORM_CONSOLE
    if (Geode::get()->platformConsoleReady()) {
        std::cout << this->m_log->toString(true);
    }
    #endif
}

void LogStream::finish() {
    this->log();

    #ifdef GEODE_PLATFORM_CONSOLE
    if (Geode::get()->platformConsoleReady()) {
        std::cout << "\n";
    } else {
        Geode::get()->queueConsoleMessage(this->m_log);
    }
    #endif

    // Loader manages this memory now
    this->m_log = nullptr;
    this->m_stream.str(std::string());
}

LogStream& LogStream::operator<<(Mod* mod) {
    this->save();
    if (!this->m_log) {
        this->m_log = new LogMessage(mod);
    } else if (!this->m_log->m_sender) {
        this->m_log->m_sender = mod;
    } else {
        this->m_log->add(new LogMod(mod));
    }
    return *this;
}

LogStream& LogStream::operator<<(cocos2d::CCObject* obj) {
    this->save();
    this->init();
    this->m_log->add(new LogCCObject(obj));
    return *this;
}

LogStream& LogStream::operator<<(Severity severity) {
    this->init();
    this->m_log->m_severity = severity;
    return *this;
}

LogStream& LogStream::operator<<(Severity::type severity) {
    this->init();
    this->m_log->m_severity = severity;
    return *this;
}

LogStream& LogStream::operator<<(void* p) {
    this->init();
    *this << as<uintptr_t>(p);
    return *this;
}

LogStream& LogStream::operator<<(std::string const& str) {
    this->init();
    this->m_stream << str;
    return *this;
}

LogStream& LogStream::operator<<(std::string_view const& str) {
    this->init();
    this->m_stream << str;
    return *this;
}

LogStream& LogStream::operator<<(const char* str) {
    this->init();
    this->m_stream << str;
    return *this;
}

LogStream& LogStream::operator<<(uintptr_t n) {
    this->init();
    this->m_stream << n;
    return *this;
}

LogStream& LogStream::operator<<(int n) {
    this->init();
    this->m_stream << n;
    return *this;
}

LogStream& LogStream::operator<<(long n) {
    this->init();
    this->m_stream << n;
    return *this;
}

LogStream& LogStream::operator<<(float n) {
    this->init();
    this->m_stream << n;
    return *this;
}

LogStream& LogStream::operator<<(double n) {
    this->init();
    this->m_stream << n;
    return *this;
}

LogStream& LogStream::operator<<(cocos2d::CCPoint const& pos) {
    this->init();
    this->m_stream << pos.x << ", " << pos.y;
    return *this;
}

LogStream& LogStream::operator<<(cocos2d::CCSize const& size) {
    this->init();
    this->m_stream << size.width << " : " << size.height;
    return *this;
}

LogStream& LogStream::operator<<(cocos2d::CCRect const& rect) {
    *this << rect.origin << " | " << rect.size;
    return *this;
}

LogStream& LogStream::operator<<(geode::endl_type) {
    this->finish();
    return *this;
}

LogStream& LogStream::operator<<(geode::continue_type) {
    this->log();
    return *this;
}

LogStream& LogStream::operator<<(geode::decimal_type) {
    this->m_stream << std::setbase(10);
    return *this;
}

LogStream& LogStream::operator<<(geode::hexadecimal_type) {
    this->m_stream << std::setbase(16);
    return *this;
}

LogStream::~LogStream() {
    if (this->m_log) {
        delete this->m_log;
    }
}

