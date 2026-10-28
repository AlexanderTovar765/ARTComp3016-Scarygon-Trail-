// Logic tests for The Scarygon Trail. They cover everything that does not need a window:
// the party and its members, the Observer pattern, the narrator (State pattern), the event
// loader (including deliberately broken files), settings, the playtest log, audio fallbacks,
// and whether every piece of text fits its panel. See docs/test-cases.md for the test list.
//
// Run from the repo root:   logic_tests            (or:  logic_tests path/to/repo/root)
// Exit code 0 = all passed.

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Animations.h"
#include "AudioManager.h"
#include "Difficulty.h"
#include "Effects.h"
#include "LieTracker.h"
#include "MemberWatcher.h"
#include "NarrativeState.h"
#include "Party.h"
#include "PlaytestLog.h"
#include "Progress.h"
#include "Settings.h"
#include "TextRenderer.h"
#include "ThresholdWatcher.h"
#include "Trail.h"

namespace fs = std::filesystem;

static int g_checks = 0;
static int g_failed = 0;
static std::string g_test;

#define CHECK(condition)                                                                         \
    do {                                                                                         \
        ++g_checks;                                                                              \
        if (!(condition)) {                                                                      \
            ++g_failed;                                                                          \
            std::cerr << "  FAIL [" << g_test << "] line " << __LINE__ << ": " #condition "\n";  \
        }                                                                                        \
    } while (0)

static void begin(const char* name) {
    g_test = name;
    std::cout << "- " << name << "\n";
}

static bool isAscii(const std::string& s) {
    for (unsigned char c : s) if (c >= 0x80) return false;
    return true;
}

