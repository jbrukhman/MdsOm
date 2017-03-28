package com.jpmorgan.mds.mercury.helper;

import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import com.wombat.mama.Mama;
import com.wombat.mama.MamaLogLevel;
import com.wombat.mama.MamaMsg;
import com.wombat.mama.MamaMsgStatus;
import com.wombat.mama.MamaMsgType;
import com.wombat.mama.MamaQueue;
import com.wombat.mama.MamaServiceLevel;
import com.wombat.mama.MamaSource;
import com.wombat.mama.MamaSubscription;
import com.wombat.mama.MamaDictionary;
import com.wombat.mama.MamaSubscriptionCallback;
import com.wombat.mama.MamaSubscriptionType;
import com.wombat.mama.MamaTransport;

public class MdsOmSubscription extends MamaSubscription implements MamaSubscriptionCallback {
	private MdsOmEnv env = null;
	private MamaSubscriptionCallback cb = null;
	private boolean gotResponse = false;
	private boolean waitingForDestroy = false;
	private long msgs = 0;
	private Semaphore sem = new Semaphore(0);
	private boolean useSem = false;

	public MdsOmSubscription()
	{
	}
	
	public long getMsgs() { return msgs; }
	
	public MdsOmEnv getEnv() { return env; }
	public void setEnv(MdsOmEnv env) { this.env = env; }

	public MamaDictionary getDictionary() { return env == null ? null : env.getDictionary(); }
		
	public boolean isGotResponse() { return gotResponse; }
	public void setGotResponse(boolean gotResponse) { this.gotResponse = gotResponse; }

	public void setCb(MamaSubscriptionCallback cb) { this.cb = cb; }
	public MamaSubscriptionCallback getCb() { return cb; }
	
	public String getTopic() { return getSubscSource() + "." + getSymbol(); }
	
	@Override
	public void destroy()
	{
		useSem = false;
		internalDestroy(0);
	}

	public boolean destroy(int interval)
	{
		useSem = true;
		return internalDestroy(interval);
	}
	
	private boolean internalDestroy(int interval)
	{
		Mama.log(MamaLogLevel.FINE, "MdsOmSubscription::destroy: " + env.getName() + " " + getTopic() + " " + interval);

		boolean ret = true;
		
		try {
			waitingForDestroy = true;
			super.destroy();
			if (useSem) {
				if (interval > 0) {
					ret = sem.tryAcquire(interval, TimeUnit.MILLISECONDS);
					if (!ret) {
						Mama.log(MamaLogLevel.FINE, "MdsOmSubscription::destroy: " + " " + getTopic() + " sem returned false");
					}
				} else {
					sem.acquire();
				}
			}
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmSubscription.destroy: error " + env.getName() + " " + getTopic() + " " + MdsOmUtil.getStackTrace(e));
		}
		return ret;
	}

	public void create(MdsOmEnv env, MamaTransport transport, String source, String symbol, MamaSubscriptionCallback cb, Object closure, boolean setup)
	{
		createInternal(env, transport, source, symbol, cb, closure, setup, MamaServiceLevel.REAL_TIME);
	}
	
	public void snap(MdsOmEnv env, MamaTransport transport, String source, String symbol, MamaSubscriptionCallback cb, Object closure, boolean setup)
	{
		createInternal(env, transport, source, symbol, cb, closure, setup, MamaServiceLevel.SNAPSHOT);
	}
	
	private void createInternal(MdsOmEnv env, MamaTransport transport, String source, String symbol, MamaSubscriptionCallback cb, Object closure, boolean setup, short subType)
	{
		Mama.log(MamaLogLevel.FINE, "MdsOmSubscription::create: " + env.getName() + " source=" + source + " symbol=" + symbol + " setup=" + setup);
		try {
			this.env = env;
			this.cb = cb;
			
			MamaQueue queue = env.getQueue();

			if (subType == MamaServiceLevel.REAL_TIME) {
				setRequiresInitial(env.isRequireInitial());
			} else {
				setRequiresInitial(true);
			}
			setSubscriptionType(MamaSubscriptionType.NORMAL);
			setServiceLevel(subType, 0);
			
			MamaSource src = new MamaSource(source, transport, source);
			
			if (setup) {
				super.setupSubscription(this, queue, src, symbol, closure);
			} else {
				super.createSubscription(this, queue, src, symbol, closure);
			}
			
			setRecoverGaps(env.isEnableDq());

		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmSubscription.create: error " + env.getName() + " " + " source=" + source + " symbol=" + symbol + " " + MdsOmUtil.getStackTrace(e));
		}
	}

