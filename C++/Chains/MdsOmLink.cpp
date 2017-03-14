//******************************************************************************
// Implementation module for chain MdsOmLink class
//******************************************************************************

#include "MdsOmChains.h"
#include "MdsOmChainInternal.h"

namespace MdsOmNs {

//******************************************************************************
// Constructor
//******************************************************************************
MdsOmLink::MdsOmLink() :
				sub(NULL), 
				status(MDS_OM_STATUS_OK),
				_nNumUpdates(0),
				dirty(false),
				pub(NULL),
				msgType(MAMA_MSG_TYPE_UNKNOWN)
{
	theChain = NULL;
	chainConfig = NULL;
	subMsg = NULL;

	wthread_mutexattr_t attr;
	wthread_mutexattr_init (&attr);
	wthread_mutexattr_settype (&attr, WTHREAD_MUTEX_RECURSIVE);
	wthread_mutex_init(&elementListLock, &attr);
}

//******************************************************************************
// Initialise link
//******************************************************************************
int MdsOmLink::init(MdsOmChain* chain,
					ChainConfig* chainConf,
					const char *source,
					const char *symbol) 
{
	theChain = chain;

	chainConfig = chainConf;

	this->source = source;
	this->symbol = symbol;
	this->subject = this->source + "." + this->symbol;

	return 0;
}

//******************************************************************************
// Destructor - destroy child objects(the elements)
//******************************************************************************
MdsOmLink::~MdsOmLink()
{
	clearElements();

	// This is invoked by the onDestroy for the sub
	wthread_mutex_destroy(&elementListLock);
}

void MdsOmLink::destroy()
{
	if (pub) {
		MamaMsg* m = pub->retrieveMamaMsg();
		if (m) pub->destroyMamaMsg(m);
		pub->destroy();
		// The publisher is now gone, cannot be used again.
		pub = NULL;
	}

	if (sub) {
		// Cancel subscripton (if active) and destroy subject
		unsubscribe();
		// The subscriber is now gone, cannot be used again
	} else {
		// No sub, don't wait for onDestroy, delete now
		delete this;
	}
}

const char* MdsOmLink::getSubject() const { return subject.c_str(); }

const char* MdsOmLink::getSymbol() const { return symbol.c_str(); }

const char* MdsOmLink::getSource() const { return source.c_str(); }

MamaMsg* MdsOmLink::getPubMamaMsg() const 
{
	if (pub == NULL) return NULL;
	dirty = true;
	return pub->retrieveMamaMsg();
}

MamaMsg* MdsOmLink::getSubMamaMsg() const
{
	return subMsg;
}

// ----------------------------------------------------------------------------
MamaMsg* MdsOmLink::setupPub()
{
	MamaMsg* msg = NULL;
	if (pub == NULL) {
		pub = theChain->om->getPublisher(subject.c_str(), theChain->getPublisherCallback(), theChain->getPublisherClosure());
		msg = pub->getNewMamaMsg();		// create mama msg
		pub->storeMamaMsg(msg);			// store in publisher for later
		env = pub->getEnv();
		dictionary = env->getDictionary();
	} else {
		msg = pub->retrieveMamaMsg();	// get previously created msg
		msg->clear();
	}
	return msg;
}

//******************************************************************************
// Publish the link
// pref - prefdisplay field for Reuters
// rdn - rdndisplay field for Reuters
//******************************************************************************
void MdsOmLink::publish(bool firstPublish)
{
	publishOne(firstPublish);
	publishTwo(firstPublish);
}

void MdsOmLink::publishOne(bool firstPublish)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmLink::Publish: %s", getSubject());

	setupPub();
}

