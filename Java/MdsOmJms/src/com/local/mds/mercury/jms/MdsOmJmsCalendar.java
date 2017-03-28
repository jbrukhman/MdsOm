package com.jpmorgan.mds.mercury.jms;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import com.wombat.mama.MamaDateTime;
import com.wombat.mama.MamaDateTimePrecision;

/**
 * Handle date time as Java Calendar and MamaDateTime
 * 
 * @author u045594 Note that the time here does not have a timezone. Don't used
 *         methods in either vendor class the assume UTC (like
 *         MamaDateTime.setToNow() or Calendar.setTimeInMillis()) unless that is
 *         really what you want.
 */
public class MdsOmJmsCalendar {
	private boolean hasDate = true;
	private boolean hasTime = true;
	private Calendar cal = null;

	// Default values for calendar class
	private SimpleDateFormat dateTimeFormat = new SimpleDateFormat("dd MMM yyyy HH:mm:ss");
	private SimpleDateFormat dateFormat = new SimpleDateFormat("dd MMM yyyy");
	private SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss");

	public SimpleDateFormat getDateTimeFormat() {
		return dateTimeFormat;
	}

	public SimpleDateFormat getDateFormat() {
		return dateFormat;
	}

	public SimpleDateFormat getTimeFormat() {
		return timeFormat;
	}

	public MdsOmJmsCalendar() {
		cal = Calendar.getInstance();
	}

	public MdsOmJmsCalendar(SimpleDateFormat dateF, SimpleDateFormat timeF, SimpleDateFormat dateTimeF){
		dateFormat = dateF;
		timeFormat = timeF;
		dateTimeFormat = dateTimeF;
		cal = Calendar.getInstance();
	}
	
	public MdsOmJmsCalendar(MdsOmJmsConfig config){
		dateFormat = config.getDateFormat();
		timeFormat = config.getTimeFormat();
		dateTimeFormat = config.getDateTimeFormat();
		cal = Calendar.getInstance();
	}
	
	public MdsOmJmsCalendar(Calendar cal) {
		this.cal = cal;
	}
	
	public MdsOmJmsCalendar(Calendar cal, MdsOmJmsConfig config) {
		dateFormat = config.getDateFormat();
		timeFormat = config.getTimeFormat();
		dateTimeFormat = config.getDateTimeFormat();
		this.cal = cal;
	}


	public boolean hasDate() {
		return hasDate;
	}

	public void setHasDate(boolean hasDate) {
		this.hasDate = hasDate;
	}

	public boolean hasTime() {
		return hasTime;
	}

	public void setHasTime(boolean hasTime) {
		this.hasTime = hasTime;
	}

	public Calendar getCalendar() {
		return cal;
	}

	public void setCalendar(Calendar cal) {
		this.cal = cal;
	}

	public void setDate(Date date) {
		cal.setTime(date);
	}

	public Date getDate(){
		return cal.getTime();
	}
	public void set(String str) throws ParseException {
		Date dt = null;
		// TODO length may not be sufficient
		if (str.length() == dateFormat.toPattern().length()) {
			hasDate = true;
			hasTime = false;
			dt = dateFormat.parse(str);
		} else if (str.length() == timeFormat.toPattern().length()) {
			hasDate = false;
			hasTime = true;
			dt = timeFormat.parse(str);
		} else {
			hasDate = true;
			hasTime = true;
			dt = dateTimeFormat.parse(str);
		}
		setDate(dt);
	}

	protected void setToMamaDateTime(MamaDateTime dt) {
		if (hasDate && hasTime) {
			dt.set(cal.get(Calendar.YEAR), cal.get(Calendar.MONTH) + 1,
					cal.get(Calendar.DAY_OF_MONTH),
					cal.get(Calendar.HOUR_OF_DAY), cal.get(Calendar.MINUTE),
					cal.get(Calendar.SECOND),
					cal.get(Calendar.MILLISECOND) * 1000,
					MamaDateTimePrecision.PREC_MICROSECONDS, null);
		} else if (hasDate) {
			dt.setDate(cal.get(Calendar.YEAR), cal.get(Calendar.MONTH) + 1,
					cal.get(Calendar.DAY_OF_MONTH));
		} else if (hasTime) {
			dt.setTime(cal.get(Calendar.HOUR_OF_DAY), cal.get(Calendar.MINUTE),
					cal.get(Calendar.SECOND),
					cal.get(Calendar.MILLISECOND) * 1000,
					MamaDateTimePrecision.PREC_MICROSECONDS, null);
		}
	}

