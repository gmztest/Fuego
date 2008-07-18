//----------------------------------------------------------------------------
/** @file GoUctDefaultRootFilter.cpp
    See GoUctDefaultRootFilter.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "GoUctDefaultRootFilter.h"

#include "GoBensonSolver.h"
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoModBoard.h"

using namespace std;

//----------------------------------------------------------------------------

GoUctDefaultRootFilter::GoUctDefaultRootFilter(const GoBoard& bd)
    : m_bd(bd),
      m_checkLadders(false),
      m_minLadderLength(6)
{
}

vector<SgPoint> GoUctDefaultRootFilter::Get()
{
    vector<SgPoint> rootFilter;
    SgBlackWhite toPlay = m_bd.ToPlay();
    SgBlackWhite opp = SgOppBW(toPlay);

    // Safe territory

    GoModBoard modBoard(m_bd);
    GoBoard& bd = modBoard.Board();
    // Use Benson solver to ensure that all moves that capture dead stones
    // are liberties of dead blocks
    GoBensonSolver bensonSolver(bd);
    SgBWSet safe;
    bensonSolver.FindSafePoints(&safe);
    for (GoBoard::Iterator it(bd); it; ++it)
    {
        SgPoint p = *it;
        if (m_bd.IsLegal(p))
        {
            bool isSafe = safe[toPlay].Contains(p);
            bool isSafeOpp = safe[opp].Contains(p);
            bool hasOppNeighbors = bd.HasNeighbors(p, opp);
            // Always generate capturing moves in own safe territory, even
            // if current rules do no use CaptureDead(), because the UCT
            // player always scores with Tromp-Taylor after two passes in the
            // in-tree phase
            if ((isSafe && ! hasOppNeighbors) || isSafeOpp)
                rootFilter.push_back(p);
        }
    }

    // Loosing ladder defense moves
    if (m_checkLadders)
        for (GoBlockIterator it(m_bd); it; ++it)
        {
            SgPoint p = *it;
            if (m_bd.GetStone(p) == toPlay && m_bd.InAtari(p))
            {
                if (m_ladder.Ladder(m_bd, p, toPlay, &m_ladderSequence, true))
                {
                    if (m_ladderSequence.Length() >= m_minLadderLength)
                        rootFilter.push_back(m_bd.TheLiberty(p));
                }
            }

        }

    return rootFilter;
}

//----------------------------------------------------------------------------