package com.jpmorgan.mds.mercury.jms;

import javax.jms.JMSException;
import javax.jms.MapMessage;

import com.wombat.mama.MamaMsgType;

public enum MdsOmJmsMsgType {
	INITIAL, UPDATE, STATUS, CORRECTION, RESET, STREAM_UPDATE, UNKNOWN, BOOK_INITIAL, BOOK_UPDATE;
	
	/**
	 * Get MamaMsgType from the enum.
	 * @return
	 */
	protected byte getCode() {
		switch (this) {
		case INITIAL:
			return MamaMsgType.TYPE_INITIAL;
		case BOOK_INITIAL:
			return MamaMsgType.TYPE_BOOK_INITIAL;
		case UPDATE:
			return MamaMsgType.TYPE_UPDATE;
		case BOOK_UPDATE:
			return MamaMsgType.TYPE_BOOK_UPDATE;
		case STATUS:
			return MamaMsgType.TYPE_SEC_STATUS;
		default:
			// TODO add the others
			return MamaMsgType.TYPE_UPDATE;
		}
	}
	
	/**
	 * Get the msgType from a message.
	 * @param map
	 * @return
	 */
	public static MdsOmJmsMsgType getMessageType(MapMessage map) {
		try {
			int ft;
			ft = map.getInt("MdMsgType");
			return get(ft);
		} catch (JMSException e) {
			// e.printStackTrace();
		} catch (Exception ex) {
			// TODO sometimes in L2 data there is no msg type
		}
		return UNKNOWN;
	}
	
	private static MdsOmJmsMsgType get(int t) {
		switch (t) {
		case MamaMsgType.TYPE_UPDATE:
			return UPDATE;
		case MamaMsgType.TYPE_INITIAL:
			return INITIAL;
		case MamaMsgType.TYPE_BOOK_UPDATE:
			return BOOK_UPDATE;
		case MamaMsgType.TYPE_BOOK_INITIAL:
			return BOOK_INITIAL;
		case MamaMsgType.TYPE_SEC_STATUS:
			return STATUS;
		default:
			return UNKNOWN;
		}
	}
	
	public static void setMessageType(MapMessage map, MdsOmJmsMsgType msgType) throws JMSException {
		int t;
		switch (msgType) {
		case UPDATE:
			t = MamaMsgType.TYPE_UPDATE;
			break;
		case INITIAL:
			t = MamaMsgType.TYPE_INITIAL;
			break;
		case BOOK_UPDATE:
			t = MamaMsgType.TYPE_BOOK_UPDATE;
			break;
		case BOOK_INITIAL:
			t = MamaMsgType.TYPE_BOOK_INITIAL;
			break;
		case STATUS:
			t = MamaMsgType.TYPE_SEC_STATUS;
			break;
		default:
			// TODO maybe throw exception instead
			t = MamaMsgType.TYPE_INITIAL;
			break;
		}
		map.setByte("MdMsgType", (byte) t);
	}
}
