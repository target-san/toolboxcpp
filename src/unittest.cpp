#include <gtest/gtest.h>
#define LOG_DETAILED
#include <log_facade/Log.hpp>
#include <log_facade/Backend.hpp>

#include <stdexcept>
// These are used to get what's received by logger methods
static log_facade::Metadata g_last_metadata;
static log_facade::Record   g_last_record;

struct TestLogger: public log_facade::Logger
{
public:
    TestLogger()
    {
        // Small hack. Works nicely because set_logger detaches pointer from
        // unique_ptr and binds it to internal storage
        log_facade::set_logger(std::unique_ptr<Logger>(this));
    }

    bool is_enabled(log_facade::Metadata const& meta) override
    {
        g_last_metadata = meta;
        return true;
    }

    void write(log_facade::Record const& rec, log_facade::WriterFunc writer) override
    {
        g_last_record = rec;
    }
};

TestLogger g_logger;

struct DummyLogger: log_facade::Logger
{
    bool is_enabled(log_facade::Metadata const&) override { return false; }
    void write(log_facade::Record const&, log_facade::WriterFunc) override {}
};

TEST(TestLogger, BasicInit)
{
    // Ensure nullptr is checked
    EXPECT_THROW(
        log_facade::set_logger(std::unique_ptr<log_facade::Logger>()),
        std::invalid_argument
    );
    // Ensure no double-init
    EXPECT_THROW(
        log_facade::set_logger(std::unique_ptr<log_facade::Logger>(new DummyLogger())),
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
