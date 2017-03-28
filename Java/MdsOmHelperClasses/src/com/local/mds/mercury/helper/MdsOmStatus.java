package com.jpmorgan.mds.mercury.helper;

@SuppressWarnings("serial")
public class MdsOmStatus extends Exception {
	private short status;
	
	public MdsOmStatus (short status) {
		this.status = status;
	}
	
	/** No Envs */
    public static final short OK								= 0;
	/** No Envs */
    public static final short NO_ENVS							= 1;
    /** Invalid Env */
    public static final short INVALID_ENV						= 2;
    /** Bridge did not connect */
    public static final short BRIDGE_DID_NOT_CONNECT			= 3;
    /** Bridge Timeout */
    public static final short BRIDGE_TIMEOUT					= 4;
    /** Dictionary did not connect */
    public static final short DICTIONARY_DID_NOT_CONNECT		= 5;
    /** Dictionary Timeout */
    public static final short DICTIONARY_TIMEOUT				= 6;
    /** Dictionary no source */
    public static final short DICTIONARY_NO_SOURCE				= 7;
    /** Null Arg */
    public static final short NULL_ARG							= 8;
    /** No transports available */
    public static final short NO_TRANSPORTS_AVAILABLE			= 9;
    /** Transports not provided */
    public static final short TRANSPORT_NOT_PROVIDED			= 10;
    /** No queues available */
    public static final short NO_QUEUES_AVAILABLE				= 11;
    /** Invalid config */
    public static final short INVALID_CONFIG 					= 12;
    /** Cannot find source in config */
    public static final short CANNOT_FIND_SOURCE_IN_CONFIG		= 13;
    /** Mama properties file not found */
    public static final short PROPERTIES_FILE_NOT_FOUND			= 14;
    /** The format of the chain symbol is not Reuters compliant */
    public static final short CHAIN_BAD_FORMAT					= 15;
    /** The requested chain template was not found (usually located in mama.properties) */
    public static final short CHAIN_TEMPLATE_NOT_FOUND			= 16;
    /** One of the links in the chain was not found in the infra */
    public static final short CHAIN_NOT_FOUND					= 17;
    /** The format of the chain symbol does not conform to Reuters format */
    public static final short CHAIN_BAD_INDEX					= 18;
    /** One of the links in the chain returned not-entitles, permission denied to subscription */
    public static final short CHAIN_NOT_ENTITLED				= 19;
    /** General chain error */
    public static final short CHAIN_ERROR						= 20;
    /* One of the links in the chain was deleted by the publisher */
    public static final short CHAIN_LINK_DELETE					= 21;
    /* Timeout on a link */
    public static final short CHAIN_LINK_TIMEOUT				= 22;
    /* NO DACS hosts in mama.properties */
    public static final short DACS_NO_HOSTS						= 23;
    /* DACS host connections failed */
    public static final short DACS_CONNECT_FAILED				= 24;
    /** Unknow error */
    public static final short UNKNOWN							= 25;

    /**
     * Return a text description of the message's status.
     *
     * @return The description.
     */
    public static String stringForStatus(final short type)
    {
        switch (type) {
        case NO_ENVS:							return "MDS_OM_STATUS_NO_ENVS";
        case INVALID_ENV:						return "MDS_OM_STATUS_INVALID_ENV";     
        case BRIDGE_DID_NOT_CONNECT:			return "MDS_OM_STATUS_BRIDGE_DID_NOT_CONNECT";
        case BRIDGE_TIMEOUT:					return "MDS_OM_STATUS_BRIDGE_TIMEOUT";
        case DICTIONARY_DID_NOT_CONNECT:		return "MDS_OM_STATUS_DICTIONARY_DID_NOT_CONNECT";
        case DICTIONARY_TIMEOUT:				return "MDS_OM_STATUS_DICTIONARY_TIMEOUT";
        case DICTIONARY_NO_SOURCE:				return "MDS_OM_STATUS_DICTIONARY_NO_SOURCE";
        case NULL_ARG:							return "MDS_OM_STATUS_NULL_ARG";
        case NO_TRANSPORTS_AVAILABLE:			return "MDS_OM_STATUS_NO_TRANSPORTS_AVAILABLE";            
        case TRANSPORT_NOT_PROVIDED:			return "MDS_OM_STATUS_TRANSPORT_NOT_PROVIDED";                
        case NO_QUEUES_AVAILABLE:				return "MDS_OM_STATUS_NO_QUEUES_AVAILABLE";
        case INVALID_CONFIG:					return "MDS_OM_STATUS_INVALID_CONFIG";    
        case CANNOT_FIND_SOURCE_IN_CONFIG:		return "MDS_OM_STATUS_CANNOT_FIND_SOURCE_IN_CONFIG";
        case PROPERTIES_FILE_NOT_FOUND:         return "PROPERTIES_FILE_NOT_FOUND";
        case CHAIN_BAD_FORMAT:          		return "CHAIN_BAD_FORMAT";
        case CHAIN_TEMPLATE_NOT_FOUND:          return "CHAIN_TEMPLATE_NOT_FOUND";
        case CHAIN_NOT_FOUND:           		return "CHAIN_NOT_FOUND";
        case CHAIN_BAD_INDEX:           		return "CHAIN_BAD_INDEX";
        case CHAIN_NOT_ENTITLED:                return "CHAIN_NOT_ENTITLED";
        case CHAIN_ERROR:               		return "CHAIN_ERROR";
        case CHAIN_LINK_DELETE:         		return "CHAIN_LINK_DELETE";
        case CHAIN_LINK_TIMEOUT:         		return "CHAIN_LINK_TIMEOUT";
        case DACS_NO_HOSTS:         			return "DACS_NO_HOSTS";
        case DACS_CONNECT_FAILED:         		return "DACS_CONNECT_FAILED";
        case UNKNOWN:           				return "UNKNOWN";
        default: 								return "UNKNOWN";
        }
    }
    
    @Override
    public String toString() {
    	return stringForStatus(status);
    }
    
    public short getStatus() {
    	return status;
    }
} 