void MdsOmLink::publishTwo(bool firstPublish)
{
	MamaMsg* msg = pub->retrieveMamaMsg();
	MamaFieldDescriptor* field = NULL;

	//--------------------------------------------------------------------------
	if (!dirty && firstPublish == false) {
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmLink::Publish: %s link not modified, don't publish", this->getSubject());
		return;
	}

	char** chainFields = chainConfig->getLinkArray();
	int i = 0;
	{
		MdsOmLock lock(&elementListLock);
		ElementList::iterator it = elementList.begin();
		while (it != elementList.end()) {
			MdsOmElement* e = *it++;
			field = dictionary->getFieldByName(chainFields[i]);		
			if (field != NULL) {
				msg->addString("", field->getFid(), e->getName());
			}
			i++;
		}
	}

	// Fill out the last fields if there are less than 14 elements
	for (; i < chainConfig->getMaxNumLinks(); ++i) {
		field = dictionary->getFieldByName(chainFields[i]);		
		if (field != NULL) {
			msg->addString("", field->getFid(), "");
		}
	}

	// Set NEXT_LR and PREV_LR fields
	field = dictionary->getFieldByName(chainConfig->getNextLinkName());		
	if (field != NULL) {
		msg->addString("", field->getFid(), nextLinkName.c_str());
	}

	field = dictionary->getFieldByName(chainConfig->getPrevLinkName());		
	if (field != NULL) {
		msg->addString("", field->getFid(), prevLinkName.c_str());
	}

	// PREF_LINK
	field = dictionary->getFieldByName(chainConfig->getPrefLink());		
	if (field != NULL) {
		msg->addString("", field->getFid(), nextLinkName.c_str());
	}

	// REF_COUNT
	field = dictionary->getFieldByName(chainConfig->getRefCount());		
	if (field != NULL) {
		msg->addU64("", field->getFid(), getElementCount());
	}

	mamaMsgType msgType = MAMA_MSG_TYPE_UPDATE;
	if (firstPublish) {
		// Send first publish as INITIAL
		msgType = MAMA_MSG_TYPE_INITIAL;
	}
	msg->addU8(MamaFieldMsgType.mName, MamaFieldMsgType.mFid, msgType);
	msg->addU8(MamaFieldMsgStatus.mName, MamaFieldMsgStatus.mFid, MAMA_MSG_STATUS_OK);

	// Publish it
	// displayMsg(env, source.c_str(), symbol.c_str(), *msg, 0);
	pub->send(msg);

	dirty = false;
}

