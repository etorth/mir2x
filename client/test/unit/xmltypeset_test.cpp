// XMLTypeset geometry regression suite.
//
// XMLTypeset (client/src/xmltypeset.cpp/.hpp) lays out XML-tagged rich text into lines of
// tokens, supporting incremental edits (insert/delete/split/join/resize) as well as
// LALIGN_LEFT/RIGHT/CENTER/JUSTIFY/DISTRIBUTED alignment. it is complicated enough, and
// has been broken by "small" changes often enough, that any change to it should be run
// against this suite before being committed.
//
// build & run:
//   cmake --build <builddir> --target test_xmltypeset
//   <builddir>/client/test/unit/test_xmltypeset [res_dir]
//
// or, to build and run every registered test project-wide via CTest:
//   cmake --build <builddir> --target check          # builds tests, then runs ctest
//   ctest --test-dir <builddir> -R xmltypeset         # rerun just this one (after building)
//
// res_dir defaults to MIR2X_TEST_DEFAULT_RES_DIR (${CMAKE_INSTALL_PREFIX}/client/res at
// configure time) and must contain font/fontex.zsdb and emoji/emoji.zsdb, i.e. the client
// must have been installed at least once (`cmake --install <builddir>`) before this runs.
//
// exit code is 0 iff every case below passes; on failure the offending case (and its full
// InitArgs/xml) is printed to stderr before the process exits non-zero. stdout is kept
// deterministic (see the std::cout suppression around Log's construction in main(), below)
// so cmake/run_test.py can diff it against xmltypeset_test.log.gold.

#include <cstdio>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include "clientargparser.hpp"
#include "emojidb.hpp"
#include "fontexdb.hpp"
#include "log.hpp"
#include "sdldevice.hpp"
#include "totype.hpp"
#include "utf8f.hpp"
#include "xmltypeset.hpp"

// client_objs (everything the client is built from, minus main.cpp) references these
// globals from many unrelated widgets (see main.cpp for the authoritative list), so they
// must be defined somewhere in this link unit even though this test only ever assigns
// the handful XMLTypeset/FontexDB/EmojiDB actually touch; the rest stay nullptr, which is
// fine as long as no code path exercised by this test dereferences them
class PNGTexDB;
class PNGTexOffDB;
class MapBinDB;
class BGMusicDB;
class SoundEffectDB;
class IMEBoard;
class MessageStackBoard;
class Client;

ClientArgParser   *g_clientArgParser = nullptr;
Log               *g_mir2xLog        = nullptr;
PNGTexDB          *g_progUseDB       = nullptr;
PNGTexDB          *g_itemDB          = nullptr;
PNGTexDB          *g_mapDB           = nullptr;
PNGTexOffDB       *g_heroDB          = nullptr;
PNGTexOffDB       *g_hairDB          = nullptr;
PNGTexOffDB       *g_monsterDB       = nullptr;
PNGTexOffDB       *g_weaponDB        = nullptr;
PNGTexOffDB       *g_helmetDB        = nullptr;
PNGTexOffDB       *g_equipDB         = nullptr;
PNGTexOffDB       *g_magicDB         = nullptr;
PNGTexOffDB       *g_standNPCDB      = nullptr;
PNGTexOffDB       *g_selectCharDB    = nullptr;
EmojiDB           *g_emojiDB         = nullptr;
BGMusicDB         *g_bgmDB           = nullptr;
SoundEffectDB     *g_seffDB          = nullptr;
MapBinDB          *g_mapBinDB        = nullptr;
FontexDB          *g_fontexDB        = nullptr;
SDLDevice         *g_sdlDevice       = nullptr;
IMEBoard          *g_imeBoard        = nullptr;
MessageStackBoard *g_notifyBoard     = nullptr;
Client            *g_client          = nullptr;

namespace
{
    struct TestFonts: FontexDB
    {
        TestFonts(): FontexDB(1024 * 1024 * 64) {}
        ~TestFonts() override { clear(); }
    };

    struct TestEmoji: EmojiDB
    {
        ~TestEmoji() override { clear(); }
    };

