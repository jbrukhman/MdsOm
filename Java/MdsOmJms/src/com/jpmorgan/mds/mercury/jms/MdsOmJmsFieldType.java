package com.jpmorgan.mds.mercury.jms;

import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.MessageFormatException;
import javax.jms.Topic;

import com.jpmorgan.mds.mercury.helper.MdsOmEnv;
import com.wombat.mama.MamaFieldDescriptor;

public enum MdsOmJmsFieldType {
	FIELD_UNKNOWN,		/** unknown field */
	FIELD_BOOL,			/** Java boolean */
	FIELD_BYTE,			/** Java byte */
	FIELD_CHAR,			/** Java char */
	FIELD_SHORT,		/** Java short */
	FIELD_INT,			/** Java int */
	FIELD_LONG,			/** Java long */
	FIELD_FLOAT,		/** Java float */
	FIELD_DOUBLE,		/** Java double */
	FIELD_STRING,		/** Java String */
	FIELD_CALENDAR,		/** Java Calendar - datetime */
	FIELD_PRICE,		/** Java double - price */
	FIELD_BYTE_ARRAY,	/** Java byte[] */
	FIELD_MSG_LIST,		/** Java List<MapMessage> */
	FIELD_STRING_LIST;	/** Java List<String> */
		
	public static MdsOmJmsFieldType getFieldType(MapMessage map, String name) throws JMSException {
		MapMessageImpl impl = (MapMessageImpl) map;
		MdsOmEnv env = impl.getEnv();
		MamaFieldDescriptor fd = env.getFieldDescriptor(name);
		if (fd != null) {
			switch (fd.getType()) {
			case MamaFieldDescriptor.BOOL:
				if (impl.getConfig().isBasicMode()) return MdsOmJmsFieldType.FIELD_LONG;
				return MdsOmJmsFieldType.FIELD_BOOL;
			case MamaFieldDescriptor.CHAR:
				return MdsOmJmsFieldType.FIELD_CHAR;
			case MamaFieldDescriptor.I8:
				if (impl.getConfig().isBasicMode()) return MdsOmJmsFieldType.FIELD_LONG;
				return MdsOmJmsFieldType.FIELD_BYTE;
			case MamaFieldDescriptor.U8:
			case MamaFieldDescriptor.I16:
				if (impl.getConfig().isBasicMode()) return MdsOmJmsFieldType.FIELD_LONG;
				return MdsOmJmsFieldType.FIELD_SHORT;
			case MamaFieldDescriptor.U16:
			case MamaFieldDescriptor.I32:
				if (impl.getConfig().isBasicMode()) return MdsOmJmsFieldType.FIELD_LONG;
				return MdsOmJmsFieldType.FIELD_INT;
			case MamaFieldDescriptor.U32:
			case MamaFieldDescriptor.I64:
			case MamaFieldDescriptor.U64:
				return MdsOmJmsFieldType.FIELD_LONG;
			case MamaFieldDescriptor.F32:
				if (impl.getConfig().isBasicMode()) return MdsOmJmsFieldType.FIELD_DOUBLE;
				return MdsOmJmsFieldType.FIELD_FLOAT;
			case MamaFieldDescriptor.F64:
				return MdsOmJmsFieldType.FIELD_DOUBLE;
			case MamaFieldDescriptor.STRING:
				return MdsOmJmsFieldType.FIELD_STRING;
			case MamaFieldDescriptor.TIME:
				if (impl.getConfig().isBasicMode()) return MdsOmJmsFieldType.FIELD_STRING;
				return MdsOmJmsFieldType.FIELD_CALENDAR;
			case MamaFieldDescriptor.PRICE:
				if (impl.getConfig().isBasicMode()) return MdsOmJmsFieldType.FIELD_DOUBLE;
				return MdsOmJmsFieldType.FIELD_PRICE;
			case MamaFieldDescriptor.OPAQUE:
				return MdsOmJmsFieldType.FIELD_BYTE_ARRAY;
			case MamaFieldDescriptor.VECTOR_MSG:
				return MdsOmJmsFieldType.FIELD_MSG_LIST;
			case MamaFieldDescriptor.VECTOR_STRING:
				return MdsOmJmsFieldType.FIELD_STRING_LIST;
			default:
				return MdsOmJmsFieldType.FIELD_UNKNOWN;
			}
		}
		String symbol = "unknown";
		Topic topic = ((MapMessageImpl) map).getTopic();
		if (topic != null) {
			symbol = topic.getTopicName();
		}
		throw new MessageFormatException("Field not found for env=" + env.getName() + " topic=" + symbol + " field=" + name);
	}
}