void MdsOmLink::displayMsg(MdsOmEnv* env, const char* source, const char* symbol, const MamaMsg& msg, int indent)
{
	size_t numFields = msg.getNumFields();
	int bump = 2;

	mamaMsgType type = MAMA_MSG_TYPE_UNKNOWN;
	try { type = msg.getType(); } catch(...) { }

	mamaMsgStatus status = MAMA_MSG_STATUS_UNKNOWN;
	try { status = msg.getStatus(); } catch(...) { }

	char strBuffer[256];

	if (indent == 0) {
		printf("--- %s.%s payload=%c type=%s status=%s fields=%d ----------------------------\n",
			source, symbol,
			msg.getPayloadType(), mamaMsgType_stringForType(type), mamaMsgStatus_stringForStatus(status), numFields);
	} else {
		printf("%*s{\n", indent - bump, "");
	}

	MamaMsgIterator* it = new MamaMsgIterator();
	msg.begin(*it);
	for (; *(*it) != NULL; ++(*it)) {
		try {
			MamaMsgField field = *(*it);

			if (field == NULL) continue;

			mama_fid_t fid = field.getFid();
			const char* name = getFieldName(env, field, fid);

			printf("%*s%*d %-11s%20s ", indent, "", 5, fid, field.getTypeName(), name);

			mamaFieldType fieldType = field.getType();
			switch (fieldType) {
			case MAMA_FIELD_TYPE_VECTOR_MSG: {
				const MamaMsg** vmsg;
				mama_size_t len;
				field.getVectorMsg(vmsg, len);
				printf("%d\n", len);
				for (mama_size_t i = 0; i < len; i++) {
					displayMsg(env, source, symbol, *vmsg[i], indent + bump);
					printf("%*s}\n", indent, "");
				}
				break;
			}
			case MAMA_FIELD_TYPE_MSG: {
				MamaMsg tmp;
				field.getMsg(tmp);
				printf("\n");
				displayMsg(env, source, symbol, tmp, indent + bump);
				break;
			}
			case MAMA_FIELD_TYPE_BOOL: {
				mama_bool_t v = field.getBool();
				printf("%d\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_CHAR: {
				char v = field.getChar();
				printf("%c(%d)\n", v, v); 
				break;
			}
			case MAMA_FIELD_TYPE_I8: {
				mama_i8_t v = field.getI8();
				printf("%d(%c)\n", v, v);
				break;
			}
			case MAMA_FIELD_TYPE_U8: {
				mama_u8_t v = field.getU8();
				printf("%u\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_I16: {
				mama_i16_t v = field.getI16();
				printf("%d\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_U16: {
				mama_u16_t v = field.getU16();
				printf("%u\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_I32: {
				mama_i32_t v = field.getI32();
				printf("%d\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_U32: {
				mama_u32_t v = field.getU32();
				printf("%u\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_I64: {
				mama_i64_t v = field.getI64();
				printf("%lld\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_U64: {
				mama_u64_t v = field.getU64();
				printf("%llu\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_F32: {
				mama_f32_t v = field.getF32();
				printf("%.4f\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_F64: {
				mama_f64_t v = field.getF64();
				printf("%.4lf\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_STRING: {
				const char* v = field.getString();
				printf("'%s'\n", v);
				break;
			}
			case MAMA_FIELD_TYPE_TIME: {
				MamaDateTime mamaDateTime;
				field.getDateTime(mamaDateTime);
				mamaDateTime.getAsString(strBuffer, sizeof(strBuffer));
				printf("%s(%d %d)\n", strBuffer, mamaDateTime.hasDate(), mamaDateTime.hasTime());
				break;
			}
			case MAMA_FIELD_TYPE_PRICE: {
				MamaPrice mamaPrice;
				field.getPrice(mamaPrice);
				double v = mamaPrice.getValue();

				// Get floating point decimal points from mama precision
				mamaPricePrecision prec = mamaPrice.getPrecision();
				int dp =(int) prec;
				if (prec == MAMA_PRICE_PREC_INT) dp = 0;
				else if (prec > MAMA_PRICE_PREC_10000000000) dp =(int) MAMA_PRICE_PREC_10000000000;

				printf("%.*lf(%d %d)\n", dp, v, prec, mamaPrice.getIsValidPrice());
				break;
			} 
			case MAMA_FIELD_TYPE_OPAQUE: {
				size_t len = 0;
				const void* data = field.getOpaque(len);
				printf("%d\n", len);
				break;
			}
			default:
				strBuffer[0] = '\0';
				field.getAsString(strBuffer, sizeof(strBuffer));
				printf("%s\n", strBuffer);
				break; 
			}
		} catch(MamaStatus& status) {
			printf("%s\n", status.toString());
		} catch(MdsOmStatus& status) {
			printf("%s\n", status.toString());
		}
	}
	delete it;
}
	
const char* MdsOmLink::getFieldName(MdsOmEnv* env, MamaMsgField field, mama_fid_t fid)
{
	const char* name = NULL;
	try {
		// Try the dictionary
		name = env->getFieldName(fid);
		if (name == NULL) {
			// Try the field itself
			name = field.getName();
		}
	} catch(...) {
	}
	if (name == NULL || *name == '\0') name = "(none)";
	return name;
}

//******************************************************************************
//
// Drop the link
//
//******************************************************************************
void MdsOmLink::drop()
{
	MamaMsg* msg = setupPub();

	msg->addU8(MamaFieldMsgType.mName, MamaFieldMsgType.mFid, MAMA_MSG_TYPE_DELETE);
	msg->addU8(MamaFieldMsgStatus.mName, MamaFieldMsgStatus.mFid, MAMA_MSG_STATUS_OK);
		
	// Send msg
	pub->send(msg);
}

//******************************************************************************
// Subscribe to the links subject
//******************************************************************************
void MdsOmLink::subscribe()
{
	_nNumUpdates = 0;

	mama_log(MAMA_LOG_LEVEL_FINE, "Chain Subscribe: %s", source.c_str(), symbol.c_str());	

	sub = theChain->om->subscribe(source.c_str(), symbol.c_str(), this, NULL, true);
	env = sub->getEnv();
	dictionary = env->getDictionary();
	sub->activate();
}

//******************************************************************************
// Capture to the links subject
//******************************************************************************
void MdsOmLink::capture()
{
	_nNumUpdates = 0;

	//mama_log(MAMA_LOG_LEVEL_NORMAL, "Capture: %s", source.c_str(), symbol.c_str());	

	sub = theChain->om->snap(source.c_str(), symbol.c_str(), this, NULL, true);
	env = sub->getEnv();
	dictionary = env->getDictionary();
	sub->activate();
}

//******************************************************************************
// Unsubscribe from the links Teknekron subject
//******************************************************************************
void MdsOmLink::unsubscribe()
{
	if (sub) {
		sub->destroy();
		sub = NULL;
	}
}

bool MdsOmLink::addElement(const char* name)
{
	if (isLinkFull())
		 return false;

	MdsOmElement* e = new MdsOmElement();
	e->init(name);
	{
		MdsOmLock lock(&elementListLock);
		elementList.push_back(e);
	}

	return true;
}

//******************************************************************************
//
// Return if the link is full or not
//
//******************************************************************************
bool MdsOmLink::isLinkFull() const
{
	return (int) getElementCount() >= chainConfig->getMaxNumLinks();
}

//******************************************************************************
//
// Get the first element in this link
//
//******************************************************************************
MdsOmList<const char*>* MdsOmLink::getElements(MdsOmList<const char*>* l)
{
	MdsOmLock lock(&elementListLock);
	if (l == NULL) l = new MdsOmList<const char*>();
	else l->clear();
	ElementList::iterator it = elementList.begin();
	while (it != elementList.end()) {
		MdsOmElement* e = *it++;
		l->add(e->getName());
	}
	return l;
}

void MdsOmLink::clearElements()
{
	MdsOmLock lock(&elementListLock);
	ElementList::iterator itx = elementList.begin();
	while (itx != elementList.end()) {
		MdsOmElement* e = *itx++;
		delete e;
	}
	elementList.clear();
}

//******************************************************************************
// Set the name of the next link
//******************************************************************************
void MdsOmLink::setNextLinkName(const char *pszNextLinkName)
{
	nextLinkName = pszNextLinkName;
}

//******************************************************************************
// Set name of previous link in chain
//******************************************************************************
void MdsOmLink::setPrevLinkName(const char *pszPrevLinkName)
{
	prevLinkName = pszPrevLinkName;
}

MdsOmChainLinkType MdsOmLink::getLinkType() const { return linkType; }

void MdsOmLink::setLinkType(MdsOmChainLinkType t) { linkType = t; }

MdsOmStatusCode MdsOmLink::getStatus() const { return status; };

const char* MdsOmLink::getChainLinkTypeText(MdsOmChainLinkType n) const {
	switch (n) {
		case MDSOM_CHAIN_LINK_TYPE_UNKNOWN:  return "MDS_CHAIN_LINK_TYPE_UNKNOWN";  break;
		case MDSOM_CHAIN_LINK_TYPE_LINK:     return "MDS_CHAIN_LINK_TYPE_LINK";		break;
		case MDSOM_CHAIN_LINK_TYPE_LONGLINK: return "MDS_CHAIN_LINK_TYPE_LONGLINK"; break;
	}
	return "Not Recognized???";
}

size_t MdsOmLink::getElementCount() const { return(int) elementList.size(); }

size_t MdsOmLink::getMaxElements() const { return chainConfig->getMaxNumLinks(); }

size_t MdsOmLink::getTotalNumUpdates() const { return _nNumUpdates; };

// methods that mama needs
void MdsOmLink::onCreate(MamaSubscription* subscriber)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "onCreate: %s", subscriber->getSymbol());
}

void MdsOmLink::onError(MamaSubscription* subscription, const MamaStatus& mstatus, const char* subject)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "onError: %s %s", subject, mstatus.toString());

	MdsOmStatusCode code;
	switch (mstatus.getStatus()) {
	case MAMA_STATUS_OK:				code = MDS_OM_STATUS_OK; break;
	case MAMA_STATUS_NOT_ENTITLED:		code = MDS_OM_STATUS_CHAIN_NOT_ENTITLED; break;
	case MAMA_STATUS_NOT_PERMISSIONED:	code = MDS_OM_STATUS_CHAIN_NOT_ENTITLED; break;
	case MAMA_STATUS_BAD_SYMBOL:		code = MDS_OM_STATUS_CHAIN_BAD_FORMAT; break;
	case MAMA_STATUS_NOT_FOUND:			code = MDS_OM_STATUS_CHAIN_NOT_FOUND; break;
	case MAMA_STATUS_TIMEOUT:			code = MDS_OM_STATUS_CHAIN_LINK_TIMEOUT; break;
	case MAMA_STATUS_DELETE:			code = MDS_OM_STATUS_CHAIN_LINK_DELETE; break;
	default:							code = MDS_OM_STATUS_CHAIN_ERROR; break;
	}
	status = code;

	elementList.clear();
	theChain->errorNextLink(this, code, getSubject());
}

void MdsOmLink::onQuality(MamaSubscription* subscription, mamaQuality quality, const char* symbol, short cause, const void* platformInfo)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "%s onQuality callback", getSubject());
	theChain->onQualityLink(this, quality, getSubject());
}

void MdsOmLink::onDestroy(MamaSubscription* subscription)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "%s onDestroy callback", getSubject());
}

void MdsOmLink::onGap(MamaSubscription* subscription)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "%s onGap callback", getSubject());
}

