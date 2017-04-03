/*
  Apery, a USI shogi playing engine derived from Stockfish, a UCI chess playing engine.
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2016 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad
  Copyright (C) 2011-2016 Hiraoka Takuya

  Apery is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Apery is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "benchmark.hpp"
#include "common.hpp"
#include "usi.hpp"
#include "position.hpp"
#include "search.hpp"

#if 1
// 今はベンチマークというより、PGO ビルドの自動化の為にある。
void benchmark(Position& pos, std::istream& is) {
    std::string token;
    std::string options[] = {"name Threads value 1",
                             "name MultiPV value 1",
                             "name OwnBook value false",
                             "name Max_Random_Score_Diff value 0"};
    for (auto& str : options) {
        std::istringstream is(str);
        pos.searcher()->setOption(is);
    }

    std::ifstream ifs("benchmark.sfen");
    std::string sfen;
    while (std::getline(ifs, sfen)) {
        std::cout << sfen << std::endl;
        std::istringstream ss_sfen(sfen);
        setPosition(pos, ss_sfen);
        std::istringstream ss_go("byoyomi 10000");
        go(pos, ss_go);
        pos.searcher()->threads.main()->waitForSearchFinished();
    }
}
#else
using namespace std;

namespace {
    const vector<string> Defaults = { //nozomi
        "l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w GR5pnsg 1",
        "+R5g2/4g3k/5snp1/p2P1ppN1/4p2P1/P1g1nPP1K/1Pp2SN2/3ps1L2/L1G2b1+r1 w S5Pb2l 1",
        "lnsgk1snl/1r4gb1/p1pppp1pp/6p2/1p7/6R2/PPPPPPP1P/1B7/LNSGKGSNL b P 1",
        "ln4knl/1r3sgb1/3psg1pp/ppp1p1p2/5p3/2PPP4/PPSGSPPPP/2GB3R1/LNK4NL b - 1",

        "l5knl/5rgb1/3p3pp/p+P1s2g2/1np3p2/1S1Pp4/PP1S1P1PP/1K1G3R1/LNB1g2NL w S2P2p 1",
        "6kn1/4+B3l/p3pp1pp/r5p2/2s2Ps2/1P2P3P/P1PP2P+l1/2GKG4/LN3s+bNL b 2GN3Prsp 1",
        "lr5nl/3kg4/p1ns1pg1p/1p1pp4/2P4s1/3PPS+sp1/PPG2P2P/4G1RP1/L1BK3NL b B3Pn 1",
        "ln1g3nl/1r+b2kgs1/p2ppp1pp/1Sp3p2/1P1PB4/1pPS5/PsNG1PP1P/7R1/L1G1K2NL w 2P 1",

        "ln2k2g1/r1s6/pg1gpGssp/2Pp1pP2/4Nn3/2S5B/PPN2P1pP/2KLL4/L8 w B2Pr4p 1",
        "l6nl/1l7/3k3g1/p2spNp2/4bP3/PPPP1p2P/1S2P1N1+b/1KG2G1p1/LN1r5 w RG5P2sp 1",
        "+R3s3l/4g2k1/p1ppppPpp/9/n5+bP1/1PPP2p2/PGS5P/1KBG2L2/LN2r3L b SN3Pgsn 1",
        "ln1b5/1rs2ggk1/p2pp1sp1/1pP2pp1P/P6+s1/1LNPP4/1LSG1P3/1RKG5/LN7 b NPb4p 1",

        "l2g4l/1ks1g4/ppnsp1+Rpp/2ppr4/P6P1/1PP1PP3/1K1PB2+bP/2SNG4/LN1G3NL b 3Ps 1",
        "+R6nl/5s3/4n1+bpp/p2pp3k/1P5l1/1GP2PN2/P2PPN1sP/2gSKp2R/1s2G2LL b Gb5p 1",
        "ln1gk2nl/1r1s1sgb1/p1pp1p1pp/4p1p2/1p7/2PP5/PPBSPPPPP/2GR1K3/LN3GSNL w - 1",
        "lnS1k3l/2r3g2/p2ppgnpp/2psb1R2/5p3/2P6/PP1PPPS1P/1SG1K1G2/LN6L b B2Pn2p 1"
    };

} // namespace

void benchmark(Position& current, istream& is) {

    string token;
    vector<string> fens;
    LimitsType limits;
    auto s = current.searcher();

    // Assign default values to missing arguments
    string ttSize = (is >> token) ? token : "16";
    string threads = (is >> token) ? token : "1";
    string limit = (is >> token) ? token : "13";
    string fenFile = (is >> token) ? token : "default";
    string limitType = (is >> token) ? token : "depth";

    s->options["USI_Hash"] = ttSize;
    s->options["Threads"] = threads;
    s->clear();

    if (limitType == "time")
        limits.moveTime = stoi(limit); // movetime is in millisecs

    else if (limitType == "nodes")
        limits.nodes = stoi(limit);

    else if (limitType == "mate")
        limits.mate = stoi(limit);

    else
        limits.depth = stoi(limit);

    if (fenFile == "default")
        fens = Defaults;

    //else if (fenFile == "current")
    //    fens.push_back(current.fen());

    else
    {
        string fen;
        ifstream file(fenFile);

        if (!file.is_open())
        {
            cerr << "Unable to open file " << fenFile << endl;
            return;
        }

        while (getline(file, fen))
            if (!fen.empty())
                fens.push_back(fen);

        file.close();
    }

    uint64_t nodes = 0;
    Timer t = Timer::currentTime();

    for (size_t i = 0; i < fens.size(); ++i)
    {
        Position pos(fens[i], s->threads.main(), s->thisptr);

        cerr << "\nPosition: " << i + 1 << '/' << fens.size() << endl;

        /*if (limitType == "perft")
        nodes += Search::perft(pos, limits.depth * OnePly);

        else*/
        {
            limits.startTime = Timer::currentTime();
            s->threads.startThinking(pos, limits, pos.searcher()->states);
            s->threads.main()->waitForSearchFinished();
            nodes += s->threads.nodesSearched();
        }
    }

    const int elapsed = t.elapsed(); // Ensure positivity to avoid a 'divide by zero'

    //dbg_print(); // Just before to exit

    cerr << "\n==========================="
         << "\nTotal time (ms) : " << elapsed
         << "\nNodes searched  : " << nodes
         << "\nNodes/second    : " << 1000 * nodes / elapsed << endl;
}

#endif
