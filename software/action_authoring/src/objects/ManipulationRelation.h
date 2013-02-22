#ifndef MANIPULATION_RELATION_H
#define MANIPULATION_RELATION_H

#include <boost/shared_ptr.hpp>
#include <affordance/AffordanceState.h>
#include <affordance/ManipulatorState.h>
#include "RelationState.h"
#include "AtomicConstraint.h"

namespace action_authoring
{
class ManipulationRelation : public AtomicConstraint
{

    //----------Constructor
public:
    ManipulationRelation(affordance::AffConstPtr affordance,
                         affordance::ManipulatorStateConstPtr manipulator,
                         RelationStatePtr relationState);

    //----------Accessors
    affordance::AffConstPtr getAffordance() const
    {
        return _affordance;
    }
    affordance::ManipulatorStateConstPtr getManipulator() const
    {
        return _manipulator;
    }
    RelationStatePtr getRelationState() const
    {
        return _relationState;
    }

    void setAffordance(affordance::AffConstPtr affordance)
    {
        _affordance = affordance;
    }
    void setManipulator(affordance::ManipulatorStateConstPtr manipulator)
    {
        _manipulator = manipulator;
    }
    void setRelationState(RelationStatePtr relationState)
    {
        _relationState = relationState;
    }

    //------------Fields
private:
    affordance::AffConstPtr _affordance;
    affordance::ManipulatorStateConstPtr _manipulator;
    RelationStatePtr _relationState;

}; // class ManipulationRelation
typedef boost::shared_ptr<ManipulationRelation> ManRelPtr;
typedef boost::shared_ptr<const ManipulationRelation> ManRelConstPtr;

} //namespace action_authoring

#endif //MANIPULATION_RELATION_H
