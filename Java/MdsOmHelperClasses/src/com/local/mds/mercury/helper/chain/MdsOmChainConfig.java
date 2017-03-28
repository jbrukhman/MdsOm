
package com.jpmorgan.mds.mercury.helper.chain;

import java.util.ArrayList;
import java.util.List;

import com.jpmorgan.mds.mercury.helper.MdsOmUtil;
import com.wombat.mama.Mama;
import com.wombat.mama.MamaLogLevel;

//******************************************************************************
// ChainConfig class definition
//******************************************************************************
class ChainConfig {
	
	private String templateString = "mama.chains.template.";
	private String templateName = "";
	
	private String linkNames = "LINK_1,LINK_2,LINK_3,LINK_4,LINK_5,LINK_6,LINK_7,LINK_8,LINK_9,LINK_10,LINK_11,LINK_12,LINK_13,LINK_14";
	private String nextFieldName = "NEXT_LR";
	private String prevFieldName = "PREV_LR";
	private String refCount = "REF_COUNT";
	private String prefLink = "PREF_LINK";

	private List<String> linkArray = new ArrayList<String>();

	public String getTemplateName() { return templateName; }
	public String getLinkNames() { return linkNames; }
	public List<String> getLinkArray() { return linkArray; }

	public String getNextLinkName() { return nextFieldName; }
	public String getPrevLinkName() { return prevFieldName; }
	public String getRefCount() { return refCount; }
	public String getPrefLink() { return prefLink; }

	public int getMaxNumLinks() { return linkArray.size(); }
	
	public ChainConfig(String tName)
	{
		if (MdsOmUtil.isBlank(tName)) {
			// No template, look for default specified in config file
			String prop = templateString + "default";
			String p = Mama.getProperty(prop);
			if (MdsOmUtil.isNotBlank(p)) templateName = p;
		} else {
			// Use their template name
			templateName = tName;
		}
	}
	
	public void Init()
	{
		// SETUP DEFAULTS, these are used if there is no template available
	
		Mama.log(MamaLogLevel.NORMAL, String.format("ChainConfig: template=%s", templateName));
	
		if (MdsOmUtil.isNotBlank(templateName)) {
			String prop = templateString + templateName + ".LinkNames";
	
			String str = Mama.getProperty(prop);
			if (MdsOmUtil.isNotBlank(str)) {
				linkNames = str;
			}
			Mama.log(MamaLogLevel.FINE, String.format("ChainConfig template[%s] getting LinkNames=%s(%s)", templateName, linkNames, prop));
		
			prop = templateString + templateName + ".NextLinkName";
			str = Mama.getProperty(prop);
			if (MdsOmUtil.isNotBlank(str)) {
				nextFieldName = str;
			}
			Mama.log(MamaLogLevel.FINE, String.format("ChainConfig template[%s] getting NextLinkName=%s", templateName, nextFieldName));
	
			prop = templateString + templateName + ".PrevLinkName";
			str = Mama.getProperty(prop);
			if (MdsOmUtil.isNotBlank(str)) {
				prevFieldName = str;
			}
			Mama.log(MamaLogLevel.FINE, String.format("ChainConfig template[%s] getting PrevLinkName=%s", templateName, prevFieldName));
	
			prop = templateString + templateName + ".RefCount";
			str = Mama.getProperty(prop);
			if (MdsOmUtil.isNotBlank(str)) {
				refCount = str;
			}
			Mama.log(MamaLogLevel.FINE, String.format("ChainConfig template[%s] getting RefCount=%s", templateName, refCount)); 
	
			prop = templateString + templateName + ".PrefLink";
			str = Mama.getProperty(prop);
			if (MdsOmUtil.isNotBlank(str)) {
				prefLink = str;
			}
			Mama.log(MamaLogLevel.FINE, String.format("ChainConfig template[%s] getting PrefLink=%s", templateName, prefLink));
		}
	
		String[] tokens = linkNames.split(",");
		for (String s : tokens) {
			linkArray.add(s.trim());
		}
	}
}
