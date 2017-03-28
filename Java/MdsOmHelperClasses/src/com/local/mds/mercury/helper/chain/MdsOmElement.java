
package com.jpmorgan.mds.mercury.helper.chain;

class MdsOmElement {
	String name = "";
	boolean dirty = false;

	MdsOmElement(String name)
	{
		this.name = name;
	}
	
	String getName() { return name; }
	
	void setName(String name) { this.name = name; }
	
	boolean isDirty() { return dirty; }
	
	void setDirty(boolean dirty) { this.dirty = dirty; }

}