    // a real captured NPC dialog paragraph (server/script/quest/野蛮冲撞任务.lua,
    // npc_ask_magic), kept verbatim so the suite exercises real CJK content with
    // embedded <t color="..."> tags, not just synthetic ASCII fixtures
    const std::string capture =
        "<par>嗯，你好像在实战中也有些体会。虽然战士总是想在最前方战斗，但是没有这种"
        "<t color=\"red\">护身术</t>。魔法师可以利用瞬息移动魔法消失掉，道士也可以利用"
        "隐身术隐藏起自己的行踪，我们只有将敌人打倒后才可以脱身。如果被层层包围，真是"
        "死路一条。我也是经历了无数的生死考验，真是为了解决战士的困难才创造了"
        "<t color=\"red\">野蛮冲撞</t>。</par>";

    void require(bool condition, const std::string &message)
    {
        if(!condition){
            throw std::runtime_error(message);
        }
    }

    // replays the same padding/wrapping/justify math XMLTypeset itself uses, then checks
    // the resulting token boxes against it: this catches both wrapping and alignment bugs
    // (wrong W1/W2, wrong line width, wrong per-line X offset, wrong board width)
    void checkGeometry(const XMLTypeset &typeset, const XMLTypeset::InitArgs &args, bool allowWideLeaf = false)
    {
        XMLParagraph paragraph;
        paragraph.loadXMLNode(typeset.getXMLNode());
        const int align = args.lineWidth == 0 ? LALIGN_LEFT : args.lineAlign;
        const int wordSpace = args.wordSpace;
        const int target = args.lineWidth + args.lineMargin[0] + args.lineMargin[1];
        std::vector<int> widths;
        for(int y = 0; y < typeset.lineCount(); ++y){
            const int count = typeset.lineTokenCount(y);
            int natural = 0;
            int logical = 0;
            for(int x = 0; x < count; ++x){
                const auto token = typeset.getToken(x, y);
                int left = 0;
                int right = 0;
                if(paragraph.leaf(token->leaf).type() == LEAF_UTF8STR){
                    auto key = token->utf8char.key;
                    if(args.codeXfer){
                        key = utf8f::exchangeCodePointInU64Key(key, args.codeXfer(utf8f::codePointFromU64Key(key)));
                    }
                    require(g_fontexDB->retrieve(key, &left, &right) != nullptr, "missing font fixture");
                }
                const int w1 = x == 0 ? args.lineMargin[0] : left + wordSpace / 2;
                const int w2 = x + 1 == count ? args.lineMargin[1] : right + (wordSpace + 1) / 2;
                natural += w1 + token->box.info.w + w2;
                logical += token->box.width();
                require(token->box.state.w1 >= w1 && token->box.state.w2 >= w2, "natural padding was reduced");
                if(x == 0){
                    require(token->box.state.w1 == args.lineMargin[0], "first W1 margin changed");
                }
                if(x + 1 == count){
                    require(token->box.state.w2 == args.lineMargin[1], "last W2 margin changed");
                }
                else{
                    const auto next = typeset.getToken(x + 1, y);
                    require(token->box.state.x + token->box.info.w + token->box.state.w2
                            == next->box.state.x - next->box.state.w1, "non-contiguous logical token boxes");
                }
            }
            const auto context = "line " + std::to_string(y) + " target=" + std::to_string(target)
                + " natural=" + std::to_string(natural) + " logical=" + std::to_string(logical);
            if(args.lineWidth > 0 && count > 1 && !allowWideLeaf){
                require(natural <= target, "wrapping overflow: " + context);
            }
            const bool justify = (align == LALIGN_DISTRIBUTED || (align == LALIGN_JUSTIFY && y + 1 < typeset.lineCount()))
                && count > 1 && natural < target;
            require(logical == (justify ? target : natural), "wrong line width: " + context);
            widths.push_back(logical);
        }

        // ref: the alignment baseline used to compute each line's X offset.
        // LALIGN_RIGHT/LALIGN_CENTER: a huge unbreakable leaf can widen this beyond target
        // (lines above are shifted to match), while LALIGN_DISTRIBUTED intentionally keeps
        // using target even when a line overflows it (no lines are re-anchored)
        int ref = target;
        if(align == LALIGN_RIGHT || align == LALIGN_CENTER){
            for(const int logical: widths){
                ref = std::max(ref, logical);
            }
        }

        // resetBoardPixelRegion() only measures already-arranged tokens: it does not
        // guarantee reserving the full alignment frame when no line's content reaches it
        int expectedFW = 0;
        for(int y = 0; y < typeset.lineCount(); ++y){
            const int logical = widths.at(y);
            const bool center = align == LALIGN_CENTER || (align == LALIGN_DISTRIBUTED && typeset.lineTokenCount(y) == 1);
            const int offset = align == LALIGN_RIGHT ? std::max(0, ref - logical)
                : center ? std::max(0, (ref - logical) / 2) : 0;
            expectedFW = std::max(expectedFW, offset + logical);
        }
        require(typeset.fw() == expectedFW, "wrong board width: align=" + std::to_string(align)
                + " expected=" + std::to_string(expectedFW) + " actual=" + std::to_string(typeset.fw()));

        for(int y = 0; y < typeset.lineCount(); ++y){
            const int logical = widths.at(y);
            const bool center = align == LALIGN_CENTER || (align == LALIGN_DISTRIBUTED && typeset.lineTokenCount(y) == 1);
            const int offset = align == LALIGN_RIGHT ? std::max(0, ref - logical)
                : center ? std::max(0, (ref - logical) / 2) : 0;
            require(typeset.getToken(0, y)->box.state.x == offset + args.lineMargin[0], "wrong alignment offset");
            if(center && logical <= ref){
                // room is measured against ref (the alignment baseline), not fw(): fw() is a tight
                // measurement of arranged tokens and may be smaller than ref when no line's content
                // reaches it, so comparing room against fw() would be wrong for shorter lines.
                // skip the check when logical > ref: an unbreakable leaf/single token can overflow
                // ref (e.g. LALIGN_DISTRIBUTED never widens for a single-token line), and there's
                // no "room" left to balance in that case (matches the intentional "ugly" overflow
                // behavior for LALIGN_DISTRIBUTED)
                const int leftRoom = typeset.getToken(0, y)->box.state.x - args.lineMargin[0];
                const int rightRoom = ref - leftRoom - logical;
                require(std::abs(leftRoom - rightRoom) <= 1, "unequal center alignment room");
            }
            if(align == LALIGN_RIGHT){
                const auto last = typeset.getLineBackToken(y);
                require(last->box.state.x + last->box.info.w + last->box.state.w2 == typeset.fw(), "line does not reach board's right edge");
            }
        }
    }

