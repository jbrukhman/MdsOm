package com.jpmorgan.mds.mercury.jms;

import javax.jms.JMSException;
import javax.jms.MapMessage;

import com.wombat.mama.MamaMsgStatus;
import com.wombat.mama.MamaStatus;

public enum MdsOmJmsMsgStatus {
	OK, STALE, CLOSED, DENIED, INVALID, UNKNOWN;
	
	public static final MdsOmJmsMsgStatus values[] = values();
	
	protected static MdsOmJmsMsgStatus code(int t) {
		// TODO check array bounds
		return MdsOmJmsMsgStatus.values[t];	
	}
	
	public static void setMamaStatus(MapMessage map, int mamaStatus) throws JMSException {
		switch (mamaStatus) {
		case MamaStatus.MAMA_STATUS_OK:
			setMessageStatus(map, MamaMsgStatus.STATUS_OK);
			break;
		case MamaStatus.MAMA_STATUS_BAD_SYMBOL:
			setMessageStatus(map, MamaMsgStatus.STATUS_BAD_SYMBOL);
			break;
		case MamaStatus.MAMA_STATUS_NOT_FOUND:
			setMessageStatus(map, MamaMsgStatus.STATUS_NOT_FOUND);
			break;
		case MamaStatus.MAMA_STATUS_NOT_ENTITLED:
			setMessageStatus(map, MamaMsgStatus.STATUS_NOT_ENTITLED);
			break;
		case MamaStatus.MAMA_STATUS_NOT_PERMISSIONED:
			setMessageStatus(map, MamaMsgStatus.STATUS_NOT_PERMISSIONED);
			break;
		default:
			setMessageStatus(map, MamaMsgStatus.STATUS_OK);
			break;
		}
	}
	
	public static MdsOmJmsMsgStatus getMessageStatus(MapMessage map) {
		try {
			int ft;
			ft = map.getInt("MdMsgStatus");
			return getMessageStatus(map, ft);
		} catch (JMSException e) {
		} catch (Exception ex) {
		}
		return UNKNOWN; 
	}
	
	public static MdsOmJmsMsgStatus getMessageStatus(MapMessage map, int msgStatus) {
		switch (msgStatus) {
		case MamaMsgStatus.STATUS_OK:
			return OK;
		case MamaMsgStatus.STATUS_STALE:
			return STALE;
		case MamaMsgStatus.STATUS_NOT_PERMISSIONED:
		case MamaMsgStatus.STATUS_NOT_ENTITLED:
			return DENIED;
		case MamaMsgStatus.STATUS_NOT_FOUND:
		case MamaMsgStatus.STATUS_BAD_SYMBOL:
			return INVALID;
		default:
			return UNKNOWN;
		}
	}
	
	public static void setMessageStatus(MapMessage map, int msgStatus) throws JMSException {
		map.setByte("MdMsgStatus", (byte) msgStatus);
	}
	
	public static void setMessageStatus(MapMessage map, MdsOmJmsMsgStatus msgStatus) throws JMSException {
		int mms;
		switch (msgStatus) {
		case OK:
			mms = MamaMsgStatus.STATUS_OK;
			break;
		case STALE:
			mms = MamaMsgStatus.STATUS_STALE;
			break;
		case INVALID:
			mms = MamaMsgStatus.STATUS_NOT_FOUND;
			break;
		case DENIED:
			mms = MamaMsgStatus.STATUS_NOT_ENTITLED;
			break;
		default:
			mms = MamaMsgStatus.STATUS_OK;
			break;
		}
		map.setByte("MdMsgStatus", (byte) mms);
	}
}