void MdsOmLink::onRecapRequest(MamaSubscription* subscription)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "%s onRecapRequest callback", getSubject());
}

void MdsOmLink::onMsg(MamaSubscription* subscription, MamaMsg& msg)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "onMsg: %s", subject.c_str());

	subMsg = &msg;

	_nNumUpdates++;
	msgType = msg.getType();

	// displayMsg(env, source.c_str(), symbol.c_str(), msg, 0);

	// we have data
	switch (msg.getType()) {
		case MAMA_MSG_TYPE_DELETE:
		case MAMA_MSG_TYPE_EXPIRE:
			mama_log(MAMA_LOG_LEVEL_ERROR, "Symbol deleted or expired. No more subscriptions %s", subject.c_str());
			unsubscribe();
			// These actually do not get sent here, they come in onError() cb.
			return;
		default: 
			break;
	}

	switch (msg.getStatus()) {
		case MAMA_MSG_STATUS_BAD_SYMBOL:
		case MAMA_MSG_STATUS_EXPIRED:
		case MAMA_MSG_STATUS_TIMEOUT:
		case MAMA_MSG_STATUS_NOT_PERMISSIONED:
		case MAMA_MSG_STATUS_NOT_ENTITLED:
		case MAMA_MSG_STATUS_NOT_FOUND:
			mama_log(MAMA_LOG_LEVEL_ERROR, "Symbol error. No more subscriptions %s %d", subject.c_str(), msg.getStatus());
			unsubscribe();
			return;
		default:
			break;
	}

	// we should have a valid subscription
	// look for field names to see what type of links exist
	checkForChain(msg, subscription);

	subMsg = NULL;
}