    // a flattened snapshot of every geometric detail XMLTypeset tracks: used to compare an
    // incrementally-edited typeset against a from-scratch rebuild of the same final text
    std::vector<int> geometry(const XMLTypeset &typeset)
    {
        std::vector<int> result{typeset.lineCount(), typeset.fw(), typeset.fh(), typeset.px(), typeset.py(), typeset.pw(), typeset.ph()};
        for(int y = 0; y < typeset.lineCount(); ++y){
            result.push_back(typeset.lineTokenCount(y));
            result.push_back(typeset.lineStartY(y));
            for(int x = 0; x < typeset.lineTokenCount(y); ++x){
                const auto t = typeset.getToken(x, y);
                result.insert(result.end(), {t->leaf, t->box.info.w, t->box.info.h,
                        t->box.state.x, t->box.state.y, t->box.state.w1, t->box.state.w2,
                        t->box.state.h1, t->box.state.h2});
            }
        }
        return result;
    }

    void checkFresh(const XMLTypeset &typeset, const XMLTypeset::InitArgs &args, bool allowWideLeaf = false)
    {
        checkGeometry(typeset, args, allowWideLeaf);
        XMLTypeset fresh(args);
        fresh.loadXMLNode(typeset.getXMLNode());
        require(geometry(typeset) == geometry(fresh), "incremental layout differs from a fresh rebuild");
    }