static std::string readAll(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

static void writeFile(const fs::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
}

// ---------------------------------------------------------------------------

static void testEffectsAndParsing() {
    begin("T01 Effects::describe");
    Effects e;
    CHECK(e.isZero() && e.describe() == "no change");
    e.food = -5; e.sanity = 3;
    CHECK(e.describe() == "Food -5, Sanity +3");
    e.health = -12; e.member = "Pa";
    CHECK(e.describe() == "Food -5, Pa's health -12, Sanity +3");

    begin("T02 parsePart / parseCondition accept sensible spellings and reject nonsense");
    TimePart p;
    CHECK(parsePart("Dawn", p) && p == TimePart::Dawn);
    CHECK(parsePart("mid-morning", p) && p == TimePart::MidMorning);
    CHECK(parsePart("  MID MORNING ", p) && p == TimePart::MidMorning);
    CHECK(parsePart("night", p) && p == TimePart::Night);
    CHECK(!parsePart("midday", p) && !parsePart("", p));
    Condition c;
    CHECK(parseCondition("LowFood", c) && c == Condition::LowFood);
    CHECK(parseCondition("low wagon", c) && c == Condition::LowWagon);
    CHECK(parseCondition("LOWHEALTH", c) && c == Condition::LowHealth);
    CHECK(!parseCondition("lowmoney", c) && !parseCondition("", c));
    ConditionSet set;
    CHECK(!set.any());
    set.set(Condition::LowFood);
    CHECK(set.has(Condition::LowFood) && !set.has(Condition::LowWagon) && set.any());
    set.set(Condition::LowFood, false);
    CHECK(!set.any());
}

static void testMembers() {
    begin("T03 PartyMember: frailty, healing cap, permanent death, status thresholds");
    PartyMember normal("Test", 100), frail("Frail", 150), tough("Tough", 90);
    normal.changeHealth(-10); frail.changeHealth(-10); tough.changeHealth(-10);
    CHECK(normal.getHealth() == 90 && frail.getHealth() == 85 && tough.getHealth() == 91);  // damage rounds up
    normal.changeHealth(+50);
    CHECK(normal.getHealth() == 100);                                                          // healing is capped
    CHECK(normal.getStatus() == PartyMember::Status::Healthy);
    normal.changeHealth(-65);
    CHECK(normal.getHealth() == 35 && normal.getStatus() == PartyMember::Status::Ill);         // below 40 = ill
    normal.changeHealth(-500);
    CHECK(normal.getHealth() == 0 && normal.getStatus() == PartyMember::Status::Dead && !normal.isAlive());
    normal.changeHealth(+50);
    CHECK(normal.getHealth() == 0);                                                            // the dead stay dead
    normal.reset();
    CHECK(normal.getHealth() == 100);
    CHECK(findMemberIndex("granny") == 3 && findMemberIndex("MA") == 0 && findMemberIndex("nobody") == -1);
}

static void testPartyRules() {
    begin("T04 Party starts full; Health is the average of the members (dead count as 0)");
    Party party;
    CHECK(party.getFood() == 60 && party.getSanity() == 100 && party.getWagon() == 100);
    CHECK(party.getHealth() == 100 && party.aliveCount() == 4 && party.getMembers().size() == 4);
    Effects big; big.health = -500; big.member = "Granny";
    party.apply(big);
    CHECK(party.aliveCount() == 3 && party.getHealth() == 75);

    begin("T05 advanceTime: the trail's toll, and fewer mouths eat less");
    Party fresh;
    Effects toll = fresh.advanceTime();
    CHECK(toll.food == -3 && toll.sanity == -4 && toll.wagon == -2 && toll.health == -4);      // full party
    CHECK(fresh.getFood() == 57 && fresh.getSanity() == 96 && fresh.getWagon() == 98);
    Party two; Effects kill; kill.health = -500;
    kill.member = "Ma"; two.apply(kill); kill.member = "Junior"; two.apply(kill);
    Party probe = two;                                   // 2 alive -> the food toll is 2, not 3
    const int before = probe.getFood();
    probe.advanceTime();
    CHECK(before - probe.getFood() == 2);

    begin("T06 Starvation: at 0 food every living member takes damage each part");
    Party hungry; Effects spend; spend.food = -58;       // food 2
    hungry.apply(spend);
    const int healthBefore = hungry.getHealth();
    hungry.advanceTime();                                // eats 2 -> 0 food -> starving
    CHECK(hungry.getFood() == 0);
    CHECK(healthBefore - hungry.getHealth() > hungry.rules().hardshipPerPart);   // more than hardship alone
    const int again = hungry.getHealth();
    hungry.advanceTime();
    CHECK(hungry.getFood() == 0 && hungry.getHealth() < again);

    begin("T07 Targeted effects hit one member; untargeted hit everyone alive; unknown name is ignored");
    Party t; Effects hit; hit.health = -10; hit.member = "Pa";
    t.apply(hit);
    CHECK(t.getMembers()[1].getHealth() == 91 && t.getMembers()[0].getHealth() == 100);   // Pa is tough (90%)
    Effects all; all.health = -10;
    Party u; u.apply(all);
    CHECK(u.getMembers()[0].getHealth() == 90 && u.getMembers()[3].getHealth() == 85);    // Granny 150%
    Effects bogus; bogus.health = -50; bogus.member = "Nobody";
    Party v; v.apply(bogus);
    CHECK(v.getHealth() == 100);

    begin("T08 Grief: each death costs the survivors sanity");
    Party g; Effects death; death.health = -500; death.member = "Granny";
    g.apply(death);
    CHECK(g.getSanity() == 100 - Party::kGriefSanity);
    g.apply(death);                                                                       // already dead: no second penalty
    CHECK(g.getSanity() == 100 - Party::kGriefSanity);

    begin("T09 Game-over conditions and cause of death");
    Party alive; CHECK(alive.isAlive() && alive.causeOfDeath().empty());
    Party mad; Effects s; s.sanity = -100; mad.apply(s);
    CHECK(!mad.isAlive() && mad.causeOfDeath().find("sanity") != std::string::npos);
    Party wreck; Effects w; w.wagon = -100; wreck.apply(w);
    CHECK(!wreck.isAlive() && wreck.causeOfDeath().find("wagon") != std::string::npos);
    Party dead; Effects d; d.health = -500; dead.apply(d);
    CHECK(!dead.isAlive() && dead.aliveCount() == 0 && dead.causeOfDeath().find("died") != std::string::npos);
    Party clamp; Effects up; up.food = 999; up.sanity = 999; up.wagon = 999; clamp.apply(up);
    CHECK(clamp.getFood() == 100 && clamp.getSanity() == 100 && clamp.getWagon() == 100);
}

static void testObservers() {
    begin("T10 ThresholdWatcher: flags track low stats; warnings only on a drop INTO danger");
    Party party; ThresholdWatcher watcher; party.addObserver(&watcher);
    CHECK(!watcher.active().any() && watcher.takeWarnings().empty());
    Effects drop; drop.food = -40;                      // food 60 -> 20 (low is <= 25)
    party.apply(drop);
    CHECK(watcher.active().has(Condition::LowFood));
    std::vector<std::string> warnings = watcher.takeWarnings();
    CHECK(warnings.size() == 1 && warnings[0].find("food") != std::string::npos);
    CHECK(watcher.takeWarnings().empty());              // taken once, then gone
    Effects more; more.food = -5;                       // still low: no repeat warning
    party.apply(more);
    CHECK(watcher.active().has(Condition::LowFood) && watcher.takeWarnings().empty());
    Effects recover; recover.food = +50;                // back to safe: flag clears, no warning
    party.apply(recover);
    CHECK(!watcher.active().has(Condition::LowFood) && watcher.takeWarnings().empty());
    Effects wagon; wagon.wagon = -30;                   // wagon 100 -> 70 (low is <= 75)
    party.apply(wagon);
    CHECK(watcher.active().has(Condition::LowWagon) && watcher.takeWarnings().size() == 1);

    begin("T11 MemberWatcher: illness, recovery and death messages");
    Party p2; MemberWatcher members; p2.addObserver(&members);
    Effects sick; sick.health = -50; sick.member = "Junior";       // 100 -> 40: not ill yet (ill is < 40)
    p2.apply(sick);
    CHECK(members.takeMessages().empty());
    sick.health = -5;
    p2.apply(sick);
    std::vector<std::string> msgs = members.takeMessages();
    CHECK(msgs.size() == 1 && msgs[0] == "Junior has fallen ill.");
    Effects heal; heal.health = +30; heal.member = "Junior";
    p2.apply(heal);
    msgs = members.takeMessages();
    CHECK(msgs.size() == 1 && msgs[0] == "Junior is feeling better.");
    Effects kill; kill.health = -500; kill.member = "Granny";
    p2.apply(kill);
    CHECK(members.sawDeath());
    msgs = members.takeMessages();
    CHECK(msgs.size() == 1 && msgs[0].find("Granny has died") != std::string::npos);
    CHECK(!members.sawDeath());                                    // reset once taken

    begin("T12 Several observers, removal, and copies do not inherit observers");
    Party p3; ThresholdWatcher a, b; p3.addObserver(&a); p3.addObserver(&b); p3.addObserver(&a);  // duplicate add ignored
    Effects low; low.food = -45;
    p3.apply(low);
    CHECK(a.active().has(Condition::LowFood) && b.active().has(Condition::LowFood));
    CHECK(a.takeWarnings().size() == 1);                           // told once, not twice
    p3.removeObserver(&b);
    Effects wag; wag.wagon = -40;
    p3.apply(wag);
    CHECK(a.active().has(Condition::LowWagon) && !b.active().has(Condition::LowWagon));
    ThresholdWatcher third;
    Party p4; p4.addObserver(&third);
    Party copy = p4;                                               // scratch copy: must not notify
    Effects x; x.food = -50;
    copy.apply(x);
    CHECK(!third.active().any() && copy.getFood() == 10);
    Party assigned; assigned.addObserver(&third);
    assigned = copy;                                               // assignment is silent too
    CHECK(assigned.getFood() == 10 && !third.active().any());

    begin("T13 Party::reset notifies observers (so flags recover)");
    Party p5; ThresholdWatcher w5; p5.addObserver(&w5);
    Effects starve; starve.food = -50;
    p5.apply(starve);
    CHECK(w5.active().has(Condition::LowFood));
    w5.clearWarnings();
    p5.reset();
    CHECK(!w5.active().any() && w5.takeWarnings().empty());
}

static void testNarrator() {
    begin("T14 NarrativeState: thresholds, determinism, honest/lying behaviour");
    CHECK(NarrativeState::kindForSanity(100) == NarrativeKind::Stable);
    CHECK(NarrativeState::kindForSanity(60) == NarrativeKind::Stable);
    CHECK(NarrativeState::kindForSanity(59) == NarrativeKind::Unreliable);
    CHECK(NarrativeState::kindForSanity(30) == NarrativeKind::Unreliable);
    CHECK(NarrativeState::kindForSanity(29) == NarrativeKind::Breakdown);
    auto stable = NarrativeState::create(NarrativeKind::Stable);
    auto unrel  = NarrativeState::create(NarrativeKind::Unreliable);
    auto broken = NarrativeState::create(NarrativeKind::Breakdown);
    const std::string sample = "The family watched the oxen cross the river. The Wagon is fine, the stranger said. WAGON!";
    int changedU = 0, changedB = 0, asides = 0;
    for (unsigned seed = 0; seed < 200; ++seed) {
        CHECK(stable->filterText(sample, seed) == sample);
        CHECK(unrel->filterText(sample, seed) == unrel->filterText(sample, seed));     // no flicker
        CHECK(broken->filterText(sample, seed) == broken->filterText(sample, seed));
        CHECK(isAscii(unrel->filterText(sample, seed)) && isAscii(broken->filterText(sample, seed)));
        if (unrel->filterText(sample, seed) != sample) ++changedU;
        const std::string b = broken->filterText(sample, seed);
        if (b != sample) ++changedB;
        if (b.find('(') != std::string::npos) ++asides;
        for (int v : {0, 1, 40, 99, 100}) {
            for (Stat s : {Stat::Food, Stat::Health, Stat::Sanity, Stat::Wagon}) {
                CHECK(stable->shownValue(v, s, seed) == v);
                const int u = unrel->shownValue(v, s, seed);
                CHECK(u >= 0 && u <= 100 && std::abs(u - v) <= 10);
                if (s == Stat::Sanity) CHECK(u == v);                                  // honest while merely Unreliable
                const int bb = broken->shownValue(v, s, seed);
                CHECK(bb >= 0 && bb <= 100 && std::abs(bb - v) <= 30);
            }
        }
    }
    CHECK(changedU == 200 && changedB == 200 && asides == 200);                         // lies always happen
    CHECK(unrel->filterText("Nothing here can be swapped at all.", 3) == "Nothing here can be swapped at all.");
    CHECK(unrel->filterText("", 3).empty());
    CHECK(broken->filterText("", 5).find('(') != std::string::npos);                    // empty text does not crash
    CHECK(broken->valueLabel(55) == "??" && stable->valueLabel(55) == "55");
    bool upper = false, caps = false;
    for (unsigned seed = 0; seed < 200 && !(upper && caps); ++seed) {
        const std::string b = broken->filterText("Wagon and WAGON and wagon", seed);
        upper = upper || b.find("Hearse") != std::string::npos;
        caps  = caps  || b.find("HEARSE") != std::string::npos;
    }
    CHECK(upper && caps);                                                               // capitalisation preserved
    CHECK(stable->dread() == 0.0f && unrel->dread() > 0.0f && broken->dread() > unrel->dread());
}

// ---------------------------------------------------------------------------

static void testLieTracker() {
    begin("T24 LieTracker finds swapped words and added asides");
    LieTracker::TextLie lie;
    CHECK(!LieTracker::findTextLie("The wagon crosses the river.", "The wagon crosses the river.", lie));
    CHECK(!LieTracker::findTextLie("", "", lie));
    CHECK(LieTracker::findTextLie("The wagon crosses the river.", "The hearse crosses the river.", lie));
    CHECK(!lie.isAside && lie.truth == "wagon" && lie.shown == "hearse");
    CHECK(LieTracker::findTextLie("Your wagon, again.", "Your hearse, again.", lie));      // punctuation is ignored
    CHECK(lie.truth == "wagon" && lie.shown == "hearse");
    CHECK(LieTracker::findTextLie("Wagon first.", "Hearse first.", lie));                  // capitals are fine
    CHECK(lie.truth == "Wagon" && lie.shown == "Hearse");
    CHECK(LieTracker::findTextLie("All is well.", "All is well. (It is fine.)", lie));
    CHECK(lie.isAside && lie.shown == "(It is fine.)");
    CHECK(LieTracker::findTextLie("The ox eats.", "The critic eats. (They know.)", lie));  // a swap beats an aside as the example
    CHECK(!lie.isAside && lie.truth == "ox" && lie.shown == "critic");
    CHECK(LieTracker::findTextLie("The ox and the deer.", "The ox and the lawyer.", lie));
    CHECK(lie.truth == "deer" && lie.shown == "lawyer");                                   // the first real difference, not an equal word

    begin("T25 LieTracker counts, reports and resets");
    LieTracker tracker;
    CHECK(tracker.dayCount() == 0 && tracker.totalCount() == 0);
    CHECK(tracker.dayReport() == "Truth check: the narrator told the truth today.");
    CHECK(tracker.journeyReport().empty());
    tracker.recordText("The wagon rolls.", "The wagon rolls.");                            // not a lie
    tracker.recordBars({ { "Food", 40, 40 }, { "Health", 70, 70 } });                      // not a lie
    CHECK(tracker.totalCount() == 0);
    tracker.recordText("The wagon rolls.", "The hearse rolls.");
    CHECK(tracker.dayCount() == 1 && tracker.totalCount() == 1);
    CHECK(tracker.dayReport().find("once") != std::string::npos);
    CHECK(tracker.dayReport().find("\"hearse\" where the truth was \"wagon\"") != std::string::npos);
    tracker.recordBars({ { "Food", 47, 41 }, { "Health", 10, 99 } });                      // several wrong bars still count once
    CHECK(tracker.dayCount() == 2);
    CHECK(tracker.dayReport().find("2 times") != std::string::npos);
    CHECK(tracker.dayReport().find("Food bar showed 47 when the truth was 41") != std::string::npos);
    CHECK(tracker.dayReport().find("Health bar") == std::string::npos);                    // only the first example is given
    tracker.startDay();                                                                    // a new day: the day starts again, the journey does not
    CHECK(tracker.dayCount() == 0 && tracker.totalCount() == 2);
    CHECK(tracker.dayReport() == "Truth check: the narrator told the truth today.");
    CHECK(tracker.journeyReport() == "The narrator misled you 2 times.");
    tracker.recordText("All is well.", "All is well. (Nobody is coming.)");
    CHECK(tracker.dayReport().find("comments you never asked for") != std::string::npos);
    tracker.reset();
    CHECK(tracker.totalCount() == 0 && tracker.dayCount() == 0 && tracker.journeyReport().empty());
    CHECK(isAscii(tracker.dayReport()));

    begin("T26 The tracker agrees with the real narrator on every real event text");
    Trail trail;
    CHECK(trail.loadFromFile("data/events.txt"));
    const std::unique_ptr<NarrativeState> stable = NarrativeState::create(NarrativeKind::Stable);
    const std::unique_ptr<NarrativeState> unrel = NarrativeState::create(NarrativeKind::Unreliable);
    const std::unique_ptr<NarrativeState> broken = NarrativeState::create(NarrativeKind::Breakdown);
    int texts = 0;
    int detectedUnreliable = 0;
    for (int day = 1; day <= trail.lastDay(); ++day) {
        for (int part = 0; part < kPartsPerDay; ++part) {
            const Event* event = trail.getEvent(day, static_cast<TimePart>(part));
            if (!event) continue;
            std::vector<std::string> samples{ event->getText() };
            for (const Choice& choice : event->getChoices()) samples.push_back(choice.getResultText());
            for (const std::string& sample : samples) {
                ++texts;
                for (unsigned seed = 1; seed <= 20; ++seed) {
                    LieTracker::TextLie found;
                    CHECK(!LieTracker::findTextLie(sample, stable->filterText(sample, seed), found));   // Stable never lies
                    for (const NarrativeState* state : { unrel.get(), broken.get() }) {
                        const std::string shown = state->filterText(sample, seed);
                        const bool differs = shown != sample;
                        CHECK(LieTracker::findTextLie(sample, shown, found) == differs);               // never misses, never invents
                        if (differs && !found.isAside) {
                            CHECK(!found.truth.empty() && !found.shown.empty() && found.truth != found.shown);
                            CHECK(shown.find(found.shown) != std::string::npos);                        // the player really saw it
                        }
                        if (state == unrel.get() && differs) ++detectedUnreliable;
                    }
                }
            }
        }
    }
    CHECK(texts >= 15 * 5);
    CHECK(detectedUnreliable > 0);                                                                     // the narrator does lie, and it is caught

    // HUD bars: Stable is honest, Breakdown moves them, and the tracker notices.
    LieTracker bars;
    for (unsigned seed = 1; seed <= 50; ++seed) {
        for (const NarrativeState* state : { stable.get(), unrel.get(), broken.get() }) {
            std::vector<LieTracker::Bar> readings;
            for (Stat stat : { Stat::Food, Stat::Health, Stat::Sanity, Stat::Wagon }) {
                readings.push_back({ "x", state->shownValue(50, stat, seed), 50 });
            }
            LieTracker one;
            one.recordBars(readings);
            if (state == stable.get()) CHECK(one.totalCount() == 0);
            bars.recordBars(readings);
        }
    }
    CHECK(bars.totalCount() > 0);
}

static void testRealEvents(const fs::path& root) {
    begin("T15 The shipped data/events.txt loads cleanly with every slot filled");
    Trail trail;
    CHECK(trail.loadFromFile((root / "data" / "events.txt").string()));
    for (const std::string& w : trail.getWarnings()) std::cerr << "  warning: " << w << "\n";
    CHECK(trail.getWarnings().empty());
    CHECK(trail.eventCount() == 20 && trail.lastDay() == 3);
    CHECK(trail.hasDay(1) && trail.hasDay(3) && !trail.hasDay(4));
    for (int day = 1; day <= 3; ++day) {
        for (int i = 0; i < kPartsPerDay; ++i) {
            const Event* ev = trail.getEvent(day, static_cast<TimePart>(i));
            CHECK(ev != nullptr && !ev->getCondition().has_value());                   // a normal event in every slot
            if (!ev) continue;
            CHECK(ev->getChoices().size() == 4);
            for (const Choice& ch : ev->getChoices()) CHECK(!ch.getEffects().isZero());  // every choice does something
        }
    }

    begin("T16 Desperate variants replace the normal event only while their condition is active");
    ConditionSet none, lowFood, lowWagon;
    lowFood.set(Condition::LowFood);
    lowWagon.set(Condition::LowWagon);
    const Event* normal = trail.getEvent(2, TimePart::Noon, none);
    const Event* desperate = trail.getEvent(2, TimePart::Noon, lowFood);
    CHECK(normal && desperate && normal != desperate);
    CHECK(!normal->getCondition() && desperate->getCondition() && *desperate->getCondition() == Condition::LowFood);
    CHECK(trail.getEvent(2, TimePart::Noon, lowWagon) == normal);                       // wrong condition: normal event
    CHECK(trail.getEvent(1, TimePart::Dawn, lowFood) == trail.getEvent(1, TimePart::Dawn, none));  // no variant for that slot
    int variants = 0;
    for (int day = 1; day <= 3; ++day) for (int i = 0; i < kPartsPerDay; ++i) {
        ConditionSet all; all.set(Condition::LowFood); all.set(Condition::LowWagon); all.set(Condition::LowHealth);
        const Event* e = trail.getEvent(day, static_cast<TimePart>(i), all);
        if (e && e->getCondition()) ++variants;
    }
    CHECK(variants == 5);

    begin("T17 Text fits its panel: honest, and in the worst lying state; labels fit their buttons");
    TextRenderer text;
    auto broken = NarrativeState::create(NarrativeKind::Breakdown);
    ConditionSet every; every.set(Condition::LowFood); every.set(Condition::LowWagon); every.set(Condition::LowHealth);
    int worstEvent = 0, worstResult = 0, worstEverything = 0;
    for (int pass = 0; pass < 2; ++pass) {
        for (int day = 1; day <= 3; ++day) for (int i = 0; i < kPartsPerDay; ++i) {
            const Event* ev = trail.getEvent(day, static_cast<TimePart>(i), pass ? every : none);
            if (!ev) continue;
            const int honest = static_cast<int>(text.wrap(ev->getText(), 912, 2).size());
            CHECK(honest <= 4);                                                          // event screen: 4 lines above 4 buttons
            worstEvent = std::max(worstEvent, honest);
            for (unsigned seed = 0; seed < 100; ++seed) {
                worstEvent = std::max(worstEvent, static_cast<int>(text.wrap(broken->filterText(ev->getText(), seed), 912, 2).size()));
            }
            for (size_t c = 0; c < ev->getChoices().size(); ++c) {
                const Choice& ch = ev->getChoices()[c];
                CHECK(text.wrap(std::to_string(c + 1) + ". " + ch.getLabel(), 430, 2).size() <= 2);   // button: 2 lines
                for (unsigned seed = 0; seed < 100; ++seed) {
                    // result screen (7 lines): lying story + truthful summary + toll + starving
                    const std::string typical = broken->filterText(ch.getResultText(), seed) + " (" + ch.getEffects().describe() +
                        ") The trail takes its toll (Food -3, Health -13, Sanity -4, Wagon -2). The party is starving.";
                    worstResult = std::max(worstResult, static_cast<int>(text.wrap(typical, 912, 2).size()));
                    // ...plus everything that could be appended at once (death, warnings, narrator notice)
                    const std::string all = typical + " Junior has died. The curse, it turns out, was real. Warning: the food is running low."
                        " Warning: the family is in poor health. The trail has stopped pretending to be on your side.";
                    worstEverything = std::max(worstEverything, static_cast<int>(text.wrap(all, 912, 2).size()));
                }
            }
        }
    }
    CHECK(worstEvent <= 4);
    CHECK(worstResult <= 7);
    std::cout << "    longest event text " << worstEvent << "/4 lines; longest typical result " << worstResult
              << "/7 lines; everything-at-once result " << worstEverything << " lines (cut off with '...' above 7)\n";
}

static void testBrokenFiles(const fs::path& tmp) {
    begin("T18a Missing, empty and wrong files never crash and always say why");
    Trail missing;
    CHECK(!missing.loadFromFile((tmp / "does_not_exist.txt").string()) && !missing.getWarnings().empty());
    writeFile(tmp / "empty.txt", "");
    Trail empty;
    CHECK(!empty.loadFromFile((tmp / "empty.txt").string()) && !empty.getWarnings().empty());
    writeFile(tmp / "junk.txt", "this is not an events file\n\x01\x02 random = bytes\n[CHOICE]\n");
    Trail junk;
    CHECK(!junk.loadFromFile((tmp / "junk.txt").string()) && !junk.getWarnings().empty());

    begin("T18b A malformed event is skipped with a warning; good events survive");
    writeFile(tmp / "bad.txt",
        "\xEF\xBB\xBF# BOM + Windows line endings + curly quotes\r\n"
        "stray = line before any event\r\n"
        "[CHOICE]\r\nlabel = orphan\r\n"
        "[EVENT]\r\nday = 1\r\npart = Dawn\r\ntext = It\xE2\x80\x99s \xE2\x80\x9Cquoted\xE2\x80\x9D \xE2\x80\x94 and \xC3\xA9\r\n"
        "[CHOICE]\r\nlabel = Good\r\nresult = Fine\r\nfood = +7\r\nbogus = 1\r\n"
        "[EVENT]\r\nday = 1\r\npart = Noon\r\ntext = bad number\r\n[CHOICE]\r\nlabel=a\r\nresult=b\r\nfood = lots\r\n"
        "[EVENT]\r\nday = 1\r\npart = Teatime\r\ntext = bad part\r\n[CHOICE]\r\nlabel=a\r\nresult=b\r\n"
        "[EVENT]\r\nday = 1\r\npart = Evening\r\ntext = no choices\r\n"
        "[EVENT]\r\nday = 1\r\npart = Night\r\ntext = five choices\r\n"
        "[CHOICE]\r\nlabel=1\r\nresult=r\r\n[CHOICE]\r\nlabel=2\r\nresult=r\r\n[CHOICE]\r\nlabel=3\r\nresult=r\r\n[CHOICE]\r\nlabel=4\r\nresult=r\r\n[CHOICE]\r\nlabel=5\r\nresult=r\r\n"
        "[EVENT]\r\nday = 1\r\npart = Dawn\r\ntext = duplicate\r\n[CHOICE]\r\nlabel=x\r\nresult=y\r\n"
        "[EVENT]\r\nday = 2\r\npart = Dawn\r\ntext = missing result\r\n[CHOICE]\r\nlabel=only label\r\n"
        "[EVENT]\r\nday = 3\r\npart = night\r\ntext = case-insensitive keys\r\n[CHOICE]\r\nLABEL = Up\r\nRESULT = Down\r\nSANITY = 999\r\n"
        "[EVENT]\r\nday = 3\r\npart = Dawn\r\nrequires = LowMoney\r\ntext = unknown condition\r\n[CHOICE]\r\nlabel=a\r\nresult=b\r\n"
        "[EVENT]\r\nday = 3\r\npart = Dawn\r\ntext = unknown member\r\n[CHOICE]\r\nlabel=a\r\nresult=b\r\nmember = Nobody\r\n");
    Trail bad;
    CHECK(bad.loadFromFile((tmp / "bad.txt").string()));
    CHECK(!bad.getWarnings().empty());
    CHECK(bad.eventCount() == 2);                                                        // Dawn (day 1) and Night (day 3)
    const Event* good = bad.getEvent(1, TimePart::Dawn);
    CHECK(good != nullptr && good->getText() == "It's \"quoted\" - and ?");              // curly quotes, dash, accent handled
    CHECK(good && good->getChoices().size() == 1 && good->getChoices()[0].getEffects().food == 7);   // "+7" parsed
    const Event* night = bad.getEvent(3, TimePart::Night);
    CHECK(night && night->getChoices()[0].getLabel() == "Up" && night->getChoices()[0].getEffects().sanity == 100);  // clamped
    CHECK(bad.getEvent(1, TimePart::Noon) == nullptr && bad.getEvent(1, TimePart::Evening) == nullptr);
    CHECK(bad.getEvent(3, TimePart::Dawn) == nullptr);                                   // both bad day-3 Dawn events rejected
    bool sawCondition = false, sawMember = false;
    for (const std::string& w : bad.getWarnings()) {
        sawCondition = sawCondition || w.find("unknown condition") != std::string::npos;
        sawMember    = sawMember    || w.find("unknown member") != std::string::npos;
    }
    CHECK(sawCondition && sawMember);

    begin("T18c Duplicate detection, variants, and a slot with only variants");
    writeFile(tmp / "variants.txt",
        "[EVENT]\nday = 1\npart = Dawn\ntext = normal\n[CHOICE]\nlabel=a\nresult=b\nfood=1\n"
        "[EVENT]\nday = 1\npart = Dawn\nrequires = LowFood\ntext = hungry\n[CHOICE]\nlabel=a\nresult=b\nfood=2\n"
        "[EVENT]\nday = 1\npart = Dawn\nrequires = lowfood\ntext = hungry again\n[CHOICE]\nlabel=a\nresult=b\nfood=3\n"
        "[EVENT]\nday = 1\npart = Dawn\nrequires = LowWagon\ntext = wagon\n[CHOICE]\nlabel=a\nresult=b\nwagon=1\n"
        "[EVENT]\nday = 1\npart = Noon\nrequires = LowHealth\ntext = only a variant\n[CHOICE]\nlabel=a\nresult=b\nhealth=1\nmember=granny\n");
    Trail variants;
    CHECK(variants.loadFromFile((tmp / "variants.txt").string()));
    CHECK(variants.eventCount() == 4);                                                   // the duplicate LowFood was dropped
    ConditionSet food, wagon, both;
    food.set(Condition::LowFood); wagon.set(Condition::LowWagon);
    both.set(Condition::LowFood); both.set(Condition::LowWagon);
    CHECK(variants.getEvent(1, TimePart::Dawn)->getText() == "normal");
    CHECK(variants.getEvent(1, TimePart::Dawn, food)->getText() == "hungry");
    CHECK(variants.getEvent(1, TimePart::Dawn, wagon)->getText() == "wagon");
    CHECK(variants.getEvent(1, TimePart::Dawn, both)->getText() == "hungry");            // file order breaks the tie
    CHECK(variants.getEvent(1, TimePart::Noon) == nullptr);                              // only a variant: nothing normally
    ConditionSet health; health.set(Condition::LowHealth);
    const Event* v = variants.getEvent(1, TimePart::Noon, health);
    CHECK(v && v->getChoices()[0].getEffects().member == "Granny");                      // member name canonicalised
    bool sawWarning = false;
    for (const std::string& w : variants.getWarnings()) {
        sawWarning = sawWarning || w.find("only conditional variants") != std::string::npos
                                || w.find("duplicate") != std::string::npos;
    }
    CHECK(sawWarning);
}

static void testPosition(const fs::path& root) {
    begin("T19 Position tracking: 15 parts, then the next day does not exist");
    Trail trail;
    CHECK(trail.loadFromFile((root / "data" / "events.txt").string()));
    int steps = 0;
    while (trail.currentDay() <= 3) {
        CHECK(trail.currentEvent() != nullptr);
        if (trail.currentPart() == TimePart::Night) CHECK(trail.isLastPartOfDay());
        trail.advance();
        ++steps;
    }
    CHECK(steps == 15 && trail.currentDay() == 4);
    trail.reset();
    CHECK(trail.currentDay() == 1 && trail.currentPart() == TimePart::Dawn);
}

static void testSettings(const fs::path& tmp) {
    begin("T20 Settings: defaults, on/off spellings, bad lines reported, missing file fine");
    Settings none = Settings::load((tmp / "no_such_settings.txt").string());
    CHECK(none.audio && !none.playtestLog && none.warnings.empty());
    writeFile(tmp / "settings.txt",
              "\xEF\xBB\xBF# comment\r\naudio = OFF\r\nplaytest_log = yes\r\nvolume = 11\r\nplaytest_log\r\naudio = maybe\r\n");
    Settings s = Settings::load((tmp / "settings.txt").string());
    CHECK(!s.audio && s.playtestLog);
    CHECK(s.warnings.size() == 3);                                                       // volume, missing '=', 'maybe'
    Settings shipped = Settings::load((fs::current_path() / "data" / "settings.txt").string());
    CHECK(shipped.warnings.empty());
}

static void testPlaytestLog(const fs::path& tmp) {
    begin("T21 PlaytestLog: anonymous CSV, quoting, closes cleanly, fails safely");
    CHECK(PlaytestLog::csvField("plain") == "plain");
    CHECK(PlaytestLog::csvField("a,b") == "\"a,b\"");
    CHECK(PlaytestLog::csvField("say \"hi\"") == "\"say \"\"hi\"\"\"");
    CHECK(PlaytestLog::csvField("line\nbreak") == "\"linebreak\"");

    const fs::path folder = tmp / "logs";
    std::string path;
    {
        PlaytestLog log;
        CHECK(!log.isOpen());
        log.logRunStart();                                                               // before start(): harmless
        CHECK(log.start(folder.string(), "test build"));
        CHECK(log.isOpen() && log.sessionId().size() == 6);
        path = log.filePath();
        log.logRunStart();
        log.logChoice(1, "Dawn", "normal", 2, "Cook breakfast, quickly", 50, 80, 90, 95, "Stable", 4);
        log.logRunEnd("survived", 15, 3, 10, 60, 40, 50);
    }                                                                                    // destructor closes the file
    const std::string text = readAll(path);
    CHECK(text.find("# build: test build") != std::string::npos);
    CHECK(text.find("seconds,run,type,day,part,variant,choice,label") != std::string::npos);
    CHECK(text.find("CHOICE,1,Dawn,normal,2,\"Cook breakfast, quickly\",50,80,90,95,Stable,4") != std::string::npos);
    CHECK(text.find("END") != std::string::npos && text.find("QUIT") != std::string::npos);
    const char* personal[] = { "C:\\", "Users", "@", "atovar" };
    for (const char* needle : personal) CHECK(text.find(needle) == std::string::npos);   // nothing personal in the file
    PlaytestLog impossible;                                                              // a folder that cannot exist
    CHECK(!impossible.start((tmp / "empty.txt" / "inside_a_file").string(), "x"));
}

static void testAudio(const fs::path& root) {
    begin("T22 AudioManager is harmless without init, with missing files, and (if built in) with irrKlang");
    AudioManager audio;
    audio.play(Sfx::Click);
    audio.setMood(Mood::Dread);
    audio.setMuted(true);
    audio.setMuted(false);
    CHECK(!audio.isAvailable());
    const bool started = audio.init((root / "assets" / "audio").string(), /*useNullDevice=*/true);
    CHECK(started == AudioManager::builtWithIrrKlang());                                 // silent stub, or real engine
    for (Sfx s : { Sfx::Click, Sfx::Good, Sfx::Bad, Sfx::Warning, Sfx::Death, Sfx::DayEnd, Sfx::StateChange, Sfx::Start }) audio.play(s);
    audio.setMood(Mood::Calm);
    CHECK(audio.getMood() == Mood::Calm);
    audio.setMuted(true);
    CHECK(audio.isMuted());
    audio.play(Sfx::Death);
    audio.setMuted(false);
    AudioManager nofiles;
    nofiles.init((root / "no_such_folder").string(), true);
    nofiles.setMood(Mood::Uneasy);
    nofiles.play(Sfx::Good);
    std::cout << "    audio: " << (AudioManager::builtWithIrrKlang() ? "tested with irrKlang" : "tested as silent stub") << "\n";

    begin("T23 Every sound file the game asks for exists");
    for (const char* file : { "ambient_calm.wav", "ambient_uneasy.wav", "ambient_dread.wav", "click.wav", "good.wav",
                              "bad.wav", "warning.wav", "death.wav", "day_end.wav", "state_change.wav", "start.wav" }) {
        CHECK(fs::exists(root / "assets" / "audio" / file) && fs::file_size(root / "assets" / "audio" / file) > 100);
    }
}


// ---------------------------------------------------------------------------------------------
// Round 2 playtest changes: difficulty levels, scene/action animations, remembered progress
// ---------------------------------------------------------------------------------------------

static void testDifficulty() {
    begin("T27 Difficulty: Easy is gentler than Normal, Normal gentler than Hard; names, parsing, cycling");
    const DifficultyRules& e = rulesFor(Difficulty::Easy);
    const DifficultyRules& n = rulesFor(Difficulty::Normal);
    const DifficultyRules& h = rulesFor(Difficulty::Hard);
    CHECK(e.startFood >= n.startFood && n.startFood >= h.startFood);
    CHECK(e.foodPerPart <= n.foodPerPart && n.foodPerPart <= h.foodPerPart);
    CHECK(e.sanityPerPart <= n.sanityPerPart && n.sanityPerPart <= h.sanityPerPart);
    CHECK(e.hardshipPerPart <= n.hardshipPerPart && n.hardshipPerPart <= h.hardshipPerPart);
    CHECK(e.starvationDamage <= n.starvationDamage && n.starvationDamage <= h.starvationDamage);
    CHECK(e.harmPercent <= n.harmPercent && n.harmPercent <= h.harmPercent);
    CHECK(e.helpPercent >= n.helpPercent && n.helpPercent >= h.helpPercent);
    CHECK(e.harmPercent > 0 && h.helpPercent > 0 && e.startFood <= Party::kMaxStat);

    CHECK(std::string(difficultyName(Difficulty::Easy)) == "Easy" && std::string(difficultyName(Difficulty::Hard)) == "Hard");
    Difficulty parsed = Difficulty::Normal;
    CHECK(parseDifficulty("HARD", parsed) && parsed == Difficulty::Hard);
    CHECK(parseDifficulty("easy", parsed) && parsed == Difficulty::Easy);
    CHECK(!parseDifficulty("impossible", parsed) && parsed == Difficulty::Easy);          // untouched on failure
    CHECK(nextDifficulty(Difficulty::Easy) == Difficulty::Normal && nextDifficulty(Difficulty::Normal) == Difficulty::Hard &&
          nextDifficulty(Difficulty::Hard) == Difficulty::Easy);

    // The same party on each difficulty: a fresh journey starts with that difficulty's food, and the trail's toll follows it
    int previousToll = -1;
    for (Difficulty d : { Difficulty::Easy, Difficulty::Normal, Difficulty::Hard }) {
        Party p;
        p.setDifficulty(d);
        p.reset();
        CHECK(p.difficulty() == d && p.getFood() == rulesFor(d).startFood);
        const int foodBefore = p.getFood(), sanityBefore = p.getSanity(), wagonBefore = p.getWagon();
        p.advanceTime();
        CHECK(foodBefore - p.getFood() == rulesFor(d).foodPerPart);
        CHECK(sanityBefore - p.getSanity() == rulesFor(d).sanityPerPart);
        CHECK(wagonBefore - p.getWagon() == rulesFor(d).wagonPerPart);
        const int toll = 100 - p.getHealth();
        CHECK(toll >= rulesFor(d).hardshipPerPart);              // frail members lose a little more than the base hardship
        CHECK(toll >= previousToll);                             // never gentler than the easier level
        previousToll = toll;

        Party copy(p);                                           // copies (used by the simulator) keep the difficulty
        CHECK(copy.difficulty() == d);
        Party other;
        other = p;
        CHECK(other.difficulty() == d);
        p.reset();                                               // reset keeps the difficulty
        CHECK(p.difficulty() == d && p.getFood() == rulesFor(d).startFood);
    }

    // Starvation damage follows the difficulty too
    for (Difficulty d : { Difficulty::Easy, Difficulty::Hard }) {
        Party p; p.setDifficulty(d); p.reset();
        Effects drain; drain.food = -100;
        p.apply(drain);
        CHECK(p.getFood() == 0);
        const int before = p.getHealth();
        p.advanceTime();
        CHECK(before - p.getHealth() >= rulesFor(d).hardshipPerPart + rulesFor(d).starvationDamage);
    }

    begin("T28 Party::adjusted scales harm and help, never to zero, never changing who is hit");
    Party normal; normal.setDifficulty(Difficulty::Normal); normal.reset();
    Party easy;   easy.setDifficulty(Difficulty::Easy);     easy.reset();
    Party hard;   hard.setDifficulty(Difficulty::Hard);     hard.reset();
    Effects fx; fx.food = -10; fx.health = -6; fx.sanity = 4; fx.wagon = -1; fx.member = "Granny";
    const Effects ne = normal.adjusted(fx), ea = easy.adjusted(fx), ha = hard.adjusted(fx);
    CHECK(ne.member == "Granny" && ea.member == "Granny" && ha.member == "Granny");
    CHECK(ha.food <= ne.food && ne.food <= ea.food);                   // harm: Hard hurts most (more negative)
    CHECK(ha.health <= ne.health && ne.health <= ea.health);
    CHECK(ha.sanity <= ne.sanity && ne.sanity <= ea.sanity);           // help: Hard helps least
    CHECK(ea.wagon < 0 && ne.wagon < 0 && ha.wagon < 0);               // -1 stays a loss on every difficulty
    CHECK(ha.food < fx.food);                                          // Hard really is harsher than the data file
    Effects zero;
    CHECK(hard.adjusted(zero).isZero() && easy.adjusted(zero).isZero());
    Effects tiny; tiny.sanity = 1;
    CHECK(hard.adjusted(tiny).sanity >= 1);                            // a +1 never rounds away
    // What describe() reports is what apply() does, so the bracketed summary stays true
    Party scratch; scratch.setDifficulty(Difficulty::Hard); scratch.reset();
    const Effects landed = scratch.adjusted(fx);
    const int foodBefore = scratch.getFood();
    scratch.apply(landed);
    CHECK(scratch.getFood() == std::max(0, std::min(100, foodBefore + landed.food)));
}

static void testAnimations(const fs::path& root, const fs::path& tmp) {
    begin("T29 Animation names: every scene and action round-trips, parsing forgives case and dashes");
    for (int i = 0; i < kSceneCount; ++i) {
        SceneId back = SceneId::Trail;
        CHECK(parseScene(sceneName(static_cast<SceneId>(i)), back) && static_cast<int>(back) == i);
    }
    for (int i = 0; i <= kActionCount; ++i) {
        ActionId back = ActionId::None;
        CHECK(parseAction(actionName(static_cast<ActionId>(i)), back) && static_cast<int>(back) == i);
    }
    SceneId sc = SceneId::Trail; ActionId ac = ActionId::None;
    CHECK(parseScene("Oxen-Circle", sc) && sc == SceneId::OxenCircle);
    CHECK(parseScene("MAP MAN", sc) && sc == SceneId::MapMan);
    CHECK(parseAction("FLEE", ac) && ac == ActionId::Flee);
    CHECK(!parseScene("moon_base", sc) && !parseAction("dance", ac));
    CHECK(kSceneCount >= 10 && kActionCount >= 15);

    begin("T30 The shipped events all have an animation, and every animation is used somewhere");
    Trail trail;
    CHECK(trail.loadFromFile((root / "data" / "events.txt").string()));
    bool sceneUsed[kSceneCount] = {};
    bool actionUsed[kActionCount + 1] = {};
    int untaggedEvents = 0, untaggedChoices = 0, sawUnknownKeyWarning = 0;
    for (int day = 1; day <= trail.lastDay(); ++day) {
        for (int part = 0; part < kPartsPerDay; ++part) {
            ConditionSet everything;
            everything.set(Condition::LowFood); everything.set(Condition::LowWagon); everything.set(Condition::LowHealth);
            for (const ConditionSet& active : { ConditionSet(), everything }) {
                const Event* ev = trail.getEvent(day, static_cast<TimePart>(part), active);
                if (!ev) continue;
                if (ev->getScene() == SceneId::Trail) ++untaggedEvents;
                sceneUsed[static_cast<int>(ev->getScene())] = true;
                for (const Choice& choice : ev->getChoices()) {
                    if (choice.getAction() == ActionId::None) ++untaggedChoices;
                    actionUsed[static_cast<int>(choice.getAction())] = true;
                }
            }
        }
    }
    for (const std::string& w : trail.getWarnings()) {
        if (w.find("unknown scene") != std::string::npos || w.find("unknown action") != std::string::npos) ++sawUnknownKeyWarning;
    }
    CHECK(untaggedEvents == 0 && untaggedChoices == 0);
    CHECK(sawUnknownKeyWarning == 0);
    for (int i = 1; i < kSceneCount; ++i) CHECK(sceneUsed[i]);          // (Trail is the default, not an event scene)
    for (int i = 1; i <= kActionCount; ++i) CHECK(actionUsed[i]);       // no dead animations

    begin("T31 A bad scene or action name only warns: the event and its choices still load");
    writeFile(tmp / "anim.txt",
        "[EVENT]\nday = 1\npart = Dawn\nscene = River\ntext = ok\n"
        "[CHOICE]\nlabel = a\nresult = b\naction = Flee\n"
        "[CHOICE]\nlabel = c\nresult = d\naction = moonwalk\n"
        "[EVENT]\nday = 1\npart = Noon\nscene = underwater\ntext = bad scene\n"
        "[CHOICE]\nlabel = a\nresult = b\n");
    Trail anim;
    CHECK(anim.loadFromFile((tmp / "anim.txt").string()));
    CHECK(anim.eventCount() == 2);
    const Event* river = anim.getEvent(1, TimePart::Dawn);
    CHECK(river && river->getScene() == SceneId::River);
    CHECK(river && river->getChoices().size() == 2 && river->getChoices()[0].getAction() == ActionId::Flee &&
          river->getChoices()[1].getAction() == ActionId::None);
    const Event* noon = anim.getEvent(1, TimePart::Noon);
    CHECK(noon && noon->getScene() == SceneId::Trail);
    int warned = 0;
    for (const std::string& w : anim.getWarnings()) {
        if (w.find("unknown action") != std::string::npos || w.find("unknown scene") != std::string::npos) ++warned;
    }
    CHECK(warned == 2);
}

static void testProgress(const fs::path& tmp) {
    begin("T32 Progress: endings are classified, discoveries count once, and everything round-trips through the file");
    CHECK(Progress::classify(true, 4, 50, 80) == EndingId::Intact);
    CHECK(Progress::classify(true, 3, 50, 80) == EndingId::LostOne);
    CHECK(Progress::classify(true, 2, 50, 80) == EndingId::Half);
    CHECK(Progress::classify(true, 1, 50, 80) == EndingId::Lone);
    CHECK(Progress::classify(false, 0, 50, 80) == EndingId::EveryoneDied);
    CHECK(Progress::classify(false, 2, 0, 80) == EndingId::SanityLost);
    CHECK(Progress::classify(false, 2, 10, 0) == EndingId::WagonLost);
    for (int i = 0; i < kEndingCount; ++i) CHECK(std::string(endingTitle(static_cast<EndingId>(i))).size() > 3);

    Progress p;
    CHECK(p.scenesSeen() == 0 && p.actionsSeen() == 0 && p.endingsSeen() == 0 && p.clearedCount() == 0);
    CHECK(p.markScene(SceneId::Deer) && !p.markScene(SceneId::Deer));         // new once, then not new
    CHECK(p.markAction(ActionId::Flee) && !p.markAction(ActionId::Flee));
    CHECK(!p.markAction(ActionId::None));                                      // "no animation" is not a discovery
    CHECK(p.markEnding(EndingId::Half) && !p.markEnding(EndingId::Half));
    p.markCleared(Difficulty::Hard);
    p.difficulty = Difficulty::Hard;
    p.runs = 7;
    CHECK(p.scenesSeen() == 1 && p.actionsSeen() == 1 && p.endingsSeen() == 1 && p.clearedCount() == 1);
    CHECK(p.animationsSeen() == 2 && Progress::animationTotal() == kSceneCount + kActionCount);

    const std::string file = (tmp / "save" / "progress.txt").string();           // a folder that doesn't exist yet
    CHECK(p.save(file));
    Progress q = Progress::load(file);
    CHECK(q.difficulty == Difficulty::Hard && q.runs == 7);
    CHECK(q.sceneSeen(SceneId::Deer) && !q.sceneSeen(SceneId::River));
    CHECK(q.actionSeen(ActionId::Flee) && !q.actionSeen(ActionId::Burn));
    CHECK(q.endingSeen(EndingId::Half) && !q.endingSeen(EndingId::Intact));
    CHECK(q.cleared(Difficulty::Hard) && !q.cleared(Difficulty::Easy));
    CHECK(q.scenesSeen() == 1 && q.actionsSeen() == 1 && q.endingsSeen() == 1);

    begin("T33 Progress: a missing, damaged or unwritable file never breaks the game");
    std::vector<std::string> warnings;
    Progress none = Progress::load((tmp / "nothing_here.txt").string(), &warnings);
    CHECK(warnings.empty() && none.animationsSeen() == 0 && none.difficulty == Difficulty::Normal);
    writeFile(tmp / "damaged.txt", "\xEF\xBB\xBF" "this is nonsense\ndifficulty = ludicrous\nruns = lots\nscenes = deer,moon_base\n"
                                    "actions = flee,dance\nendings = half,heaven\ncleared = easy,never\nmystery = 1\n");
    warnings.clear();
    Progress damaged = Progress::load((tmp / "damaged.txt").string(), &warnings);
    CHECK(warnings.size() >= 7);                                              // every bad line is reported
    CHECK(damaged.difficulty == Difficulty::Normal && damaged.runs == 0);
    CHECK(damaged.sceneSeen(SceneId::Deer) && damaged.actionSeen(ActionId::Flee) &&
          damaged.endingSeen(EndingId::Half) && damaged.cleared(Difficulty::Easy));   // the good parts survive
    CHECK(damaged.scenesSeen() == 1 && damaged.actionsSeen() == 1);
    CHECK(!Progress().save((tmp / "save" / "progress.txt" / "inside_a_file" / "x.txt").string()));  // can't write: false, no crash
}

int main(int argc, char** argv) {
    const fs::path root = argc > 1 ? fs::path(argv[1]) : fs::current_path();
    if (!fs::exists(root / "data" / "events.txt")) {
        std::cerr << "Run from the repo root (or pass it as the first argument): data/events.txt not found under "
                  << root << "\n";
        return 2;
    }
    fs::current_path(root);  // the game and Settings use paths relative to the repo root

    const fs::path tmp = fs::temp_directory_path() / "scarygon_tests";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    std::cout << "Scarygon Trail logic tests\n";
    testEffectsAndParsing();
    testMembers();
    testPartyRules();
    testObservers();
    testNarrator();
    testLieTracker();
    testRealEvents(root);
    testBrokenFiles(tmp);
    testPosition(root);
    testSettings(tmp);
    testPlaytestLog(tmp);
    testAudio(root);
    testDifficulty();
    testAnimations(root, tmp);
    testProgress(tmp);

    fs::remove_all(tmp);
    std::cout << "\n" << g_checks << " checks, " << g_failed << " failed\n";
    if (g_failed == 0) std::cout << "ALL TESTS PASSED\n";
    return g_failed == 0 ? 0 : 1;
}
