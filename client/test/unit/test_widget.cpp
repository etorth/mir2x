#include <catch2/catch_test_macros.hpp>
#include "widget.hpp"

TEST_CASE("childID tracks live sibling order", "[widget]")
{
    Widget parent {{}};
    Widget first {{.parent{&parent}}};
    Widget second {{.parent{&parent}}};
    Widget third {{.parent{&parent}}};

    REQUIRE_FALSE(parent.childID().has_value());
    REQUIRE(first.childID() == 0);
    REQUIRE(second.childID() == 1);
    REQUIRE(third.childID() == 2);

    parent.moveFront(&third);
    REQUIRE(third.childID() == 0);
    REQUIRE(first.childID() == 1);
    REQUIRE(second.childID() == 2);

    parent.moveBack(&third);
    REQUIRE(first.childID() == 0);
    REQUIRE(second.childID() == 1);
    REQUIRE(third.childID() == 2);

    parent.removeChild(first.id(), false);
    REQUIRE_FALSE(first.childID().has_value());
    REQUIRE(second.childID() == 0);
    REQUIRE(third.childID() == 1);

    parent.purge();
    REQUIRE(second.childID() == 0);
    REQUIRE(third.childID() == 1);
}

TEST_CASE("childID follows reparenting and sorting", "[widget]")
{
    Widget parent {{}};
    Widget otherParent {{}};
    Widget first {{.parent{&parent}}};
    Widget second {{.parent{&parent}}};
    Widget third {{.parent{&otherParent}}};

    otherParent.addChild(&first, false);
    REQUIRE(second.childID() == 0);
    REQUIRE(third.childID() == 0);
    REQUIRE(first.childID() == 1);

    otherParent.sort([](const Widget *lhs, bool, const Widget *rhs, bool)
    {
        return lhs->id() < rhs->id();
    });
    REQUIRE(first.childID() == 0);
    REQUIRE(third.childID() == 1);

    otherParent.clearChild();
    REQUIRE_FALSE(first.childID().has_value());
    REQUIRE_FALSE(third.childID().has_value());
}