	/**
	 * Set the MdsOmJmsCalendar from a MamaDateTime
	 * 
	 * @param dt
	 */
	protected void setFromMamaDateTime(MamaDateTime dt) {
		if (dt.hasDate() && dt.hasTime()) {
			cal.set((int) dt.getYear(), (int) dt.getMonth() - 1,
					(int) dt.getDay(), (int) dt.getHour(),
					(int) dt.getMinute(), (int) dt.getSecond());
			cal.set(Calendar.MILLISECOND, (int) (dt.getMicrosecond() / 1000));
		} else if (dt.hasDate()) {
			cal.set((int) dt.getYear(), (int) dt.getMonth() - 1,
					(int) dt.getDay());
		} else if (dt.hasTime()) {
			// Only setting time here
			cal.set(Calendar.HOUR_OF_DAY, (int) dt.getHour());
			cal.set(Calendar.MINUTE, (int) dt.getMinute());
			cal.set(Calendar.SECOND, (int) dt.getSecond());
			cal.set(Calendar.MILLISECOND, (int) (dt.getMicrosecond() / 1000));
		}
		hasDate = dt.hasDate();
		hasTime = dt.hasTime();
	}

	public String toString() {
		Date date = cal.getTime();
		SimpleDateFormat sdf = null;
		if (hasDate && hasTime)
			sdf = dateTimeFormat;
		else if (hasDate)
			sdf = dateFormat;
		else if (hasTime)
			sdf = timeFormat;
		String str = "";
		if (sdf != null) {
			str = sdf.format(date);
		}
		return str;
	}

	/**
	 * Compare based on the parameter Cal
	 * 
	 * @param jcal
	 * @return
	 */
	public boolean compare(MdsOmJmsCalendar jcal) {
		Calendar pcal = jcal.getCalendar();
		if (jcal.hasDate && jcal.hasTime) {
			if (cal.get(Calendar.YEAR) != pcal.get(Calendar.YEAR)
					|| cal.get(Calendar.MONTH) != pcal.get(Calendar.MONTH)
					|| cal.get(Calendar.DAY_OF_MONTH) != pcal
							.get(Calendar.DAY_OF_MONTH)
					|| cal.get(Calendar.HOUR_OF_DAY) != pcal
							.get(Calendar.HOUR_OF_DAY)
					|| cal.get(Calendar.MINUTE) != pcal.get(Calendar.MINUTE)
					|| cal.get(Calendar.SECOND) != pcal.get(Calendar.SECOND)
					|| cal.get(Calendar.MILLISECOND) != pcal
							.get(Calendar.MILLISECOND)) {
				return false;
			}
		} else if (jcal.hasDate) {
			if (cal.get(Calendar.YEAR) != pcal.get(Calendar.YEAR)
					|| cal.get(Calendar.MONTH) != pcal.get(Calendar.MONTH)
					|| cal.get(Calendar.DAY_OF_MONTH) != pcal
							.get(Calendar.DAY_OF_MONTH)) {
				return false;
			}
		} else if (jcal.hasTime) {
			if (cal.get(Calendar.HOUR_OF_DAY) != pcal.get(Calendar.HOUR_OF_DAY)
					|| cal.get(Calendar.MINUTE) != pcal.get(Calendar.MINUTE)
					|| cal.get(Calendar.SECOND) != pcal.get(Calendar.SECOND)
					|| cal.get(Calendar.MILLISECOND) != pcal
							.get(Calendar.MILLISECOND)) {
				return false;
			}
		}
		return true;
	}
}