void MdsOmLink::checkForChain(MamaMsg& msg, MamaSubscription* subscription)
{
	MamaFieldDescriptor* field = NULL;
	mamaMsgType type = msg.getType();
	const char* symbol = subscription->getSymbol();
	int refCount = 0;

	linkType = MDSOM_CHAIN_LINK_TYPE_UNKNOWN;

	// check for REF_COUNT - if its not there its not a chain
	field = dictionary->getFieldByName("REF_COUNT");
	if (field == NULL) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "Dictionary is missing required field 'REF_COUNT'");
		return;
	}

	// REF_COUNT exists - print chain links
	if (msg.tryField(field)) {
		refCount =(int) msg.getU64(field);
		mama_log(MAMA_LOG_LEVEL_FINE, "REF_COUNT = %d", refCount);
	} else {
		mama_log(MAMA_LOG_LEVEL_ERROR, "Message %s is missing required field 'REF_COUNT'", symbol);
		return;
	}

	nextLinkName = "";

	// determine what LINK type to use
	// it could be LINK_x or LONGLINKx
	// see if LINK_1 exists
	field = dictionary->getFieldByName("LINK_1");
	if (field != NULL) {
		// it exists in dictionary, is it in message ?
		if (msg.tryField(field)) {
			linkType = MDSOM_CHAIN_LINK_TYPE_LINK;

			if ((field = dictionary->getFieldByName("NEXT_LR")) != NULL) {
				if (msg.tryField(field)) {
					const char* p = msg.getString(field);
					nextLinkName = p;
					nextLinkName = MdsTrim(nextLinkName);
				}
			}

			if ((field = dictionary->getFieldByName("PREV_LR")) != NULL) {
				if (msg.tryField(field) == true) {
					const char* p = msg.getString(field);
					prevLinkName = p;
					prevLinkName = MdsTrim(prevLinkName);
				}
			}
		}
	}

	if (linkType == MDSOM_CHAIN_LINK_TYPE_UNKNOWN) {
		// try LONGLINK1
		field = dictionary->getFieldByName("LONGLINK1");
		if (field != NULL) {
			// it exists in dictionary
			if (msg.tryField(field)) {
				linkType = MDSOM_CHAIN_LINK_TYPE_LONGLINK;

				if ((field = dictionary->getFieldByName("LONGNEXTLR")) != NULL) {
					if (msg.tryField(field)) {
						const char* p = msg.getString(field);
						nextLinkName = p;
						nextLinkName = MdsTrim(nextLinkName);
					}
				}

				if ((field = dictionary->getFieldByName("LONGPREVLR")) != NULL) {
					if (msg.tryField(field)) {
						const char* p = msg.getString(field);
						prevLinkName = p;
						prevLinkName = MdsTrim(prevLinkName);
					}
				}
			}
		}
	}

	// Does not use recognized link names, can't continue
	if (linkType == MDSOM_CHAIN_LINK_TYPE_UNKNOWN) return;

	if (env->getType() == MDS_OM_ENV_MERCURY && nextLinkName.size() > 0) {
		// BAD BAD BAD - for Solace the '.' in a chain next needs to be a '^'
		// NEXT_LR '1#PB.ANY.EMEA_CLIENT.DELTA1.DELTA1' 
		bool foundExchange = false;
		char* p = (char*) nextLinkName.c_str();
		char* cp = p;
		p += strlen(p) - 1;
		while (p > cp) {
			if (foundExchange) {
				if (*p == '.') *p = '^';	// replace for Solace
			} else if (*p == '.') {
				foundExchange = true;
			}
			p--;
		}
	}

	// Clear elements before getting new ones
	clearElements();

	// Get the link data fields
	for (int i = 1; i <= refCount; i++) {
		char linkbuffer[64];
		switch (linkType) {
		case MDSOM_CHAIN_LINK_TYPE_LINK:
			sprintf(linkbuffer, "LINK_%-d", i);
			break;
		case MDSOM_CHAIN_LINK_TYPE_LONGLINK:
			sprintf(linkbuffer, "LONGLINK%-d", i);
			break;
		default:
			break;
		}

		field = dictionary->getFieldByName(linkbuffer);
		if (field != NULL) {
			// it exists in dictionary
			if (msg.tryField(field)) {
				const char* p = msg.getString(field);
				if (p) {
					bool ret = addElement(p);
				}
			}
		}
	}

	theChain->processNextLink(this);
}

}