    void runTests()
    {
        int cases = 0;
        const auto check = [&cases](const XMLTypeset::InitArgs &args, const std::string &xml, bool allowWideLeaf = false)
        {
            try{
                XMLTypeset typeset(args);
                typeset.loadXML(xml.c_str());
                checkGeometry(typeset, args, allowWideLeaf);
                cases++;
            }
            catch(...){
                std::fprintf(stderr, "failed at case %d: lineWidth=%d align=%d wordSpace=%d margin={%d,%d} xml=%s\n",
                        cases, args.lineWidth, args.lineAlign, args.wordSpace, args.lineMargin[0], args.lineMargin[1], xml.c_str());
                throw;
            }
        };

        for(const int width: {354, 355, 356}){
            check({.lineWidth = width, .lineAlign = LALIGN_JUSTIFY, .canThrough = false, .font = Widget::FontConfig{}}, capture);
        }
        for(const int align: {LALIGN_LEFT, LALIGN_RIGHT, LALIGN_CENTER, LALIGN_JUSTIFY, LALIGN_DISTRIBUTED}){
            for(const int font: {0, 11}){
                for(const int width: {48, 85, 170, 355}){
                    for(const int spacing: {0, 1, 4, 7}){
                        XMLTypeset::InitArgs args
                        {
                            .lineWidth = width,
                            .lineAlign = align,
                            .canThrough = false,
                            .font{.id = to_u8(font), .size = 15},
                            .wordSpace = spacing,
                            .lineMargin{3, 7},
                        };
                        check(args, capture);
                        check(args, "<par>Iii WWW abc, def.<t font=\"0\" size=\"18\" color=\"red\">X Y</t>  More text to wrap!</par>");
                    }
                }
            }
        }

        XMLTypeset::InitArgs args
        {
            .lineWidth = 80,
            .lineAlign = LALIGN_JUSTIFY,
            .canThrough = false,
            .font = Widget::FontConfig{},
            .wordSpace = 1,
            .lineMargin{2, 5},
        };
        check(args, "<par/>");
        check(args, "<par>A</par>");
        check(args, "<par>AB</par>");
        check(args, "<par>A <emoji id=\"0\"/> <emoji id=\"1\"/> B <emoji id=\"2\"/> tail</par>");
        check(args, "<par><emoji id=\"0\"/><emoji id=\"1\"/><emoji id=\"2\"/><emoji id=\"3\"/></par>");
        check(args, "<par><event id=\"a\" wrap=\"false\">one</event><event id=\"b\" wrap=\"false\">two</event> more words that wrap</par>");
        check(args, "<par><event id=\"wide\" wrap=\"false\">ABCDEFGHIJKLMNOPQRSTUVWXYZ</event> tail</par>", true);

        XMLTypeset two({.font = Widget::FontConfig{}});
        two.loadXML("<par>AA</par>");
        XMLTypeset::InitArgs tight
        {
            .lineWidth = two.fw() + 1,
            .lineAlign = LALIGN_JUSTIFY,
            .font = Widget::FontConfig{},
        };
        XMLTypeset shortLines(tight);
        shortLines.loadXML("<par>AAA</par>");
        require(shortLines.lineCount() == 2 && shortLines.lineTokenCount(0) == 2 && shortLines.lineTokenCount(1) == 1, "two-token fixture did not wrap");
        checkGeometry(shortLines, tight);
        cases++;

        args.lineWidth = 1;
        check(args, "<par>AB</par>");
        check(args, "<par><emoji id=\"0\"/><emoji id=\"1\"/></par>");

        args.lineWidth = 0;
        check(args, capture);
        args.lineWidth = 90;
        XMLTypeset edited(args);
        edited.loadXML(capture.c_str());
        checkFresh(edited, args);
        edited.insertUTF8String(2, 1, "XYZ");
        checkFresh(edited, args);
        edited.deleteToken(1, 1, 2);
        checkFresh(edited, args);
        const auto text = edited.getText();
        auto prefix = std::unique_ptr<XMLTypeset>(edited.split(3, 1));
        checkFresh(*prefix, args);
        checkFresh(edited, args);
        prefix->join(edited, true);
        require(prefix->getText() == text, "split/join lost text");
        checkFresh(*prefix, args);
        for(const int width: {125, 55, 0, 355, 90}){
            args.lineWidth = width;
            prefix->setLineWidth(width, args.lineMargin);
            checkFresh(*prefix, args);
            require(prefix->getText() == text, "resize changed text");
            cases++;
        }
        args.codeXfer = [](uint32_t){ return to_u32('*'); };
        prefix->setCodeXferFunc(args.codeXfer);
        prefix->updateGfx();
        checkFresh(*prefix, args);
        cases += 7;

        for(const int align: {LALIGN_LEFT, LALIGN_RIGHT, LALIGN_CENTER, LALIGN_JUSTIFY, LALIGN_DISTRIBUTED}){
            XMLTypeset::InitArgs edge
            {
                .lineWidth = 81,
                .lineAlign = align,
                .canThrough = true,
                .compactLine = true,
                .font = Widget::FontConfig{},
                .wordSpace = 3,
                .lineMargin{2, 6},
            };
            for(const char *xml: {
                    "<par/>", "<par>A</par>", "<par>AB</par>",
                    "<par><emoji id=\"0\"/></par>",
                    "<par>A <emoji id=\"0\"/><emoji id=\"1\"/> B <emoji id=\"2\"/> tail</par>",
                    "<par><emoji id=\"0\"/><emoji id=\"1\"/><emoji id=\"2\"/><emoji id=\"3\"/></par>",
                    "<par><event id=\"a\" wrap=\"false\">one</event><event id=\"b\" wrap=\"false\">two</event> more words to wrap</par>"}){
                check(edge, xml);
            }
            check(edge, "<par><event id=\"wide\" wrap=\"false\">ABCDEFGHIJKLMNOPQRSTUVWXYZ</event> tail</par>", true);

            XMLTypeset empty(edge);
            require(empty.fw() == 0, "wrong initial empty width");
            empty.loadXML("<par>A</par>");
            empty.clear();
            require(empty.empty(), "clear did not empty typeset");
            require(empty.fw() == 0, "clear lost the alignment frame");
            edge.lineWidth = 105;
            empty.setLineWidth(edge.lineWidth, edge.lineMargin);
            require(empty.fw() == 0, "resizing empty alignment frame failed");
            empty.loadXML("<par/>");
            checkFresh(empty, edge);
            cases++;

            XMLTypeset edit(edge);
            edit.loadXML(capture.c_str());
            edit.insertUTF8String(2, 1, "XYZ");
            checkFresh(edit, edge);
            edit.deleteToken(1, 1, 2);
            checkFresh(edit, edge);
            auto first = std::unique_ptr<XMLTypeset>(edit.split(2, 1));
            checkFresh(*first, edge);
            checkFresh(edit, edge);
            first->join(edit, true);
            checkFresh(*first, edge);
            for(const int width: {55, 125, 0, 105}){
                edge.lineWidth = width;
                first->setLineWidth(width, edge.lineMargin);
                checkFresh(*first, edge);
                cases++;
            }

            XMLTypeset overwide(edge);
            overwide.loadXML("<par><event wrap=\"false\">ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ</event></par>");
            first->join(overwide, true);
            checkFresh(*first, edge, true);
            first->deleteToken(0, first->lineCount() - 1, first->lineTokenCount(first->lineCount() - 1));
            checkFresh(*first, edge);
            cases += 7;

            edge.lineWidth = tight.lineWidth;
            edge.wordSpace = 0;
            check(edge, "<par>AAA</par>");
            edge.lineWidth = 1;
            check(edge, "<par>AB</par>");
            edge.lineWidth = 0;
            check(edge, capture);
        }

        std::printf("All %d geometry cases passed (including retained tokens, edits, resize, split/join, masks, margins, emoji, and oversized leaves).\n", cases);
    }
}

