#include <benchmark/benchmark.h>
#include <map>
#include <nameof.hpp>
#include <optional>
#include <variant>
#include <serdepp/serializer.hpp>
#include <serdepp/adaptor/nlohmann_json.hpp>
#include <serdepp/adaptor/toml11.hpp>
#include <serdepp/adaptor/yaml-cpp.hpp>
#include <fmt/format.h>

struct test {
    template<class Context>
    constexpr static auto serde(Context& context, test& value) {
      serde::serde_struct(context, value)
          .field(&test::str, "str")
          .field(&test::i, "i")
          .field(&test::vec, "vec")
          .field(&test::sm, "sm");
      //.no_remain();
    }
    std::string str;
    int i;
    std::vector<std::string> vec;
    std::map<std::string, std::string> sm;
};

namespace ns {
    struct nl_test {
        std::string str;
        int i;
        std::vector<std::string> vec;
        std::map<std::string, std::string> sm;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(nl_test, str, i, vec, sm)
    };
}


using namespace toml::literals;

nlohmann::json json_v = R"({
"str" : "hello",
"i": 10,
"vec": [ "one", "two" ],
"sm": { "one" : "tone", "two" : "ttwo"}
})"_json;

test base_t = serde::serialize<test>(json_v);

toml::value toml_v = serde::deserialize<toml::value>(base_t);

//toml::value toml_v = R"(
//vec = ["one", "two"]
//i = 10
//str = "hello"
//[sm]
//one = "tone"
//two = "ttwo"
//)"_toml;

YAML::Node yaml_v = serde::deserialize<YAML::Node>(base_t);

static void nljson_set_se_bench(benchmark::State& state) {
    for(auto _ : state) {
        serde::serialize<test>(json_v);
    }
}

static void nljson_set_nl_bench(benchmark::State& state) {
    for(auto _ : state) {
        json_v.get<ns::nl_test>();
    }
}

static void nljson_get_se_bench(benchmark::State& state) {
    test t;
    t.i = 10;
    t.str = "hello";
    t.vec = {"one", "two"};
    t.sm = {{"one", "town"}, {"two", "ttwo"}};
    for(auto _ : state) {
        serde::deserialize<nlohmann::json>(t);
    }
}

static void nljson_get_nl_bench(benchmark::State& state) {
    ns::nl_test t;
    t.i = 10;
    t.str = "hello";
    t.vec = {"one", "two"};
    t.sm = {{"one", "town"}, {"two", "ttwo"}};
    for(auto _ : state) {
         nlohmann::json{t};
    }
}

namespace ext
{
struct test
{
    std::string str;
    int i;
    std::vector<std::string> vec;
    std::map<std::string, std::string> sm;
    template<typename C, template<typename ...> class M, template<typename ...> class A>
    void from_toml(const toml::basic_value<C, M, A>& v)
    {
        this->str = toml::find<std::string>(v, "str");
        this->i   = toml::find<int>(v, "i");
        this->vec = toml::find<std::vector<std::string>>(v, "vec");
        this->sm  = toml::find<std::map<std::string, std::string>>(v, "sm");
        return;
    }

    toml::value into_toml() const // you need to mark it const.
    {
        return toml::value{{"str", this->str}, {"i", this->i}, {"vec", this->vec}, {"sm", this->sm}};
    }
};
} // ext

static void toml11_set_se_bench(benchmark::State& state) {
    for(auto _ : state) {
        serde::serialize<test>(toml_v);
    }
}

static void toml11_set_tl_bench(benchmark::State& state) {
    for(auto _ : state) {
        toml::get<ext::test>(toml_v);
    }
}

static void toml11_get_se_bench(benchmark::State& state) {
    test t;
    t.i = 10;
    t.str = "hello";
    t.vec = {"one", "two"};
    t.sm = {{"one", "town"}, {"two", "ttwo"}};
    for(auto _ : state) {
        serde::deserialize<serde::toml_v>(t);
    }
}

static void toml11_get_tl_bench(benchmark::State& state) {
    ext::test t;
    t.i = 10;
    t.str = "hello";
    t.vec = {"one", "two"};
    t.sm = {{"one", "town"}, {"two", "ttwo"}};
    for(auto _ : state) {
        toml::value v(t);
    }
}

namespace YAML {
    template<> struct convert<test> {
        static Node encode(const test& v) {
            Node node;
            node["str"] = v.str;
            node["i"] = v.i;
            node["vec"] = v.vec;
            node["sm"] = v.sm;
            return node;
        }

        static bool decode(const Node& node, test& v) {
            v.str = node["str"].as<std::string>();
            v.i   = node["i"].as<int>();
            v.vec = node["vec"].as<std::vector<std::string>>();
            v.sm  = node["sm"].as<std::map<std::string, std::string>>();
            return true;
        }
    };
}

static void yaml_set_se_bench(benchmark::State& state) {
    for(auto _ : state) {
        serde::serialize<test>(yaml_v);
    }
}

static void yaml_set_tl_bench(benchmark::State& state) {
    for(auto _ : state) {
        yaml_v.as<test>();
    }
}

static void yaml_get_se_bench(benchmark::State& state) {
    test t;
    t.i = 10;
    t.str = "hello";
    t.vec = {"one", "two"};
    t.sm = {{"one", "town"}, {"two", "ttwo"}};
    for(auto _ : state) {
        serde::deserialize<serde::yaml>(t);
    }
}

static void yaml_get_tl_bench(benchmark::State& state) {
    test t;
    t.i = 10;
    t.str = "hello";
    t.vec = {"one", "two"};
    t.sm = {{"one", "town"}, {"two", "ttwo"}};
    for(auto _ : state) {
        YAML::Node v(t);
    }
}

BENCHMARK(nljson_set_se_bench);
BENCHMARK(nljson_set_nl_bench);
BENCHMARK(nljson_get_se_bench);
BENCHMARK(nljson_get_nl_bench);
BENCHMARK(toml11_set_se_bench);
BENCHMARK(toml11_set_tl_bench);
BENCHMARK(toml11_get_se_bench);
BENCHMARK(toml11_get_tl_bench);
BENCHMARK(yaml_set_se_bench);
BENCHMARK(yaml_set_tl_bench);
BENCHMARK(yaml_get_se_bench);
BENCHMARK(yaml_get_tl_bench);
BENCHMARK_MAIN();
