#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#define LOG_DETAILED
#include <toolboxcpp/log/Log.hpp>
#include <toolboxcpp/log/Logger.hpp>

#include <stdexcept>
#include <iostream>

using namespace toolboxcpp::log;
// These are used to get what's received by logger methods
static Metadata g_last_metadata;
static Record   g_last_record;

struct TestLogger: public Logger
{
public:
    TestLogger()
    {
        set_logger_pointer(this);
    }

    bool is_enabled(Metadata const& meta) override
    {
        g_last_metadata = meta;
        return true;
    }

    void write(Record const& rec, WriterFunc) override
    {
        g_last_record = rec;
    }
};

TestLogger g_logger;

struct DummyLogger
{
    bool is_enabled(Metadata const&) { return false; }
    void write(Record const&, WriterFunc) {}
};

TEST_CASE("Logger basic init", "")
{
    // Ensure nullptr is checked
    CHECK_THROWS_AS(set_logger_pointer(nullptr), std::invalid_argument);
    // Ensure no double-init
    CHECK_THROWS_AS(set_logger(DummyLogger()), std::logic_error);
}

TEST_CASE("Logging severities", "")
{
    Severity sev {};
    Channel chan {};
    Location loc {};
    // NB: we set local meta on the same line as log message, to ensure it's the same
    $log_error("Say Hi to the world!"); sev = Severity::Error; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    CHECK(g_last_metadata.severity      == sev);
    CHECK(g_last_metadata.channel       == chan);
    CHECK(g_last_metadata.location.file == loc.file);
    CHECK(g_last_metadata.location.line == loc.line);
    CHECK(g_last_metadata.location.func == loc.func);

    $log_warn("Say Hi to the world!"); sev = Severity::Warning; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    CHECK(g_last_metadata.severity      == sev);
    CHECK(g_last_metadata.channel       == chan);
    CHECK(g_last_metadata.location.file == loc.file);
    CHECK(g_last_metadata.location.line == loc.line);
    CHECK(g_last_metadata.location.func == loc.func);

    $log_info("Say Hi to the world!"); sev = Severity::Info; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    CHECK(g_last_metadata.severity      == sev);
    CHECK(g_last_metadata.channel       == chan);
    CHECK(g_last_metadata.location.file == loc.file);
    CHECK(g_last_metadata.location.line == loc.line);
    CHECK(g_last_metadata.location.func == loc.func);

    $log_debug("Say Hi to the world!"); sev = Severity::Debug; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    CHECK(g_last_metadata.severity      == sev);
    CHECK(g_last_metadata.channel       == chan);
    CHECK(g_last_metadata.location.file == loc.file);
    CHECK(g_last_metadata.location.line == loc.line);
    CHECK(g_last_metadata.location.func == loc.func);

    $log_trace("Say Hi to the world!"); sev = Severity::Trace; chan = $LogCurrentChannel; loc = $LogCurrentLocation;
    CHECK(g_last_metadata.severity      == sev);
    CHECK(g_last_metadata.channel       == chan);
    CHECK(g_last_metadata.location.file == loc.file);
    CHECK(g_last_metadata.location.line == loc.line);
    CHECK(g_last_metadata.location.func == loc.func);
}