int main(int argc, char **argv)
{
    try{
        const std::string resDir = argc > 1 ? argv[1] : MIR2X_TEST_DEFAULT_RES_DIR;

        char name[] = "xmltypeset_test";
        char audio[] = "--disable-audio";
        char profiler[] = "--disable-profiler";
        char *options[] = {name, audio, profiler};
        const argf::parser parser(3, options);
        ClientArgParser clientArgs(parser);
        g_clientArgParser = &clientArgs;

        logDisableProfiler();

        // Log's constructor unconditionally prints an init banner to std::cout that embeds
        // a pid and a timestamped log file path - neither is reproducible across runs, so
        // it can never match a checked-in gold file. suppress std::cout for the duration of
        // construction only (this doesn't touch the process's real stdout fd, so the
        // std::printf-based results the tests print later via runTests() are unaffected);
        // g3log's own shutdown banner goes to std::cerr, not stdout, so no such suppression
        // is needed for that.
        std::ostringstream logBannerDiscard;
        auto * const savedCoutBuf = std::cout.rdbuf(logBannerDiscard.rdbuf());
        Log log("mir2x-xmltypeset-test");
        std::cout.rdbuf(savedCoutBuf);
        g_mir2xLog = &log;

        SDLDevice device;
        g_sdlDevice = &device;
        device.createMainWindow();

        TestFonts fonts;
        g_fontexDB = &fonts;
        fonts.load((resDir + "/font/fontex.zsdb").c_str());

        TestEmoji emoji;
        g_emojiDB = &emoji;
        emoji.load((resDir + "/emoji/emoji.zsdb").c_str());

        runTests();
        return 0;
    }
    catch(const std::exception &e){
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
