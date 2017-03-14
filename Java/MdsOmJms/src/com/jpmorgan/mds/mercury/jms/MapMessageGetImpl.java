
//
// Code generated by script gen_map_get.
// Do not change here, change in script.
//
package com.jpmorgan.mds.mercury.jms;

import javax.jms.JMSException;
import javax.jms.MessageFormatException;

import com.wombat.mama.MamaException;
import com.wombat.mama.MamaFieldDescriptor;
import com.wombat.mama.MamaFieldTypeException;
import com.wombat.mama.MamaPrice;

public class MapMessageGetImpl {
	private MapMessageImpl impl = null;
		
	protected MapMessageGetImpl(MapMessageImpl impl) {
		this.impl = impl;
	} 

	/** byte
	 * Code generated by script gen_map_get.
	 * Do not change here, change in script.
	 */
	public byte getByte(String name) throws JMSException {
		MamaFieldDescriptor fd = null;
		try {
			fd = impl.checkFieldName(name);
			switch (fd.getType()) {
			case MamaFieldDescriptor.I8:
				return impl.getMsg().getI8(fd);
			case MamaFieldDescriptor.STRING:
				String sval = impl.getMsg().getString(fd);
				if (sval == null) throw new MessageFormatException("getByte: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
				else return Byte.valueOf(sval);
			default:
				throw new MessageFormatException("getByte: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
			}
		} catch (MamaFieldTypeException e1) {
			throw new MessageFormatException("getByte: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
		} catch (MamaException e2) {
			throw new JMSException("getByte: error for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd) + " impl.getMsg()=" + e2.getMessage());
		}
	}

	/** char
	 * Code generated by script gen_map_get.
	 * Do not change here, change in script.
	 */
	public char getChar(String name) throws JMSException {
		MamaFieldDescriptor fd = null;
		try {
			fd = impl.checkFieldName(name);
			switch (fd.getType()) {
			case MamaFieldDescriptor.CHAR:
				return impl.getMsg().getChar(fd);
			default:
				throw new MessageFormatException("getChar: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
			}
		} catch (MamaFieldTypeException e1) {
			throw new MessageFormatException("getChar: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
		} catch (MamaException e2) {
			throw new JMSException("getChar: error for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd) + " impl.getMsg()=" + e2.getMessage());
		}
	}

	/** short
	 * Code generated by script gen_map_get.
	 * Do not change here, change in script.
	 */
	public short getShort(String name) throws JMSException {
		MamaFieldDescriptor fd = null;
		try {
			fd = impl.checkFieldName(name);
			switch (fd.getType()) {
			case MamaFieldDescriptor.I8:
				return impl.getMsg().getI8(fd);
			case MamaFieldDescriptor.U8:
				return impl.getMsg().getU8(fd);
			case MamaFieldDescriptor.I16:
				return impl.getMsg().getI16(fd);
			case MamaFieldDescriptor.STRING:
				String sval = impl.getMsg().getString(fd);
				if (sval == null) throw new MessageFormatException("getShort: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
				else return Short.valueOf(sval);
			default:
				throw new MessageFormatException("getShort: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
			}
		} catch (MamaFieldTypeException e1) {
			throw new MessageFormatException("getShort: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
		} catch (MamaException e2) {
			throw new JMSException("getShort: error for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd) + " impl.getMsg()=" + e2.getMessage());
		}
	}

	/** int
	 * Code generated by script gen_map_get.
	 * Do not change here, change in script.
	 */
	public int getInt(String name) throws JMSException {
		MamaFieldDescriptor fd = null;
		try {
			fd = impl.checkFieldName(name);
			switch (fd.getType()) {
			case MamaFieldDescriptor.I8:
				return impl.getMsg().getI8(fd);
			case MamaFieldDescriptor.U8:
				return impl.getMsg().getU8(fd);
			case MamaFieldDescriptor.I16:
				return impl.getMsg().getI16(fd);
			case MamaFieldDescriptor.U16:
				return impl.getMsg().getU16(fd);
			case MamaFieldDescriptor.I32:
				return impl.getMsg().getI32(fd);
			case MamaFieldDescriptor.STRING:
				String sval = impl.getMsg().getString(fd);
				if (sval == null) throw new MessageFormatException("getInt: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
				else return Integer.valueOf(sval);
			default:
				throw new MessageFormatException("getInt: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
			}
		} catch (MamaFieldTypeException e1) {
			throw new MessageFormatException("getInt: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
		} catch (MamaException e2) {
			throw new JMSException("getInt: error for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd) + " impl.getMsg()=" + e2.getMessage());
		}
	}

	/** long
	 * Code generated by script gen_map_get.
	 * Do not change here, change in script.
	 */
	public long getLong(String name) throws JMSException {
		MamaFieldDescriptor fd = null;
		try {
			fd = impl.checkFieldName(name);
			switch (fd.getType()) {
			case MamaFieldDescriptor.I8:
				return impl.getMsg().getI8(fd);
			case MamaFieldDescriptor.U8:
				return impl.getMsg().getU8(fd);
			case MamaFieldDescriptor.I16:
				return impl.getMsg().getI16(fd);
			case MamaFieldDescriptor.U16:
				return impl.getMsg().getU16(fd);
			case MamaFieldDescriptor.I32:
				return impl.getMsg().getI32(fd);
			case MamaFieldDescriptor.U32:
				return impl.getMsg().getU32(fd);
			case MamaFieldDescriptor.I64:
				return impl.getMsg().getI64(fd);
			case MamaFieldDescriptor.U64:
				return impl.getMsg().getU64(fd);
			case MamaFieldDescriptor.STRING:
				String sval = impl.getMsg().getString(fd);
				if (sval == null) throw new MessageFormatException("getLong: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
				else return Long.valueOf(sval);
			default:
				throw new MessageFormatException("getLong: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
			}
		} catch (MamaFieldTypeException e1) {
			throw new MessageFormatException("getLong: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
		} catch (MamaException e2) {
			throw new JMSException("getLong: error for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd) + " impl.getMsg()=" + e2.getMessage());
		}
	}

	/** float
	 * Code generated by script gen_map_get.
	 * Do not change here, change in script.
	 */
	public float getFloat(String name) throws JMSException {
		MamaFieldDescriptor fd = null;
		try {
			fd = impl.checkFieldName(name);
			switch (fd.getType()) {
			case MamaFieldDescriptor.F32:
				return impl.getMsg().getF32(fd);
			case MamaFieldDescriptor.STRING:
				String sval = impl.getMsg().getString(fd);
				if (sval == null) throw new MessageFormatException("getFloat: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
				else return Float.valueOf(sval);
			default:
				throw new MessageFormatException("getFloat: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
			}
		} catch (MamaFieldTypeException e1) {
			throw new MessageFormatException("getFloat: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
		} catch (MamaException e2) {
			throw new JMSException("getFloat: error for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd) + " impl.getMsg()=" + e2.getMessage());
		}
	}

	/** double
	 * Code generated by script gen_map_get.
	 * Do not change here, change in script.
	 */
	public double getDouble(String name) throws JMSException {
		MamaFieldDescriptor fd = null;
		try {
			fd = impl.checkFieldName(name);
			switch (fd.getType()) {
			case MamaFieldDescriptor.F32:
				return impl.getMsg().getF32(fd);
			case MamaFieldDescriptor.F64:
				return impl.getMsg().getF64(fd);
			case MamaFieldDescriptor.PRICE:
				MamaPrice pval = impl.getMsg().getPrice(fd);
				if (pval == null) throw new MessageFormatException("getDouble: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
				else return pval.getValue();
			case MamaFieldDescriptor.STRING:
				String sval = impl.getMsg().getString(fd);
				if (sval == null) throw new MessageFormatException("getDouble: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
				else return Double.valueOf(sval);
			default:
				throw new MessageFormatException("getDouble: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
			}
		} catch (MamaFieldTypeException e1) {
			throw new MessageFormatException("getDouble: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
		} catch (MamaException e2) {
			throw new JMSException("getDouble: error for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd) + " impl.getMsg()=" + e2.getMessage());
		}
	}

	/** boolean
	 * Code generated by script gen_map_get.
	 * Do not change here, change in script.
	 */
	public boolean getBoolean(String name) throws JMSException {
		MamaFieldDescriptor fd = null;
		try {
			fd = impl.checkFieldName(name);
			switch (fd.getType()) {
			case MamaFieldDescriptor.BOOL:
				return impl.getMsg().getBoolean(fd);
			case MamaFieldDescriptor.STRING:
				String sval = impl.getMsg().getString(fd);
				if (sval == null) throw new MessageFormatException("getBoolean: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
				else return Boolean.valueOf(sval);
			default:
				throw new MessageFormatException("getBoolean: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
			}
		} catch (MamaFieldTypeException e1) {
			throw new MessageFormatException("getBoolean: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
		} catch (MamaException e2) {
			throw new JMSException("getBoolean: error for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd) + " impl.getMsg()=" + e2.getMessage());
		}
	}

	/** byte[]
	 * Code generated by script gen_map_get.
	 * Do not change here, change in script.
	 */
	public byte[] getBytes(String name) throws JMSException {
		MamaFieldDescriptor fd = null;
		try {
			fd = impl.checkFieldName(name);
			switch (fd.getType()) {
			case MamaFieldDescriptor.OPAQUE:
				return impl.getMsg().getOpaque(fd);
			default:
				throw new MessageFormatException("getBytes: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
			}
		} catch (MamaFieldTypeException e1) {
			throw new MessageFormatException("getBytes: not valid for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd));
		} catch (MamaException e2) {
			throw new JMSException("getBytes: error for topic=" + impl.getSymbol() + " field=" + name + " type=" + impl.getJmsTypeFromMamaType(fd) + " impl.getMsg()=" + e2.getMessage());
		}
	}
}

