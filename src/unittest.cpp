#include <gtest/gtest.h>
#define LOG_DETAILED
#include <log_facade/Log.hpp>
#include <log_facade/logger/Backend.hpp>

#include <stdexcept>

using log_facade::logger::Logger;
using log_facade::logger::Metadata;
using log_facade::logger::Record;
// These are used to get what's received by logger methods
static Metadata g_last_metadata;
static Record   g_last_record;

struct TestLogger: public Logger
{
public:
    TestLogger()
    {
        // Small hack. Works nicely because set_logger detaches pointer from
        // unique_ptr and binds it to internal storage
        log_facade::logger::set_logger(this);
    }

    bool is_enabled(Metadata const& meta) override
    {
        g_last_metadata = meta;
        return true;
    }

    void write(Record const& rec, log_facade::WriterFunc writer) override
    {
        g_last_record = rec;
    }
};

TestLogger g_logger;

struct DummyLogger: Logger
{
    bool is_enabled(Metadata const&) override { return false; }
    void write(Record const&, log_facade::WriterFunc) override {}
};

TEST(TestLogger, BasicInit)
{
    // Ensure nullptr is checked
    EXPECT_THROW(
        log_facade::logger::set_logger(nullptr),
        std::invalid_argument
    );
    // Ensure no double-init
    EXPECT_THROW(
        log_facade::logger::set_logger(new DummyLogger()),
        std::logic_error
    );
}

TEST(LogFacade, BasicMacro)
{
    log_facade::Severity sev {};
    log_facade::Channel chan {};
    log_facade::Location loc {};
    // NB: we set local meta on the same line as log message, to ensure it's the same
    $log_error("Say Hi to the world!"); sev = log_facade::Severity::Error; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    EXPECT_EQ   (g_last_metadata.severity, sev);
    EXPECT_STREQ(g_last_metadata.channel,  chan);
    EXPECT_STREQ(g_last_metadata.location.file, loc.file);
    EXPECT_EQ   (g_last_metadata.location.line, loc.line);
    EXPECT_STREQ(g_last_metadata.location.func, loc.func);

    $log_warn("Say Hi to the world!"); sev = log_facade::Severity::Warning; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    EXPECT_EQ   (g_last_metadata.severity, sev);
    EXPECT_STREQ(g_last_metadata.channel,  chan);
    EXPECT_STREQ(g_last_metadata.location.file, loc.file);
    EXPECT_EQ   (g_last_metadata.location.line, loc.line);
    EXPECT_STREQ(g_last_metadata.location.func, loc.func);

    $log_info("Say Hi to the world!"); sev = log_facade::Severity::Info; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    EXPECT_EQ   (g_last_metadata.severity, sev);
    EXPECT_STREQ(g_last_metadata.channel,  chan);
    EXPECT_STREQ(g_last_metadata.location.file, loc.file);
    EXPECT_EQ   (g_last_metadata.location.line, loc.line);
    EXPECT_STREQ(g_last_metadata.location.func, loc.func);

    $log_debug("Say Hi to the world!"); sev = log_facade::Severity::Debug; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    EXPECT_EQ   (g_last_metadata.severity, sev);
    EXPECT_STREQ(g_last_metadata.channel,  chan);
    EXPECT_STREQ(g_last_metadata.location.file, loc.file);
    EXPECT_EQ   (g_last_metadata.location.line, loc.line);
    EXPECT_STREQ(g_last_metadata.location.func, loc.func);

    $log_trace("Say Hi to the world!"); sev = log_facade::Severity::Trace; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    EXPECT_EQ   (g_last_metadata.severity, sev);
    EXPECT_STREQ(g_last_metadata.channel,  chan);
    EXPECT_STREQ(g_last_metadata.location.file, loc.file);
    EXPECT_EQ   (g_last_metadata.location.line, loc.line);
    EXPECT_STREQ(g_last_metadata.location.func, loc.func);
}
