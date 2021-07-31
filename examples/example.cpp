#include <serdepp/serializer.hpp>
#include <serdepp/adaptor/nlohmann_json.hpp>
#include <serdepp/adaptor/toml11.hpp>
#include <serdepp/adaptor/fmt.hpp>
#include <serdepp/adaptor/yaml-cpp.hpp>
#include <serdepp/ostream.hpp>

#include <serdepp/attributes.hpp>
#include <serdepp/utility.hpp>
#include <memory>

using namespace serde::ostream;

enum class tenum {
    INPUT    ,
    OUTPUT   ,
    INPUT_2  ,
    OUTPUT_2 ,
};


struct nested {
    DERIVE_SERDE(nested,
            (&Self::version, "version", value_or_struct)
            (&Self::opt_desc ,"opt_desc")
            (&Self::desc ,"desc", default_{"default value"})
            .no_remain())
    std::string version;
    std::string desc;
    std::optional<std::string> opt_desc;
};

class test {
public:
    DERIVE_SERDE(test,
            (&Self::str, "str", default_{"hello"})
            (&Self::i,   "i")
            (&Self::vec, "vec")
            (&Self::io,  "io", default_{tenum::OUTPUT}, to_lower, under_to_dash)
            (&Self::in,  "in", make_optional)
            (&Self::pri, "pri", to_upper, under_to_dash)
            (&Self::m ,  "m")
            (&Self::nm , "nm"))
    std::optional<std::string> str;
    int i;
    std::optional<std::vector<std::string>> vec;
    tenum io;
    std::vector<nested> in;
    std::map<std::string, std::string> m;
    std::map<std::string, nested> nm;
    std::string pri;
};

int main()
{
    nlohmann::json v = R"({
    "i": 10,
    "vec": [ "one", "two", "three" ],
    "io" : "OUTPUT-2",
    "pri" : "PRi-FF",
    "in" : [{ "version" : "hello" }, "single"],
    "m" : { "a" : "1",
            "b" : "2",
            "c" : "3" },
    "nm" : { "a" : {"version" : "hello" },
            "b" : "hello2" }
    })"_json;

    // try {

    test t = serde::serialize<test>(v);
    fmt::print("{}\n",serde::to_str(t.io));

    YAML::Node y = serde::deserialize<YAML::Node>(10);
    //YAML::Node y;
    //y = 10;
    std::cout << y << "\n";
    serde::serialize<int>(y);


    auto v_to_json = serde::deserialize<nlohmann::json>(t);
    auto v_to_toml = serde::deserialize<serde::toml_v>(t);
    auto v_to_yaml = serde::deserialize<serde::yaml>(t);

    std::cout << "toml: " << v_to_toml << std::endl;
    fmt::print("json: {}\n", v_to_json.dump());
    std::cout << "yaml: " << v_to_yaml << std::endl;

    test t_from_toml = serde::serialize<test>(v_to_toml);
    test t_from_yaml = serde::serialize<test>(v_to_yaml);

    fmt::print("{}\n", t_from_toml);
    fmt::print("{}\n", t_from_yaml);
    std::cout << t << '\n';

    //}catch(std::exception& e) {
    //    fmt::print(stderr,"{}\n",e.what());
    //}

    return 0;
}
