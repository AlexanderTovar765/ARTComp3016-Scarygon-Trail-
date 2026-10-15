// Balance simulator for The Scarygon Trail.
//
// Plays thousands of full games using the real event loader (Trail) and the real
// rules (Party), with different strategies, and prints a Markdown report.
// It uses no SDL, so it builds and runs without the game window.
//
// Usage:   balance_sim [path/to/events.txt] [runs]      (defaults: data/events.txt, 20000)
// Build (Visual Studio Developer PowerShell, from the repo root):
//   cl /std:c++17 /EHsc /O2 /Isrc tools\balance_sim.cpp src\Trail.cpp src\Party.cpp src\Effects.cpp src\NarrativeState.cpp /Fe:balance_sim.exe
// Build (CMake):  cmake --build <build-dir> --target balance_sim

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "NarrativeState.h"
#include "Party.h"
#include "Trail.h"

namespace {
    enum class Strategy { Random, Greedy, Cautious, Reckless };

    const char* strategyName(Strategy s) {
        switch (s) {
        case Strategy::Random:   return "Random (picks any choice)";
        case Strategy::Greedy:   return "Greedy (best total effect)";
        case Strategy::Cautious: return "Cautious (protects the lowest stat)";
        case Strategy::Reckless: return "Reckless (worst total effect)";
        }
        return "";
    }

    int total(const Effects& e) { return e.food + e.health + e.sanity + e.wagon; }

    // Picks a choice index according to the strategy.
    std::size_t pick(Strategy strategy, const Event& event, const Party& party, std::mt19937& rng) {
        const std::vector<Choice>& choices = event.getChoices();
        if (strategy == Strategy::Random) {
            return std::uniform_int_distribution<std::size_t>(0, choices.size() - 1)(rng);
        }

        std::size_t best = 0;
        int bestScore = 0;
        for (std::size_t i = 0; i < choices.size(); ++i) {
            const Effects& e = choices[i].getEffects();
            int score = 0;
            if (strategy == Strategy::Greedy) {
                score = total(e);
            } else if (strategy == Strategy::Reckless) {
                score = -total(e);
            } else {  // Cautious: maximise the lowest of the resulting stats
                Party copy = party;
                copy.apply(e);
                copy.advanceTime();
                score = std::min(std::min(copy.getFood(), copy.getHealth()),
                                 std::min(copy.getSanity(), copy.getWagon()));
            }
            if (i == 0 || score > bestScore) { best = i; bestScore = score; }
        }
        return best;
    }

    struct Results {
        int runs = 0;
        int survived = 0;
        std::map<std::string, int> deaths;   // cause -> count
        std::vector<int> deathDay;           // index = day
        long long finalFood = 0, finalHealth = 0, finalSanity = 0, finalWagon = 0;
        int starvedRuns = 0;
        long long partsPlayed = 0, partsStable = 0, partsUnreliable = 0, partsBreakdown = 0;
        int ranIntoUnreliable = 0, ranIntoBreakdown = 0;
    };

    void playOne(const Trail& original, Strategy strategy, std::mt19937& rng, Results& out) {
        Trail trail = original;
        trail.reset();
        Party party;
        bool starved = false, sawUnreliable = false, sawBreakdown = false;
        const int lastDay = trail.lastDay();
        bool alive = true;

        for (int day = 1; day <= lastDay && alive; ++day) {
            for (int p = 0; p < kPartsPerDay && alive; ++p) {
                const Event* event = trail.getEvent(day, static_cast<TimePart>(p));
                if (!event) continue;

                const NarrativeKind kind = NarrativeState::kindForSanity(party.getSanity());
                ++out.partsPlayed;
                if (kind == NarrativeKind::Stable) ++out.partsStable;
                if (kind == NarrativeKind::Unreliable) { ++out.partsUnreliable; sawUnreliable = true; }
                if (kind == NarrativeKind::Breakdown)  { ++out.partsBreakdown;  sawBreakdown = true; }

                party.apply(event->getChoices()[pick(strategy, *event, party, rng)].getEffects());
                const Effects upkeep = party.advanceTime();
                if (upkeep.health < 0) starved = true;

                if (!party.isAlive()) {
                    alive = false;
                    ++out.deaths[party.causeOfDeath()];
                    if (static_cast<int>(out.deathDay.size()) <= day) out.deathDay.resize(day + 1, 0);
                    ++out.deathDay[day];
                }
            }
        }

        ++out.runs;
        if (alive) {
            ++out.survived;
            out.finalFood += party.getFood();   out.finalHealth += party.getHealth();
            out.finalSanity += party.getSanity(); out.finalWagon += party.getWagon();
        }
        if (starved) ++out.starvedRuns;
        if (sawUnreliable) ++out.ranIntoUnreliable;
        if (sawBreakdown)  ++out.ranIntoBreakdown;
    }

