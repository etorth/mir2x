#include "fflerror.hpp"
#include "pathf.hpp"
#include "processrun.hpp"
#include "clientpathfinder.hpp"

ClientPathFinder::ClientPathFinder(const ProcessRun *argProc, bool argCheckGround, int argCheckCreature, int argMaxStep)
    : AStarPathFinder(1, argMaxStep, [this](int argSrcX, int argSrcY, int argSrcDir, int argDstX, int argDstY) -> std::optional<double>
      {
          fflassert(pathf::hopValid(maxStep(), argSrcX, argSrcY, argDstX, argDstY), maxStep(), argSrcX, argSrcY, pathf::dirName(argSrcDir), argDstX, argDstY);
          return m_proc->oneStepCost(this, m_checkGround, m_checkCreature, argSrcX, argSrcY, argSrcDir, argDstX, argDstY);
      })

    , m_proc(argProc)
    , m_checkGround(argCheckGround)
    , m_checkCreature(argCheckCreature)
{
    fflassert(m_proc);

    fflassert(m_checkCreature >= 0, m_checkCreature);
    fflassert(m_checkCreature <= 2, m_checkCreature);

    fflassert(maxStep() >= 1, maxStep());
    fflassert(maxStep() <= 3, maxStep());
}

int ClientPathFinder::getGrid(int argX, int argY) const
{
    if(!m_proc->validC(argX, argY)){
        return PF_NONE;
    }

    if(const auto p = m_cache.find({argX, argY}); p != m_cache.end()){
        return p->second;
    }
    return m_cache[{argX, argY}] = m_proc->checkPathGrid(argX, argY);
}
