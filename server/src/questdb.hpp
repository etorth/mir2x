#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <utility>
#include <optional>
#include "luaf.hpp"

// one table per quest, one row per player, see Quest::LuaThreadRunner
// a field holds a serialized luaf::luaVar, or null

void dbCreateQuestTable(const std::string &);
std::optional<luaf::luaVar> dbLoadQuestField(const std::string &, uint32_t, const std::string &);

// writes the given fields of the row in one statement, so a crash keeps all of them or none, a field without value is set to null
// replace: the row is written whole, the fields not given are set to null too
void dbUpdateQuestFields(const std::string &, uint32_t, const std::vector<std::pair<std::string, std::optional<luaf::luaVar>>> &, bool);

// the field list of a lua table {fld_xxx = value, ...}, a nil can't be a table value, SYS_LUANIL stands for it
std::vector<std::pair<std::string, std::optional<luaf::luaVar>>> buildQuestFieldList(const sol::table &);
