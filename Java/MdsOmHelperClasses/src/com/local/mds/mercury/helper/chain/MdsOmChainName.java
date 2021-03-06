//******************************************************************************
//
// chainname.cxx
// Implementation of chain name class
//
//******************************************************************************

package com.jpmorgan.mds.mercury.helper.chain;

import com.jpmorgan.mds.mercury.helper.MdsOmStatus;
import com.jpmorgan.mds.mercury.helper.MdsOmUtil;
import com.wombat.mama.Mama;
import com.wombat.mama.MamaLogLevel;

class MdsOmChainName {
	
	ChainConfig chainConfig = null;
	
	String source = "";
	String symbol = "";
	String subject = "";

	public MdsOmChainName()
	{
	}
	
	//******************************************************************************
	// Initialise chain name object
	//******************************************************************************
	public short init(String source, String symbol, ChainConfig chainConfig)
	{
		if (MdsOmUtil.isBlank(source) || MdsOmUtil.isBlank(symbol) || chainConfig == null) {
			Mama.log(MamaLogLevel.ERROR, String.format("MdsOmmChainName.init: null arg source=%s symbol=%s config=%s", source, symbol, chainConfig));
			return MdsOmStatus.NULL_ARG;
		}
	
		// Store the pointer to chain config object for later reference
		this.chainConfig = chainConfig;
		
		this.source = source;
		this.symbol = symbol;
		this.subject = source + "." + symbol;
	
		return MdsOmStatus.OK;
	}
	
	//******************************************************************************
	// GetNextLinkName
	// Given the name of a link get the next link name.
	// This is used when a publisher uses AddElement() and there are more
	//	 then 14 elements, and we need to create a new link, and give it a name.
	//******************************************************************************
	String getNextLinkSubject()
	{
		//----------------------------------------------------------------------------
		// This chain is going to be published, and publish chains have a symbol
		// naming convention as such:
		//	 n#chain_name
		// where 'n' is index of chain(ie 0,1,2...)
		//----------------------------------------------------------------------------
		int index = symbol.indexOf('#');
		if (index == -1) {
			return null;
		}
	
		// Work out index of current link, ie up to # is numeric
		String linkIndexStr = symbol.substring(0, index);
		int linkIndex = 0;
		try {
			linkIndex = Integer.valueOf(linkIndexStr).intValue();
		} catch (NumberFormatException e) {
			return null;
		}
	
		// Create symbol of next link(w/o the source)
		linkIndex++;
		String linkBaseStr = symbol.substring(index);
		String nextLinkSubject = "" + linkIndex + linkBaseStr;
	
		Mama.log(MamaLogLevel.FINE, String.format("GetNextLinkSubject: sym=%s new=%s", subject, nextLinkSubject));
		
		return nextLinkSubject;
	}
}
