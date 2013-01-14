/*
 * AffordanceUpWrapper.cpp
 *
 *  Created on: Jan 14, 2013
 *      Author: mfleder
 */

#include "AffordanceUpWrapper.h"

using namespace boost;
using namespace std;

namespace affordance {

/**We use the given lcm object to subscribe.  We assume some other part of the code needs to
 * while(true)lcm->handle()*/
AffordanceUpWrapper::AffordanceUpWrapper(const boost::shared_ptr<lcm::LCM> lcm)
	: _lcm(lcm), _affordances(), _accessMutex()
{
	lcm->subscribe(AffordanceServer::AFF_SERVER_CHANNEL,
			       &AffordanceUpWrapper::handleCollectionMsg, this);
}


AffordanceUpWrapper::~AffordanceUpWrapper()
{
	//nothing to do
}


//============accessors
/**@return all affordances stored on the server*/
void AffordanceUpWrapper::getAllAffordances(std::vector<AffPtr> &affs)
{
	_accessMutex.lock(); //==========lock
	affs = _affordances; //assignment operator
	_accessMutex.unlock(); //unlock
}



//=============mutators
/**@param aff affordance to add to the server store.  or, if this affordance already
	 * exists, then we are replacing the existing affordance.
	 * Note that changes will only be refelcted in getAllAffordances once
	 * the server accepts the addition/replacement and then pushes
	 * the resulting affordance collection to this object.*/
void AffordanceUpWrapper::addOrReplace(const AffordanceState &aff)
{
	_accessMutex.lock(); //=========lock

	drc::affordance_t msg;
	aff.toMsg(&msg);
	_lcm->publish(AffordanceServer::AFFORDANCE_ADD_REPLACE_CHANNEL, &msg);

	_accessMutex.unlock(); //========unlock
}



/**receives affordance collection message from server -- which represents
 * all affordances on the server.*/
void AffordanceUpWrapper::handleCollectionMsg(const lcm::ReceiveBuffer* rbuf, const std::string& channel,
						 	 	 	 	 	  const drc::affordance_collection_t *collection)
{
	_accessMutex.lock(); //========lock

	_affordances.clear();
	for(uint i = 0; i < collection->affs.size(); i++)
	{
		AffPtr next(new AffordanceState(&collection->affs[i]));
		_affordances.push_back(next);
	}

	_accessMutex.unlock(); //=======unlock
}

} /* namespace affordance */
