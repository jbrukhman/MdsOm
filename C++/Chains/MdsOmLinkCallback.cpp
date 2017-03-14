//
// chainpubsub.cpp
//
// this is the chain helper class for OpenMAMA
//
#pragma warning(disable : 4996)

#include "MdsOmChains.h"

namespace MdsOmNs {

// methods that mama needs
void MdsOmLink::onCreate(MamaSubscription* subscriber)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "onCreate: %s", subscriber->getSymbol());
}

void MdsOmLink::onError(MamaSubscription* subscription, const MamaStatus& status, const char* subject)
{
	mama_log(MAMA_LOG_LEVEL_NORMAL, "onError: %s %s", subject, status.toString());
}

void MdsOmLink::onMsg(MamaSubscription*  subscription, MamaMsg& msg)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "onMsg: ");

	// we have data
	switch (msg.getType()) {
		case MAMA_MSG_TYPE_DELETE:
		case MAMA_MSG_TYPE_EXPIRE:
			//mMamaListen->removeSubscription (subscription);
			//subscription->destroy();
			//delete subscription;

			//if (mMamaListen->hasSubscriptions ())
			//{
			mama_log (MAMA_LOG_LEVEL_ERROR, "Symbol deleted or expired. No more subscriptions\n");
			//    exit(1);
			//}
			return;

		default: 
			break;
	}

	switch (msg.getStatus()) {
		case MAMA_MSG_STATUS_BAD_SYMBOL:
		case MAMA_MSG_STATUS_EXPIRED:
		case MAMA_MSG_STATUS_TIMEOUT:
			/*        mMamaListen->removeSubscription (subscription);
			subscription->destroy();
			delete subscription;

			if (mMamaListen->hasSubscriptions ())
			{*/
			mama_log (MAMA_LOG_LEVEL_ERROR,"Symbol deleted or expired. No more subscriptions\n");
			//    exit(1);
			//}
			return;
		default:
			break;
	}

	// we should have a valid subscription
	// look for field names to see what type of links exist
	//mchaintype = CheckForChain(msg, subscription);

	//GetNext(msg, subscription, mchaintype);
}

void MdsOmLink::onQuality(MamaSubscription* subscription, mamaQuality quality, const char* symbol, short cause, const void* platformInfo)
{
	mama_log(MAMA_LOG_LEVEL_NORMAL, "onQuality callback");
}

void MdsOmLink::onGap(MamaSubscription*  subscription)
{
	mama_log(MAMA_LOG_LEVEL_NORMAL, "onGap callback");
}

void MdsOmLink::onRecapRequest(MamaSubscription*  subscription)
{
	mama_log(MAMA_LOG_LEVEL_NORMAL, "onRecapRequest callback");
}

bool MdsOmLink::GetNext(MamaMsg& msg, MamaSubscription* subscription, ChainType ct)
{
	char  linkvalue[65];
	bool  exists = false;

	linkvalue[0] = '\0';

	// check link type to get 'next' field name
	if (ct == ChainTypeLink) {
		MamaFieldDescriptor* field = dictionary->getFieldByName("NEXT_LR");
		if (field != NULL) {
			exists = msg.tryField(field);
			if (exists) {
				msg.getFieldAsString(field, linkvalue, 64);
			}
		}
	} else {
		// its longlink
		MamaFieldDescriptor* field = dictionary->getFieldByName("LONGNEXTLR");
		if (field != NULL) {
			exists = msg.tryField(field);
			if (exists) {
				msg.getFieldAsString(field, linkvalue, 64);
			}
		}
	}

	if (exists && strlen(linkvalue) > 0) {
		// we have 'next' link
		// determine which subscription type to use
		mama_log(MAMA_LOG_LEVEL_NORMAL, "Subscribe to link %s.%s", source.c_str(), linkvalue);
#ifndef USETIMERSUB
		MdsOmSubscription* sub = env->subscribe(source.c_str(), linkvalue, this, NULL);
#else
		mnextitemname = strdup(linkvalue);
		mnextsub = true;
#endif
		//mSubscriptionList.push_back(sub);
		return true;
	}
	mcomplete = true;
	mama_log(MAMA_LOG_LEVEL_NORMAL, "Subscription complete");
	cb->MdsOmChainOnSuccess(this);

	return false;
}

