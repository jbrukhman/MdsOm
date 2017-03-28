package com.jpmorgan.mds.mercury.helper.chain;

import com.jpmorgan.mds.mercury.helper.*;

public class MdsOmChainFactory {
	public static MdsOmSubChain factorySub(MdsOm om, String subject) throws MdsOmStatus
	{
		return new MdsOmChain(om, subject, "");
	}
	
	public static MdsOmSubChain factorySub(MdsOm om, String source, String symbol) throws MdsOmStatus
	{
		return new MdsOmChain(om, source, symbol, "");
	}
	
	public static MdsOmPubChain factoryPub(MdsOm om, String subject, String templateName) throws MdsOmStatus
	{
		return new MdsOmChain(om, subject, templateName);
	}
	
	public static MdsOmPubChain factoryPub(MdsOm om, String source, String symbol, String templateName) throws MdsOmStatus
	{
		return new MdsOmChain(om, source, symbol, templateName);
	}
}