	@Override
	public void onMsg(MamaSubscription sub, MamaMsg msg)
	{
		if (waitingForDestroy) return;
		msgs++;
		
		String symbol = "??";
		String source = "??";
		try {
			symbol = getSymbol();
			source = getSubscSource();
			
			long start = 0;
			if (env.enableStats) {
				start = MdsOmUtil.getNow();
				long c = env.getQueue().getEventCount();
				
				if (env.countTransports > 1) env.statsLock.lock();
				
				if (c > env.maxQueueCount) env.maxQueueCount = c;

				// Count how many subs have received a response
				if (!gotResponse) {
					gotResponse = true;
					env.countResponses++;
				}
				
				env.countSubMsgs++;
				env.countSubMsgsPeriod++;

				short type = 0;
				int status = 0;
				try {
					type = MamaMsgType.typeForMsg(msg);
					status = MamaMsgStatus.statusForMsg(msg);
					env.countTypes(type, status);
				} catch (Exception e){
					Mama.log(MamaLogLevel.ERROR, "MdsOmSubscription.onMsg: "+ env.getName() + " "+ symbol + " error getting type="+ MamaMsgType.stringForType(type)+ " and status="+MamaMsgStatus.stringForStatus(status));
				}
				
				if (env.countTransports > 1) env.statsLock.unlock();			
			}
			
			if (cb != null) {
				cb.onMsg(this, msg);
			}

			if (env.enableStats) {
				if (env.countTransports > 1) env.statsLock.lock();
				
				env.inOnMsgSum += MdsOmUtil.getNow() - start;
				env.inOnMsgCount++;
				
				if (env.countTransports > 1) env.statsLock.unlock();
			}

		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmSubscription.onMsg: error " + env.getName() + " " + source+"." + symbol + " " + MdsOmUtil.getStackTrace(e));
		}
	}

	@Override
	public void onCreate(MamaSubscription sub) {
		Mama.log(MamaLogLevel.FINE, "MdsOmSubscription::onCreate: " + env.getName() + " " + getTopic());

		try {
			cb.onCreate(this);
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmSubscription.onCreate: error " + env.getName() + " " + getTopic() + " " + MdsOmUtil.getStackTrace(e));
		}
	}

	@Override
	public void onDestroy(MamaSubscription sub) {
		Mama.log(MamaLogLevel.FINE, "MdsOmSubscription::onDestroy: " + env.getName() + " " + getTopic());

		env.countSubs--;
		
		cb.onDestroy(this);
		
		if (useSem) {
			sem.release();
		}
	}

	@Override
	public void onError(MamaSubscription sub, short wombatStatus, int platformError, String symbol, Exception ex) {
		Mama.log(MamaLogLevel.FINE, "MdsOmSubscription::onError: " + env.getName() + " " + getTopic() + " status=" + wombatStatus + " platform=" + platformError);
		try {
			if (!gotResponse) {
				gotResponse = true;
				env.countResponses++;
			}
			cb.onError(this, wombatStatus, platformError, symbol, ex);
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmSubscription.onError: error " + env.getName() + " " + getTopic() + " " + MdsOmUtil.getStackTrace(e));
		}
	}

	@Override
	public void onGap(MamaSubscription sub) {
		Mama.log(MamaLogLevel.FINE, "MdsOmSubscription::onGap: " + env.getName() + " " + getTopic());
		try {
			cb.onGap(this);
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmSubscription.onGap: error " + env.getName() + " " + getTopic() + " " + MdsOmUtil.getStackTrace(e));
		}
	}

	@Override
	public void onQuality(MamaSubscription sub, short quality, short cause, Object platformInfo) {
		Mama.log(MamaLogLevel.FINE, "MdsOmSubscription::onQuality: " + env.getName() + " " + getTopic() + " q=" + quality + " c=" + cause);

		try {
			cb.onQuality(this, quality, cause, platformInfo);
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmSubscription.onQuality: error " + env.getName() + " " + getTopic() + " " + MdsOmUtil.getStackTrace(e));
		}
	}

	@Override
	public void onRecapRequest(MamaSubscription sub) {
		try {
			cb.onRecapRequest(this);
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmSubscription.onRecapRequest: error " + env.getName() + " " + getTopic() + " " + MdsOmUtil.getStackTrace(e));
		}
	}

}
