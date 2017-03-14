package com.jpmorgan.mds.mercury.helper;

import com.jpmorgan.mds.mercury.helper.MdsOm.MdsOmEnvType;
import com.wombat.mama.Mama;
import com.wombat.mama.MamaException;
import com.wombat.mama.MamaInbox;
import com.wombat.mama.MamaInboxCallback;
import com.wombat.mama.MamaLogLevel;
import com.wombat.mama.MamaMsg;
import com.wombat.mama.MamaMsgStatus;
import com.wombat.mama.MamaMsgType;
import com.wombat.mama.MamaPublisher;
import com.wombat.mama.MamaStatus;
import com.wombat.mama.MamaTransport;
import com.wombat.mama.MamaDictionary;

public class MdsOmPublisher extends MamaPublisher implements MamaInboxCallback {
	private String symbol = "";
	private String source = "";
	private MamaTransport transport = null;
	private MdsOmEnv env = null;
	private MamaMsg msg = null;
	
	// Temp for Tick42
	private MamaInbox inbox = null;	
	
	private MdsOmPublisherCallback cb = null;
	private Object closure = null;
	
	private static MdsOmPublisherFeedback feedback = new MdsOmPublisherFeedback();
	
	public MdsOmPublisher()
	{	
	}
	
	/**
	 * Create a publisher.
	 * @param              env the MdsOmEnv to create the publisher for.
	 * @param              transport the MamaTransport for the publisher.
	 * @param              source the source (e.g., "IDN_DEV").
	 * @param              symbol the symbol (e.g., "AAPL.N").
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public void create(MdsOmEnv env, MamaTransport transport, String source, String symbol, MdsOmPublisherCallback cb, Object closure) {
		try {
			// This is called by getPublisher in MdsOm or MdsOmEnv
			this.env = env;
			this.transport = transport;
			this.source = source;
			this.symbol = symbol;
			this.cb = cb;
			this.closure = closure;

			if (env.getType() == MdsOmEnvType.TREP) {
				inbox = new MamaInbox();
				inbox.create(transport, env.getQueue(), this);
			} else if (env.getType() == MdsOmEnvType.MERCURY) {
				// Solace - get info from bridge subscription
				feedback.addPublisher(this);
			}
	
			super.create(transport, symbol, source);
		} catch (MamaException e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmPublisher.create: error " + env.getName() + " " + source + "." + symbol + " " + MdsOmUtil.getStackTrace(e));
			throw e;
		}
	}

	/**
	 * Publish a message.
	 * This is asynchronous and feed back comes in MamaPublisherCallback
	 * @param              msg the MamaMsg to publish.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public void send(MamaMsg msg)
	{
		try {
			if (env.enableStats) {
				if (env.countTransports > 1) env.statsLock.lock();
				env.countPubMsgs++;
				env.countPubMsgsPeriod++;
				if (env.countTransports > 1) env.statsLock.unlock();
			}
	
			if (inbox != null) {
				super.sendFromInbox(inbox, msg);
			} else {
				super.send(msg);
			}
		} catch (MamaException e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmPublisher.send: error " + env.getName() + " " + source + "." + symbol + " " + MdsOmUtil.getStackTrace(e));
			throw e;
		}	
	}

	/**
	 * Destroy this publisher.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public void destroy()
	{
		try {
			if (env.getType() == MdsOmEnvType.MERCURY) {
				// Solace - get info from bridge subscription
				feedback.removePublisher(this);
			}
			
			env.countPubs--;

			super.destroy();
			
			if (inbox != null) {
				inbox.destroy();
				inbox = null;
			}			
		} catch(Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmPublisher.destroy: error " + env.getName() +" "+ source + "." + symbol + " " + MdsOmUtil.getStackTrace(e));
		}
 
	}

    protected void finalize() throws Throwable
    {
        try {
        	destroy();
        } finally {
            // Call the base class finalizer
            super.finalize();
        }
    }
    
    /**
	 * Get a MamaMsg to publish.
	 * This method handles setting the correct payload id for this publisher.
	 * The msg is owned by the caller, but can be reused for the life of the application.
	 * @return             MamaMsg*.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public MamaMsg getMamaMsg() throws Exception
	{
		return MdsOmUtil.newMamaMsg(env.getPayloadId());
	}

	/**
	 * Destroy a MamaMsg.
	 * @param              msg the MamaMsg to destroy.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public void destroyMamaMsg(MamaMsg msg) throws Exception
	{
		MdsOmUtil.deleteMamaMsg(msg);
	}

	/**
	 * Get the underlying MamaTransport.
	 * @return             MamaTransport.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public MamaTransport getMamaTransport() { return transport; }

	/**
	 * Get the MdsOmEnv for this publisher.
	 * @return             MdsOmEnv.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public MdsOmEnv getEnv() { return env; }

	/**
	 * Get the MamaDictionary.
	 * @return             MamaDictionary.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public MamaDictionary getDictionary() { return env == null ? null : env.getDictionary(); }

	/**
	 * Get the symbol.
	 * @return             symbol.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public String getSymbol() { return symbol; }

	/**
	 * Get the source.
	 * @return             source.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public String getSource() { return source; }

	public String getSubject() { return source + "." + symbol; }

	/**
	 * Store a MamaMsg in the publisher. Useful for reusing MamaMsgs.
	 * @param              msg the MamaMsg to store.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public void storeMamaMsg(MamaMsg msg) { this.msg = msg; }

	/**
	 * Retrieve a stored MamaMsg in the publisher. Useful for reusing MamaMsgs.
	 * @return             MamaMsg.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public MamaMsg retrieveMamaMsg() { return msg; }


	public MdsOmPublisherCallback getCb() { return cb; }

	public Object getClosure() { return closure; }
	
	// INBOX
	// This is used for Tick42, which returns publish status via inbox.
	// Solace does it differently.
	// OM group is working on a callback mechanism for publish.
	@Override
	public void onDestroy(MamaInbox arg0)
	{
	}

	@Override
	public void onMsg(MamaInbox inbox, MamaMsg msg)
	{
		try {
			short msgType = MamaMsgType.typeForMsg(msg);
			int msgStatus = MamaMsgStatus.statusForMsg(msg);
			if (msgType == MamaMsgType.TYPE_SEC_STATUS) {
				short mamaStatus = MdsOmUtil.convertMsgStatusToMamaStatus((short) msgStatus);
				if (mamaStatus != MamaStatus.MAMA_STATUS_OK) {
					Mama.log(MamaLogLevel.ERROR, "MdsOmPublisher::onMsg: " + env.getName() + " " + source + "." + symbol + " " + " status=" + mamaStatus);
					if (cb != null) {
						cb.onPublishError(this, mamaStatus, closure);
					}
				}
			}
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmPublisher.onMsg: error " + env.getName() + " " + source + "." + symbol + " " + MdsOmUtil.getStackTrace(e));
		}
	}
} 
