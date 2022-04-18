#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientpathfinder.hpp"

ClientPathFinder::ClientPathFinder(const ProcessRun *argProc, bool argCheckGround, int argCheckCreature, int argMaxStep)
    : AStarPathFinder([this](int argSrcX, int argSrcY, int argDstX, int argDstY) -> double
      {
          fflassert(MaxStep() >= 1, MaxStep());
          fflassert(MaxStep() <= 3, MaxStep());

          const int distance = mathf::LDistance2<int>(argSrcX, argSrcY, argDstX, argDstY);
          if(true
                  && distance != 1
                  && distance != 2
                  && distance != MaxStep() * MaxStep()
                  && distance != MaxStep() * MaxStep() * 2){
              throw fflerror("invalid step checked: (%d, %d) -> (%d, %d)", argSrcX, argSrcY, argDstX, argDstY);
          }

          return m_proc->OneStepCost(this, m_checkGround, m_checkCreature, argSrcX, argSrcY, argDstX, argDstY);
      }, argMaxStep)

    , m_proc(argProc)
    , m_checkGround(argCheckGround)
    , m_checkCreature(argCheckCreature)
{
    fflassert(m_proc);

    fflassert(m_checkCreature >= 0, m_checkCreature);
    fflassert(m_checkCreature <= 2, m_checkCreature);

    fflassert(MaxStep() >= 1, MaxStep());
    fflassert(MaxStep() <= 3, MaxStep());
}

int ClientPathFinder::getGrid(int argX, int argY) const
{
    if(!m_proc->validC(argX, argY)){
        return PathFind::INVALID;
    }

    if(const auto p = m_cache.find({argX, argY}); p != m_cache.end()){
        return p->second;
    }
    return m_cache[{argX, argY}] = m_proc->CheckPathGrid(argX, argY);
}