ChainType MdsOmLink::CheckForChain(MamaMsg& msg, MamaSubscription* subscription)
{
	MamaFieldDescriptor* field = NULL;
	ChainType chaintype = ChainTypeUnknown;
	int  refCount = 0;
	char  linkbuffer[12];
	char  linkvalue[65];

	// check for REF_COUNT - if its not there its not a chain
	field = dictionary->getFieldByName("REF_COUNT");
	if (field == NULL) {
		mama_log(MAMA_LOG_LEVEL_NORMAL, "Dictionary is missing required field 'REF_COUNT'");
		return ChainTypeUnknown;
	}

	// REF_COUNT exists - print chain links
	if (msg.tryField(field)) {
		refCount = msg.getI32(field);
		// mama_log(MAMA_LOG_LEVEL_NORMAL, "REF_COUNT = %d", refCount);
	} else {
		mama_log(MAMA_LOG_LEVEL_NORMAL, "Message is missing required field 'REF_COUNT'");
		return ChainTypeUnknown;
	}

	// determine what LINK type to use
	// it could be LINK_x or LONGLINKx
	// see if LINK_1 exists
	field = dictionary->getFieldByName("LINK_1");
	if (field != NULL) {
		// it exists in dictionary, is it in message ?
		if (msg.tryField(field)) {
			msg.getFieldAsString(field, linkvalue, 64);
			MdsOmLink* link = new MdsOmLink();			// TODO add call to init()
			linkList.push_back(link);

			// save actual link values for change testing
			MdsOmLink* link = new MdsOmLink();
			linkList.push_back(link);

			if ((field = dictionary->getFieldByName("NEXT_LR")) != NULL) {
				if (msg.tryField(field)) {
					msg.getFieldAsString(field, linkvalue, 64);
					ch->AddNext(linkvalue);
				}
			}

			if ((field = dictionary->getFieldByName("PREV_LR")) != NULL) {
				if (msg.tryField(field) == true) {
					msg.getFieldAsString(field, linkvalue, 64);
					ch->AddPrev(linkvalue);
				}
			}
			chaintype = ChainTypeLink;
		} else {
			chaintype = ChainTypeUnknown;
		}
	}

	if (chaintype == ChainTypeUnknown) {
		// try LONGLINK1
		field = dictionary->getFieldByName("LONGLINK1");
		if (field != NULL) {
			// it exists in dictionary
			if (msg.tryField(field)) {
				msg.getFieldAsString(field, linkvalue, 64);
				MdsOmLink* link = new MdsOmLink();			// TODO add call to link.init()
				linkList.push_back(link);
				ch = new MdsOmChainHeader((char*) subscription->getSymbol());
				ch->AddLink(linkvalue);
				chaintype = ChainTypeLonglink;

				if ((field = dictionary->getFieldByName("LONGNEXTLR")) != NULL) {
					if (msg.tryField(field)) {
						msg.getFieldAsString(field, linkvalue, 64);
						ch->AddNext(linkvalue);
					}
				}

				if ((field = dictionary->getFieldByName("LONGPREVLR")) != NULL) {
					if (msg.tryField(field)) {
						msg.getFieldAsString(field, linkvalue, 64);
						ch->AddPrev(linkvalue);
					}
				}
			} else {
				return ChainTypeUnknown;
			}
		}
	}


	// find the other fields
	for(int i = 2; i <= refCount; i++) {
		switch (chaintype) {
		case ChainTypeLink:
			sprintf(linkbuffer,"LINK_%-d",i);
			field = dictionary->getFieldByName(linkbuffer);
			if (field != NULL) {
				// it exists in dictionary
				if (msg.tryField(field)) {
					msg.getFieldAsString(field, linkvalue, 64);
					MdsOmLink* link = new MdsOmLink();			// TODO add call to link.init()
					linkList.push_back(link);
					ch->AddLink(linkvalue);
				}
			}

			break;
		case ChainTypeLonglink:
			sprintf(linkbuffer,"LONGLINK%-d",i);
			field = dictionary->getFieldByName(linkbuffer);
			if (field != NULL) {
				// it exists in dictionary
				if (msg.tryField(field)) {
					msg.getFieldAsString(field, linkvalue, 64);
					MdsOmLink* link = new MdsOmLink();			// TODO add call to link.init()
					linkList.push_back(link);
					ch->AddLink(linkvalue);
				}
			}
			break;
		default:
			break;
		}
	}

	// check if the header already exists - if so, its an update
	// mChainheader.insert(make_pair((char*) subscription->getSymbol(), ch));

	return chaintype;
}

}
