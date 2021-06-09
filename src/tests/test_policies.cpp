#include "CoreRegistry.h"
#include "InputModulePlugin.h"
#include "InputStream.h"
#include "Policies.h"
#include <catch2/catch.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

using namespace visor;

auto collection_config = R"(
version: "1.0"

visor:
  taps:
    anycast:
      input_type: mock
      config:
        iface: eth0
  collection:
    # policy name and description
    default_view:
#      description: "a mock view of anycast traffic"
      # input stream to create based on the given tap and optional filter config
      input:
        # this must reference a tap name, or application of the policy will fail
        tap: anycast
        filter:
          bpf: "tcp or udp"
      # stream handlers to attach to this input stream
      # these decide exactly which data to summarize and expose for collection
      handlers:
        # default configuration for the stream handlers
#        config:
#          max_deep_sample: 95
        modules:
          # the keys at this level are unique identifiers
          default_net:
            type: net
          default_dns:
            type: dns
#            config:
#              max_deep_sample: 75
          special_domain:
            type: dns
            config:
              qname_suffix: .mydomain.com
)";

auto collection_config_bad = R"(
visor:
  collection:
    missing:
)";

TEST_CASE("Policies", "[policies]")
{

    SECTION("Good Config")
    {
        CoreRegistry registry(nullptr);
        YAML::Node config_file = YAML::Load(collection_config);

        CHECK(config_file["visor"]["collection"]);
        CHECK(config_file["visor"]["collection"].IsMap());

        CHECK_NOTHROW(registry.tap_manager()->load(config_file["visor"]["taps"], true));
        CHECK_NOTHROW(registry.policy_manager()->load(config_file["visor"]["collection"]));

        auto [policy, lock] = registry.policy_manager()->module_get_locked("default_view");
        CHECK(policy->name() == "default_view");
        CHECK(policy->input_stream()->name() == "anycast_default_view");
        CHECK(policy->input_stream()->config_get<std::string>("bpf") == "tcp or udp");
        CHECK_NOTHROW(policy->start());
    }

    SECTION("Bad Config")
    {
        CoreRegistry registry(nullptr);
        YAML::Node config_file = YAML::Load(collection_config_bad);

        CHECK(config_file["visor"]["collection"]);
        CHECK(config_file["visor"]["collection"].IsMap());
        CHECK_THROWS(registry.policy_manager()->load(config_file["visor"]["collection"]));
    }

}
