#pragma once
#include "serdesmsg.hpp"

SDAcutionItemList dbQueryAcutionItemList(int);
bool dbRegisterAcutionItem(uint32_t, const SDItem &, const std::string &, uint64_t);
