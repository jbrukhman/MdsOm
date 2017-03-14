package com.jpmorgan.mds.mercury.helper;

import com.wombat.mama.Mama;
import com.wombat.mama.MamaLogLevel;
import com.wombat.mama.MamaMsg;
import com.wombat.mama.MamaMsgStatus;
import com.wombat.mama.MamaStatus;

import java.io.PrintWriter;
import java.io.StringWriter;

public class MdsOmUtil {
	private static long start = System.currentTimeMillis() / 1000;		// get process start time

	public static Pair<String, String> getSourceAndSymbol(String topic)
	{
		Pair<String, String> ret = new Pair<String, String>("", "");
		
		if (topic == null) return ret;
		
		int index = topic.indexOf(".");
		if (index != -1) {
			String source = topic.substring(0, index);
			ret.setX(source);
			
			// Can only be greater since source and symbol are required
			// - indexOf will return 1st instance of . 
			// - if indexOf(".")+1 == topic.length then there is no symbol resulting in an error later on
			if (topic.length() > index + 1) {
				String symbol = topic.substring(index + 1);
				ret.setY(symbol);
			}
		}
		return ret;
	}
	
	public static boolean isNotBlank(String s)
	{
		if (s == null || s.isEmpty()) return false;
		return true;
	}
	
	public static boolean isBlank(String s)
	{
		if (s == null || s.isEmpty()) return true;
		return false;
	}
	
	public static long getNow() { return System.nanoTime(); }
	
	/**
	 * Get a MamaMsg to publish.
	 * This method handles setting the correct payload id for this publisher.
	 * The msg is owned by the caller, but can be reused for the life of the application.
	 * @return             MamaMsg*.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public static MamaMsg newMamaMsg(char payload) throws Exception
	{
		try {
			MamaMsg msg = new MamaMsg(payload);
			return msg;
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "newMamaMsg: error " + MdsOmUtil.getStackTrace(e));
			throw e;
		}
	}

	/**
	 * Destroy a MamaMsg.
	 * @param              msg the MamaMsg to destroy.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public static void deleteMamaMsg(MamaMsg msg) throws Exception
	{
		try {
			msg.destroy();
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmPublisher.destroyMamaMsg: error " + MdsOmUtil.getStackTrace(e));
			throw e;
		}
	}

	/**
	 * Get the elapsed time in the program.
	 * Useful for logging in test programs.
	 * @return
	 */
	public static String getElapsed()
	{
		long now = System.currentTimeMillis() / 1000;

		long diff = now - start;

		int days = (int) (diff / 60 / 60 / 24);
		int hours = (int) ((diff / 60 / 60) % 24);
		int mins = (int) ((diff / 60) % 60);
		int secs = (int) ( diff % 60);

		String s = String.format("%02d:%02d:%02d:%02d", days, hours, mins, secs);

		return s;
	}
	
	/**
	 * This is used to get the exception descr and stack trace as a string for Mama logging.
	 * @param e
	 * @return
	 */
	public static String getStackTrace(Exception e)
	{
		StringWriter sw = new StringWriter();
		e.printStackTrace(new PrintWriter(sw));
		return sw.toString();
	}
	
	/**
	 * Convert a Mama msg status to a Mama status.
	 * This is only for the Tick42 way of returning info about publish event.
	 * This will be removed when OM implements a standard way to do this.
	 * @param msgStatus
	 * @return
	 */
	public static short convertMsgStatusToMamaStatus(short msgStatus)
	{
		switch (msgStatus) {
		case MamaMsgStatus.STATUS_NOT_PERMISSIONED:
		case MamaMsgStatus.STATUS_NOT_ENTITLED:
			return MamaStatus.MAMA_STATUS_NOT_PERMISSIONED;

		case MamaMsgStatus.STATUS_NOT_FOUND:
			return MamaStatus.MAMA_STATUS_NOT_FOUND;

		case MamaMsgStatus.STATUS_BAD_SYMBOL:
			return MamaStatus.MAMA_STATUS_BAD_SYMBOL;

		case MamaMsgStatus.STATUS_OK:
			return MamaStatus.MAMA_STATUS_OK;

		default:
			return MamaStatus.MAMA_STATUS_SYSTEM_ERROR;

		}
	}
} 
