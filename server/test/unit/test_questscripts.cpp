#include <catch2/catch_test_macros.hpp>
#include <sol/sol.hpp>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    void runLuaSuite(const char *path, const std::vector<std::string> &arguments)
    {
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine,
                           sol::lib::string, sol::lib::math, sol::lib::table,
                           sol::lib::io, sol::lib::os, sol::lib::debug, sol::lib::utf8);
        const std::string root = MIR2X_TEST_SOURCE_DIR;
        auto arg = lua.create_table();
        for(size_t i = 0; i < arguments.size(); ++i){
            arg[i + 1] = arguments[i];
        }
        lua["arg"] = arg;
        const auto result = lua.safe_script_file(
            root + "/" + path,
            sol::script_pass_on_error);
        if(!result.valid()){
            const sol::error error = result;
            FAIL(error.what());
        }
        REQUIRE(result.valid());
    }
}

TEST_CASE("Introductory and side quests", "[questscripts]")
{
    runLuaSuite("server/test/unit/test_questscripts.lua", {MIR2X_TEST_SOURCE_DIR});
}

TEST_CASE("Warrior and wizard quests", "[magicquests]")
{
    runLuaSuite("server/test/unit/test_magicquests.lua", {MIR2X_TEST_SOURCE_DIR});
}

TEST_CASE("Taoist quests", "[taoistquests]")
{
    runLuaSuite("server/test/unit/test_taoistquests.lua", {MIR2X_TEST_SOURCE_DIR});
}

TEST_CASE("Story quests", "[worldquests]")
{
    runLuaSuite("server/test/unit/test_worldquests.lua", {MIR2X_TEST_SOURCE_DIR});
}

TEST_CASE("Merchant contracts and NPC entry paths", "[merchants]")
{
    std::vector<std::string> scripts;
    for(const auto &entry: std::filesystem::directory_iterator(
            std::filesystem::path(MIR2X_TEST_SOURCE_DIR) / "server/script/npc")){
        if(entry.is_regular_file() && entry.path().extension() == ".lua"){
            scripts.push_back(entry.path().string());
        }
    }
    std::ranges::sort(scripts);
    REQUIRE_FALSE(scripts.empty());
    runLuaSuite("server/test/merchant.lua", scripts);
}