    std::string pct(double part, double whole) {
        std::ostringstream s;
        s << std::fixed << std::setprecision(1) << (whole > 0 ? 100.0 * part / whole : 0.0) << "%";
        return s.str();
    }
}

int main(int argc, char** argv) {
    const std::string path = argc > 1 ? argv[1] : "data/events.txt";
    const int runs = argc > 2 ? std::max(100, std::atoi(argv[2])) : 20000;

    Trail trail;
    if (!trail.loadFromFile(path)) {
        for (const std::string& w : trail.getWarnings()) std::cerr << w << "\n";
        return 1;
    }
    for (const std::string& w : trail.getWarnings()) std::cerr << "warning: " << w << "\n";

    std::cout << "# Balance report\n\n"
              << "Source: `" << path << "` (" << trail.eventCount() << " events, " << trail.lastDay()
              << " days). " << runs << " simulated runs per strategy, fixed random seed.\n\n"
              << "Start: Food " << Party::kStartFood << ", other stats " << Party::kMaxStat
              << ". After every part the trail takes its toll: Food -" << Party::kFoodPerPart << ", Sanity -"
              << Party::kSanityPerPart << ", Wagon -" << Party::kWagonPerPart << "; at 0 food the party also loses "
              << Party::kStarvationDamage << " health. Health, Sanity or Wagon at 0 ends the run.\n\n";

    std::cout << "## Survival by strategy\n\n"
              << "| Strategy | Survives all days | Starved at some point | Reached Unreliable | Reached Breakdown |\n"
              << "|---|---|---|---|---|\n";

    std::vector<std::pair<Strategy, Results>> all;
    for (Strategy s : { Strategy::Random, Strategy::Greedy, Strategy::Cautious, Strategy::Reckless }) {
        std::mt19937 rng(12345);
        Results r;
        for (int i = 0; i < runs; ++i) playOne(trail, s, rng, r);
        all.emplace_back(s, r);
        std::cout << "| " << strategyName(s) << " | " << pct(r.survived, r.runs) << " | "
                  << pct(r.starvedRuns, r.runs) << " | " << pct(r.ranIntoUnreliable, r.runs) << " | "
                  << pct(r.ranIntoBreakdown, r.runs) << " |\n";
    }

    for (const auto& entry : all) {
        const Strategy s = entry.first;
        const Results& r = entry.second;
        if (s != Strategy::Random && s != Strategy::Cautious) continue;

        std::cout << "\n## " << strategyName(s) << "\n\n";
        std::cout << "Time spent in each narrator state: Stable " << pct(r.partsStable, r.partsPlayed)
                  << ", Unreliable " << pct(r.partsUnreliable, r.partsPlayed)
                  << ", Breakdown " << pct(r.partsBreakdown, r.partsPlayed) << ".\n\n";
        if (r.survived > 0) {
            std::cout << "Average final stats of survivors: Food " << r.finalFood / r.survived
                      << ", Health " << r.finalHealth / r.survived << ", Sanity " << r.finalSanity / r.survived
                      << ", Wagon " << r.finalWagon / r.survived << ".\n\n";
        }
        std::cout << "Deaths (" << r.runs - r.survived << " of " << r.runs << " runs):\n\n";
        for (const auto& d : r.deaths) {
            std::cout << "- " << pct(d.second, r.runs) << " of all runs: " << d.first << "\n";
        }
        for (std::size_t day = 1; day < r.deathDay.size(); ++day) {
            std::cout << "- died on Day " << day << ": " << pct(r.deathDay[day], r.runs) << " of all runs\n";
        }
    }

    std::cout << "\n## Notes and limits\n\n"
              << "- Strategies are simple rules, not real players, so treat these numbers as a guide to relative difficulty.\n"
              << "- The simulation ignores the narrator's lies (only the display changes, never the effects) and playtesters' reading of the text.\n"
              << "- Generated by `tools/balance_sim.cpp`; rerun it after editing `data/events.txt`.\n";
    return 0;
}
